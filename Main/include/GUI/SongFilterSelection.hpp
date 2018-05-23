#pragma once
#include "GUI/Canvas.hpp"
#include "GUI/Label.hpp"
#include "SongFilter.hpp"
#include "SongSelectionWheel.hpp"
#include "SettingsScreen.hpp"

/*
	Filter selection element
*/
class FilterSelection : public Canvas
{
public:
	FilterSelection(shared_ptr<SongSelectionWheel> selectionWheel);

	bool Active = false;

	bool IsAll();
	void AddFilter(SongFilter* filter, FilterType type);
	void SelectFilter(SongFilter* filter, FilterType type);
	void AdvanceSelection(int32 offset);
	void SetMapDB(MapDatabase* db);
	void ToggleSelectionMode();
	String GetStatusText();

private:
	shared_ptr<SongSelectionWheel> m_selectionWheel;
	Vector<SongFilter*> m_folderFilters;
	Vector<SongFilter*> m_levelFilters;
	int32 m_currentFolderSelection = 0;
	int32 m_currentLevelSelection = 0;
	bool m_selectingFolders = true;
	Map<SongFilter*, shared_ptr<Label>> m_guiElements;
	SongFilter* m_currentFilters[2] = {nullptr};
	MapDatabase* m_mapDB;
};