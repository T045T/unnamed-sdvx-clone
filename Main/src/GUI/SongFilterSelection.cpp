#include "stdafx.h"
#include "SongFilterSelection.hpp"

FilterSelection::FilterSelection(shared_ptr<SongSelectionWheel> selectionWheel)
	: m_selectionWheel(std::move(selectionWheel))
{
	const auto lvFilter = new SongFilter();
	const auto flFilter = new SongFilter();

	AddFilter(lvFilter, Level);
	AddFilter(flFilter, Folder);
	for (size_t i = 1; i <= 20; i++)
		AddFilter(new LevelFilter(i), Level);
}

bool FilterSelection::IsAll()
{
	for (auto& m_currentFilter : m_currentFilters)
	{
		if (!m_currentFilter->IsAll())
			return true;
	}
	return false;
}

void FilterSelection::AddFilter(SongFilter* filter, FilterType type)
{
	if (type == Level)
		m_levelFilters.Add(filter);
	else
		m_folderFilters.Add(filter);

	auto label = make_shared<Label>();
	label->SetFontSize(30);
	label->SetText(Utility::ConvertToWString(filter->GetName()));
	if (!m_selectingFolders && type == Folder)
		label->color.w = 0.0f;
	else if (m_selectingFolders && type == Level)
		label->color.w = 0.0f;
	m_guiElements[filter] = label;

	Slot* labelSlot = Add(label);
	labelSlot->allowOverflow = true;
	labelSlot->autoSizeX = true;
	labelSlot->autoSizeY = true;
	labelSlot->anchor = Anchors::MiddleLeft;
	labelSlot->alignment = Vector2(0.f, 0.5f);
	if (type == Level)
	{
		m_currentLevelSelection = 0;
		SelectFilter(m_levelFilters[0], Level);
	}
	else
	{
		m_currentFolderSelection = 0;
		SelectFilter(m_folderFilters[0], Folder);
	}
}

void FilterSelection::SelectFilter(SongFilter* filter, FilterType type)
{
	const uint8 t = type == Folder ? 0 : 1;

	if (m_currentFilters[t])
		m_guiElements[m_currentFilters[t]]->SetText(Utility::ConvertToWString(m_currentFilters[t]->GetName()));
	m_guiElements[filter]->SetText(Utility::ConvertToWString(Utility::Sprintf("->%s", filter->GetName())));

	if (type == Folder)
	{
		for (size_t i = 0; i < m_folderFilters.size(); i++)
		{
			Vector2 coordinate = Vector2(50, 0);
			SongFilter* songFilter = m_folderFilters[i];

			coordinate.y = (static_cast<int>(i) - static_cast<int>(m_currentFolderSelection)) * 40.f;
			coordinate.x -= (static_cast<int>(m_currentFolderSelection) - i) * (static_cast<int>(m_currentFolderSelection) - i) * 1.5;
			Slot* labelSlot = Add(m_guiElements[songFilter]);
			AddAnimation(
				std::static_pointer_cast<IGUIAnimation>(
					std::make_shared<GUIAnimation<Vector2>>(&labelSlot->offset.pos, coordinate, 0.1f)), true);
			labelSlot->offset = Rect(coordinate, Vector2(0));
		}
	}
	else
	{
		for (size_t i = 0; i < m_levelFilters.size(); i++)
		{
			Vector2 coordinate = Vector2(50, 0);
			SongFilter* songFilter = m_levelFilters[i];

			coordinate.y = (static_cast<int>(i) - static_cast<int>(m_currentLevelSelection)) * 40.f;
			coordinate.x -= (static_cast<int>(m_currentLevelSelection) - i) * (static_cast<int>(m_currentLevelSelection) - i) * 1.5;
			Slot* labelSlot = Add(m_guiElements[songFilter]);
			AddAnimation(
				std::static_pointer_cast<IGUIAnimation>(
					std::make_shared<GUIAnimation<Vector2>>(&labelSlot->offset.pos, coordinate, 0.1f)), true);
			labelSlot->offset = Rect(coordinate, Vector2(0));
		}
	}

	m_currentFilters[t] = filter;
	m_selectionWheel->SetFilter(m_currentFilters);
}

void FilterSelection::AdvanceSelection(int32 offset)
{
	if (m_selectingFolders)
	{
		m_currentFolderSelection = (static_cast<int>(m_currentFolderSelection) + offset) % static_cast<int>(m_folderFilters.size());
		if (m_currentFolderSelection < 0)
			m_currentFolderSelection = m_folderFilters.size() + m_currentFolderSelection;
		SelectFilter(m_folderFilters[m_currentFolderSelection], Folder);
	}
	else
	{
		m_currentLevelSelection = (static_cast<int>(m_currentLevelSelection) + offset) % static_cast<int>(m_levelFilters.size());
		if (m_currentLevelSelection < 0)
			m_currentLevelSelection = m_levelFilters.size() + m_currentLevelSelection;
		SelectFilter(m_levelFilters[m_currentLevelSelection], Level);
	}
}

void FilterSelection::SetMapDB(MapDatabase* db)
{
	m_mapDB = db;
	for (const String& p : Path::GetSubDirs(g_gameConfig.GetString(GameConfigKeys::SongFolder)))
	{
		SongFilter* filter = new FolderFilter(p, m_mapDB);
		if (!filter->GetFiltered(Map<int32, SongSelectIndex>()).empty())
			AddFilter(filter, Folder);
	}
}

void FilterSelection::ToggleSelectionMode()
{
	m_selectingFolders = !m_selectingFolders;
	for (auto flFilter : m_folderFilters)
	{
		if (m_selectingFolders)
			m_guiElements[flFilter]->color.w = 1.0f;
		else
			m_guiElements[flFilter]->color.w = 0.0f;
	}
	for (auto lvFilter : m_levelFilters)
	{
		if (!m_selectingFolders)
			m_guiElements[lvFilter]->color.w = 1.0f;
		else
			m_guiElements[lvFilter]->color.w = 0.0f;
	}
}

String FilterSelection::GetStatusText()
{
	return Utility::Sprintf("%s / %s", m_currentFilters[0]->GetName(), m_currentFilters[1]->GetName());
}
