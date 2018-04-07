#pragma once
#include "GUI/Canvas.hpp"
#include "SongSelectItem.hpp"
#include "SongFilter.hpp"

/*
	Song selection wheel
*/
class SongSelectionWheel : public Canvas
{
public:
	SongSelectionWheel(shared_ptr<SongSelectStyle> style);

	// Called when a new map is selected
	Delegate<MapIndex*> OnMapSelected;
	Delegate<DifficultyIndex*> OnDifficultySelected;

	void OnMapsAdded(Vector<MapIndex*> maps);
	void OnMapsRemoved(Vector<MapIndex*> maps);
	void OnMapsUpdated(Vector<MapIndex*> maps);
	void OnMapsCleared(Map<int32, MapIndex*> newList);
	
	void SelectRandom();
	void SelectMap(int32 newIndex);
	void AdvanceSelection(int32 offset);
	void SelectDifficulty(int32 newDiff);
	void AdvanceDifficultySelection(int32 offset);

	void SetFilter(Map<int32, MapIndex *> filter);
	void SetFilter(SongFilter* filter[2]);
	void ClearFilter();

	MapIndex* GetSelection() const;
	DifficultyIndex* GetSelectedDifficulty() const;

private:
	// keyed on SongSelectIndex::id
	Map<int32, shared_ptr<SongSelectItem>> m_guiElements;
	Map<int32, SongSelectIndex> m_maps;
	Map<int32, SongSelectIndex> m_mapFilter;
	bool m_filterSet = false;

	// Currently selected map ID
	int32 m_currentlySelectedId = 0;
	// Currently selected sub-widget
	shared_ptr<SongSelectItem> m_currentSelection;

	// Current difficulty index
	int32 m_currentlySelectedDiff = 0;

	// Style to use for everything song select related
	shared_ptr<SongSelectStyle> m_style;

	const Map<int32, SongSelectIndex>& m_SourceCollection() const;
	shared_ptr<SongSelectItem> m_GetMapGUIElement(SongSelectIndex index);
	void m_OnMapSelected(SongSelectIndex index);
};