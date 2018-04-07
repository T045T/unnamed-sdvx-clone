#include "stdafx.h"
#include "SongSelectionWheel.hpp"

SongSelectionWheel::SongSelectionWheel(shared_ptr<SongSelectStyle> style)
	: m_style(std::move(style))
{}

void SongSelectionWheel::OnMapsAdded(Vector<MapIndex*> maps)
{
	for (auto m : maps)
	{
		SongSelectIndex index(m);
		m_maps.Add(index.id, index);
	}
	if (!m_currentSelection)
		AdvanceSelection(0);
}

void SongSelectionWheel::OnMapsRemoved(Vector<MapIndex*> maps)
{
	for (auto m : maps)
	{
		// TODO(local): don't hard-code the id calc here, maybe make it a utility function?
		SongSelectIndex index = m_maps.at(m->id * 10);
		m_maps.erase(index.id);

		auto it = m_guiElements.find(index.id);
		if (it != m_guiElements.end())
		{
			// Clear selection if a removed item was selected
			if (m_currentSelection == it->second)
				m_currentSelection.reset(); // unsure, was Release()

			// Remove this item from the canvas that displays the items
			Remove(std::dynamic_pointer_cast<GUIElementBase>(it->second));
			m_guiElements.erase(it);
		}
	}
	
	if (!m_maps.Contains(m_currentlySelectedId))
		AdvanceSelection(1);
}

void SongSelectionWheel::OnMapsUpdated(Vector<MapIndex*> maps)
{
	for (auto m : maps)
	{
		// TODO(local): don't hard-code the id calc here, maybe make it a utility function?
		SongSelectIndex index = m_maps.at(m->id * 10);

		auto it = m_guiElements.find(index.id);
		if (it != m_guiElements.end())
			it->second->SetIndex(index.GetMap());
	}
}

void SongSelectionWheel::OnMapsCleared(Map<int32, MapIndex*> newList)
{
	m_currentSelection.reset(); // unsure, was Release()
	for (auto g : m_guiElements)
		Remove(std::dynamic_pointer_cast<GUIElementBase>(g.second));

	m_guiElements.clear();
	m_filterSet = false;
	m_mapFilter.clear();
	m_maps.clear();
	for (auto m : newList)
	{
		SongSelectIndex index(m.second);
		m_maps.Add(index.id, index);
	}

	if (!m_maps.empty())
	{
		// Doing this here, before applying filters, causes our wheel to go
		//  back to the top when a filter should be applied
		// TODO(local): Go through everything in this file and try to clean
		//  up all calls to things like this, to keep it from updating like 7 times >.>
		//AdvanceSelection(0);
	}
}

void SongSelectionWheel::SelectRandom()
{
	if (m_SourceCollection().empty())
		return;
	uint32 selection = Random::IntRange(0, static_cast<int32>(m_SourceCollection().size()) - 1);
	auto it = m_SourceCollection().begin();
	std::advance(it, selection);
	SelectMap(it->first);
}

void SongSelectionWheel::SelectMap(int32 newIndex)
{
	Set<int32> visibleIndices;
	auto& srcCollection = m_SourceCollection();
	auto it = srcCollection.find(newIndex);
	if (it != srcCollection.end())
	{
		const float initialSpacing = 0.65f * m_style->frameMain->GetSize().y;
		const float spacing = 0.8f * m_style->frameSub->GetSize().y;
		const Anchor anchor = Anchor(0.0f, 0.5f, 1.0f, 0.5f);

		static const int32 numItems = 10;

		int32 istart;
		for (istart = 0; istart > -numItems;)
		{
			if (it == srcCollection.begin())
				break;
			--it;
			istart--;
		}

		for (int32 i = istart; i <= numItems; i++)
		{
			if (it != srcCollection.end())
			{
				SongSelectIndex index = it->second;
				int32 id = index.id;

				visibleIndices.Add(id);

				// Add a new map slot
				bool newItem = m_guiElements.find(id) == m_guiElements.end();
				std::shared_ptr<SongSelectItem> item = m_GetMapGUIElement(index);
				float offset = 0;
				if (i != 0)
				{
					offset = initialSpacing * Math::Sign(i) +
						spacing * (i - Math::Sign(i));
				}
				
				Slot* slot = Add(std::dynamic_pointer_cast<GUIElementBase>(item));

				int32 z = -abs(i);
				slot->SetZOrder(z);

				slot->anchor = anchor;
				slot->autoSizeX = true;
				slot->autoSizeY = true;
				slot->alignment = Vector2(0, 0.5f);
				if (newItem)
				{
					// Hard set target position
					slot->offset.pos = Vector2(0, offset);
					slot->offset.size.x = z * 50.0f;
				}
				else
				{
					// Animate towards target position
					item->AddAnimation(
						std::static_pointer_cast<IGUIAnimation>(
							std::make_shared<GUIAnimation<Vector2>>(&slot->offset.pos, Vector2(0, offset), 0.1f)), true);
					item->AddAnimation(
						std::static_pointer_cast<IGUIAnimation>(
							std::make_shared<GUIAnimation<float>>(&slot->offset.size.x, z * 50.0f, 0.1f)), true);
				}

				item->fade = 1.0f - (static_cast<float>(abs(i)) / static_cast<float>(numItems));
				item->innerOffset = item->fade * 100.0f;

				if (i == 0)
				{
					m_currentlySelectedId = newIndex;
					m_OnMapSelected(index);
				}

				++it;
			}
		}
	}
	m_currentlySelectedId = newIndex;

	// Cleanup invisible elements
	for (auto it = m_guiElements.begin(); it != m_guiElements.end();)
	{
		if (!visibleIndices.Contains(it->first))
		{
			Remove(std::dynamic_pointer_cast<GUIElementBase>(it->second));
			it = m_guiElements.erase(it);
			continue;
		}
		++it;
	}
}

