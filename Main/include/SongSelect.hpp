#pragma once

#include "ApplicationTickable.hpp"
#include "SongSelectItem.hpp"
#include "Audio/Sample.hpp"
#include "Audio/AudioStream.hpp"
#include "GUI/CommonGUIStyle.hpp"
#include "GUI/TextInputField.hpp"
#include "GUI/Panel.hpp"
#include "GUI/SongSelectionWheel.hpp"
#include "GUI/Label.hpp"
#include "GUI/SongFilterSelection.hpp"

/*
	Song select screen
*/
class SongSelect : public IApplicationTickable
{
public:
	~SongSelect();

	bool Init() override;
	void OnKeyPressed(int32 key) override;
	void OnKeyReleased(int32 key) override;
	void Tick(float deltaTime) override;
	void OnSuspend() override;
	void OnRestore() override;

	void OnMapSelected(MapIndex* map);
	void OnDifficultySelected(DifficultyIndex* diff);
	void OnSearchTermChanged(const WString& search);
	void TickNavigation(float deltaTime);

private:
	Timer m_dbUpdateTimer;
	shared_ptr<Canvas> m_canvas;
	MapDatabase m_mapDatabase;

	shared_ptr<SongSelectStyle> m_style;
	shared_ptr<CommonGUIStyle> m_commonGUIStyle;

	// Shows additional information about a map
	shared_ptr<SongStatistics> m_statisticsWindow;
	// Map selection wheel
	shared_ptr<SongSelectionWheel> m_selectionWheel;
	// Filter selection
	shared_ptr<FilterSelection> m_filterSelection;
	// Search field
	shared_ptr<TextInputField> m_searchField;
	// Panel to fade out selection wheel
	shared_ptr<Panel> m_fadePanel;

	shared_ptr<Label> m_filterStatus;

	// Score list canvas
	shared_ptr<Canvas> m_scoreCanvas;
	shared_ptr<LayoutBox> m_scoreList;

	// Current map that has music being preview played
	MapIndex* m_currentPreviewAudio;

	// Select sound
	Sample m_selectSound;

	// Navigation variables
	float m_advanceSong = 0.0f;
	float m_advanceDiff = 0.0f;
	MouseLockHandle m_lockMouse;
	bool m_suspended = false;
	bool m_previewLoaded = true;
	bool m_showScores = false;
	uint64_t m_previewDelayTicks = 0;

	void m_OnButtonPressed(Input::Button buttonCode);

	/*
		Song preview player with fade-in/out
	*/
	class PreviewPlayer
	{
	public:
		void FadeTo(AudioStream stream);
		void Update(float deltaTime);
		void Pause();
		void Restore();

	private:
		const float m_fadeDuration = 0.5f;
		float m_fadeTimer = 0.0f;
		AudioStream m_nextStream;
		AudioStream m_currentStream;
		bool m_nextSet = false;
	} m_previewPlayer;
};