void SongSelectionWheel::AdvanceSelection(int32 offset)
{
	auto& srcCollection = m_SourceCollection();
	auto it = srcCollection.find(m_currentlySelectedId);
	if (it == srcCollection.end())
	{
		if (srcCollection.empty())
		{
			// Remove all elements, empty
			m_currentSelection.reset(); // unsure, was Release()
			Clear();
			m_guiElements.clear();
			return;
		}
		it = srcCollection.begin();
	}

	for (uint32 i = 0; i < static_cast<uint32>(abs(offset)); i++)
	{
		auto itn = it;
		if (offset < 0)
		{
			if (itn == srcCollection.begin())
				break;
			--itn;
		}
		else
			++itn;
		if (itn == srcCollection.end())
			break;
		it = itn;
	}

	if (it != srcCollection.end())
		SelectMap(it->first);
}

void SongSelectionWheel::SelectDifficulty(int32 newDiff)
{
	m_currentSelection->SetSelectedDifficulty(newDiff);
	m_currentlySelectedDiff = newDiff;

	Map<int32, SongSelectIndex> maps = m_SourceCollection();
	SongSelectIndex* map = maps.Find(m_currentlySelectedId);

	if (map)
		OnDifficultySelected.Call(map[0].GetDifficulties()[m_currentlySelectedDiff]);
}

void SongSelectionWheel::AdvanceDifficultySelection(int32 offset)
{
	if (!m_currentSelection)
		return;

	Map<int32, SongSelectIndex> maps = m_SourceCollection();
	SongSelectIndex map = maps[m_currentlySelectedId];
	int32 newIdx = m_currentlySelectedDiff + offset;
	newIdx = Math::Clamp(newIdx, 0, static_cast<int32>(map.GetDifficulties().size()) - 1);
	SelectDifficulty(newIdx);
}

void SongSelectionWheel::SetFilter(Map<int32, MapIndex*> filter)
{
	m_mapFilter.clear();
	for (const auto& m : filter)
	{
		SongSelectIndex index(m.second);
		m_mapFilter.Add(index.id, index);
	}

	m_filterSet = true;
	AdvanceSelection(0);
}

void SongSelectionWheel::SetFilter(SongFilter* filter[2])
{
	bool isFiltered = false;
	m_mapFilter = m_maps;
	for (size_t i = 0; i < 2; i++)
	{
		if (!filter[i])
			continue;
		m_mapFilter = filter[i]->GetFiltered(m_mapFilter);
		if (!filter[i]->IsAll())
			isFiltered = true;
	}
	m_filterSet = isFiltered;
	AdvanceSelection(0);
}

void SongSelectionWheel::ClearFilter()
{
	if (m_filterSet)
	{
		m_filterSet = false;
		AdvanceSelection(0);
	}
}

MapIndex* SongSelectionWheel::GetSelection() const
{
	SongSelectIndex const* map = m_SourceCollection().Find(m_currentlySelectedId);
	if (map)
		return map->GetMap();
	return nullptr;
}

DifficultyIndex* SongSelectionWheel::GetSelectedDifficulty() const
{
	SongSelectIndex const* map = m_SourceCollection().Find(m_currentlySelectedId);
	if (map)
		return map->GetDifficulties()[m_currentlySelectedDiff];
	return nullptr;
}

const Map<int32, SongSelectIndex>& SongSelectionWheel::m_SourceCollection() const
{
	return m_filterSet ? m_mapFilter : m_maps;
}

shared_ptr<SongSelectItem> SongSelectionWheel::m_GetMapGUIElement(SongSelectIndex index)
{
	const auto it = m_guiElements.find(index.id);
	if (it != m_guiElements.end())
		return it->second;

	shared_ptr<SongSelectItem> newItem = make_shared<SongSelectItem>(m_style);

	// Send first map as metadata settings
	const BeatmapSettings& firstSettings = index.GetDifficulties()[0]->settings;
	newItem->SetIndex(index);
	m_guiElements.Add(index.id, newItem);
	return newItem;
}

// TODO(local): pretty sure this should be m_OnIndexSelected, and we should filter a call to OnMapSelected
void SongSelectionWheel::m_OnMapSelected(SongSelectIndex index)
{
	// Update compact mode selection views
	if (m_currentSelection)
		m_currentSelection->SwitchCompact(true);
	m_currentSelection = m_guiElements[index.id];
	m_currentSelection->SwitchCompact(false);

	//if(map && map->id == m_currentlySelectedId)
	//	return;

	// Clamp diff selection
	int32 selectDiff = m_currentlySelectedDiff;
	if (m_currentlySelectedDiff >= static_cast<int32>(index.GetDifficulties().size()))
		selectDiff = static_cast<int32>(index.GetDifficulties().size()) - 1;

	SelectDifficulty(selectDiff);
	OnMapSelected.Call(index.GetMap());
}
