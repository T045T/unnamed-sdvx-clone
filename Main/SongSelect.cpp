#include "stdafx.h"
#include "SongSelect.hpp"
#include "TitleScreen.hpp"
#include "Application.hpp"
#include "Scoring.hpp"
#include "Input.hpp"
#include <GUI/GUI.hpp>
#include "Game.hpp"
#include "TransitionScreen.hpp"
#include "GameConfig.hpp"
#include "SongFilter.hpp"
#include <Audio/Audio.hpp>
#include "Global.hpp"
#include "Audio/Global.hpp"
#ifdef _WIN32
#include "SDL_keycode.h"
#else
#include "SDL2/SDL_keycode.h"
#endif

void SongSelect::PreviewPlayer::FadeTo(AudioStream stream)
{
	// Already existing transition?
	if (m_nextStream)
	{
		if (m_currentStream)
		{
			m_currentStream.reset();
		}
		m_currentStream = m_nextStream;
	}
	m_nextStream = stream;
	m_nextSet = true;
	if (m_nextStream)
	{
		m_nextStream->set_volume(0.0f);
		m_nextStream->Play();
	}
	m_fadeTimer = 0.0f;
}

void SongSelect::PreviewPlayer::Update(float deltaTime)
{
	if (m_nextSet)
	{
		m_fadeTimer += deltaTime;
		if (m_fadeTimer >= m_fadeDuration)
		{
			if (m_currentStream)
			{
				m_currentStream.reset();
			}
			m_currentStream = m_nextStream;
			if (m_currentStream)
				m_currentStream->set_volume(1.0f);
			m_nextStream.reset(); // unsure, was Release()
			m_nextSet = false;
		}
		else
		{
			float fade = m_fadeTimer / m_fadeDuration;

			if (m_currentStream)
				m_currentStream->set_volume(1.0f - fade);
			if (m_nextStream)
				m_nextStream->set_volume(fade);
		}
	}
}

void SongSelect::PreviewPlayer::Pause()
{
	if (m_nextStream)
		m_nextStream->Pause();
	if (m_currentStream)
		m_currentStream->Pause();
}

void SongSelect::PreviewPlayer::Restore()
{
	if (m_nextStream)
		m_nextStream->Play();
	if (m_currentStream)
		m_currentStream->Play();
}

SongSelect::~SongSelect()
{
	// Clear callbacks
	m_mapDatabase.OnMapsCleared.Clear();
	g_input.OnButtonPressed.RemoveAll(this);
}

bool SongSelect::Init()
{
	m_commonGUIStyle = g_commonGUIStyle;
	m_canvas = std::make_shared<Canvas>();

	// Load textures for song select
	m_style = SongSelectStyle::Get(g_application.get());

	// Split between statistics and selection wheel (in percentage)
	const float screenSplit = 0.0f;

	// Statistics window
	m_statisticsWindow = std::make_shared<SongStatistics>(m_style);
	Canvas::Slot* statisticsSlot = m_canvas->Add(std::dynamic_pointer_cast<GUIElementBase>(m_statisticsWindow));
	statisticsSlot->anchor = Anchor(0, 0, screenSplit, 1.0f);
	statisticsSlot->SetZOrder(2);

	// Set up input
	g_input.OnButtonPressed.Add(this, &SongSelect::m_OnButtonPressed);

	auto background = std::make_shared<Panel>();
	background->imageFillMode = FillMode::Fill;
	background->texture = g_application->LoadTexture("bg.png");
	background->color = Color(0.5f);
	Canvas::Slot* bgSlot = m_canvas->Add(background);
	bgSlot->anchor = Anchors::Full;
	bgSlot->SetZOrder(-2);

	auto box = std::make_shared<LayoutBox>();
	Canvas::Slot* boxSlot = m_canvas->Add(box);
	boxSlot->anchor = Anchor(screenSplit, 0, 1.0f, 1.0f);
	box->layoutDirection = LayoutBox::Vertical;
	{
		m_searchField = std::make_shared<TextInputField>(m_commonGUIStyle);
		LayoutBox::Slot* searchFieldSlot = box->Add(std::dynamic_pointer_cast<GUIElementBase>(m_searchField));
		searchFieldSlot->fillX = true;
		m_searchField->OnTextUpdated.Add(this, &SongSelect::OnSearchTermChanged);

		m_filterStatus = std::make_shared<Label>();
		m_filterStatus->SetFontSize(40);
		m_filterStatus->SetText(L"All / All");
		LayoutBox::Slot* filterLabelSlot = box->Add(m_filterStatus);

		m_selectionWheel = std::make_shared<SongSelectionWheel>(m_style);
		LayoutBox::Slot* selectionSlot = box->Add(std::dynamic_pointer_cast<GUIElementBase>(m_selectionWheel));
		selectionSlot->fillY = true;
		m_selectionWheel->OnMapSelected.Add(this, &SongSelect::OnMapSelected);
		m_selectionWheel->OnDifficultySelected.Add(this, &SongSelect::OnDifficultySelected);
	}

	{
		m_fadePanel = std::make_shared<Panel>();
		m_fadePanel->color = Color(0.f);
		m_fadePanel->color.w = 0.0f;
		Canvas::Slot* panelSlot = m_canvas->Add(m_fadePanel);
		panelSlot->anchor = Anchors::Full;
	}

	{
		m_scoreCanvas = std::make_shared<Canvas>();
		Canvas::Slot* slot = m_canvas->Add(m_scoreCanvas);
		slot->anchor = Anchor(1.0, 0.0, 2.0, 10.0);

		auto scoreBg = std::make_shared<Panel>();
		scoreBg->color = Color(Vector3(0.5), 1.0);
		slot = m_scoreCanvas->Add(scoreBg);
		slot->anchor = Anchors::Full;

		m_scoreList = std::make_shared<LayoutBox>();
		m_scoreList->layoutDirection = LayoutBox::LayoutDirection::Vertical;
		slot = m_scoreCanvas->Add(m_scoreList);
		slot->anchor = Anchors::Full;
	}

	{
		m_filterSelection = std::make_shared<FilterSelection>(m_selectionWheel);
		Canvas::Slot* slot = m_canvas->Add(m_filterSelection);
		slot->anchor = Anchor(-1.0, 0.0, 0.0, 1.0);
	}
	m_filterSelection->SetMapDB(&m_mapDatabase);

	// Select interface sound
	m_selectSound = g_audio->CreateSample("audio/menu_click.wav");

	// Setup the map database
	m_mapDatabase.AddSearchPath(g_gameConfig.GetString(GameConfigKeys::SongFolder));

	m_mapDatabase.OnMapsAdded.Add(m_selectionWheel.get(), &SongSelectionWheel::OnMapsAdded);
	m_mapDatabase.OnMapsUpdated.Add(m_selectionWheel.get(), &SongSelectionWheel::OnMapsUpdated);
	m_mapDatabase.OnMapsRemoved.Add(m_selectionWheel.get(), &SongSelectionWheel::OnMapsRemoved);
	m_mapDatabase.OnMapsCleared.Add(m_selectionWheel.get(), &SongSelectionWheel::OnMapsCleared);
	m_mapDatabase.StartSearching();

	m_selectionWheel->SelectRandom();

	/// TODO: Check if debugmute is enabled
	g_audio->SetGlobalVolume(g_gameConfig.GetFloat(GameConfigKeys::MasterVolume));

	return true;
}

void SongSelect::OnKeyPressed(int32 key)
{
	if (m_filterSelection->Active)
	{
		if (key == SDLK_DOWN)
		{
			m_filterSelection->AdvanceSelection(1);
		}
		else if (key == SDLK_UP)
		{
			m_filterSelection->AdvanceSelection(-1);
		}
		else if (key == SDLK_ESCAPE)
		{
			m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&dynamic_cast<Canvas::Slot*>(m_filterSelection->slot)->anchor.left, -1.0f, 0.2f)), true);
			m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&dynamic_cast<Canvas::Slot*>(m_filterSelection->slot)->anchor.right, 0.0f, 0.2f)), true);
			m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&m_fadePanel->color.w, 0.0, 0.25)), true);
			m_filterSelection->Active = !m_filterSelection->Active;
		}
	}
	else
	{
		if (key == SDLK_DOWN)
		{
			m_selectionWheel->AdvanceSelection(1);
		}
		else if (key == SDLK_UP)
		{
			m_selectionWheel->AdvanceSelection(-1);
		}
		else if (key == SDLK_PAGEDOWN)
		{
			m_selectionWheel->AdvanceSelection(5);
		}
		else if (key == SDLK_PAGEUP)
		{
			m_selectionWheel->AdvanceSelection(-5);
		}
		else if (key == SDLK_LEFT)
		{
			m_selectionWheel->AdvanceDifficultySelection(-1);
		}
		else if (key == SDLK_RIGHT)
		{
			m_selectionWheel->AdvanceDifficultySelection(1);
		}
		else if (key == SDLK_F5)
		{
			m_mapDatabase.StartSearching();
		}
		else if (key == SDLK_F2)
		{
			m_selectionWheel->SelectRandom();
		}
		else if (key == SDLK_ESCAPE)
		{
			m_suspended = true;
			g_application->RemoveTickable(this);
		}
		else if (key == SDLK_TAB)
		{
			if (m_searchField->HasInputFocus())
				g_guiRenderer->SetInputFocus(nullptr);
			else
				g_guiRenderer->SetInputFocus(m_searchField.get());
		}
	}
}

void SongSelect::OnKeyReleased(int32 key)
{}

void SongSelect::Tick(float deltaTime)
{
	if (m_dbUpdateTimer.Milliseconds() > 500)
	{
		m_mapDatabase.Update();
		m_dbUpdateTimer.Restart();
	}

	m_filterStatus->SetText(Utility::ConvertToWString(m_filterSelection->GetStatusText()));

	// Tick navigation
	if (!IsSuspended())
		TickNavigation(deltaTime);

	// Ugly hack to get previews working with the delaty
	/// TODO: Move the ticking of the fade timer or whatever outside of onsongselected
	OnMapSelected(m_currentPreviewAudio);

	// Background
	m_previewPlayer.Update(deltaTime);
}

void SongSelect::OnSuspend()
{
	m_suspended = true;
	m_previewPlayer.Pause();
	m_mapDatabase.StopSearching();
	if (m_lockMouse)
		m_lockMouse.reset(); // unsure, was Release()

	g_rootCanvas->Remove(std::dynamic_pointer_cast<GUIElementBase>(m_canvas));
}

void SongSelect::OnRestore()
{
	m_suspended = false;
	m_previewPlayer.Restore();
	m_mapDatabase.StartSearching();

	OnSearchTermChanged(m_searchField->GetText());

	Canvas::Slot* slot = g_rootCanvas->Add(std::dynamic_pointer_cast<GUIElementBase>(m_canvas));
	slot->anchor = Anchors::Full;
}

/**
 * \brief When a map is selected in the song wheel.
 */
void SongSelect::OnMapSelected(MapIndex* map)
{
	if (map == m_currentPreviewAudio)
	{
		if (m_previewDelayTicks)
			--m_previewDelayTicks;
		else if (!m_previewLoaded)
		{
			// Set current preview audio
			DifficultyIndex* previewDiff = m_currentPreviewAudio->difficulties[0];
			String audioPath = m_currentPreviewAudio->path + Path::sep + previewDiff->settings.audioNoFX;

			AudioStream previewAudio = g_audio->CreateStream(audioPath);
			if (previewAudio)
			{
				previewAudio->SetPosition(previewDiff->settings.previewOffset);
				m_previewPlayer.FadeTo(previewAudio);
			}
			else
			{
				Logf("Failed to load preview audio from [%s]", Logger::Warning, audioPath);
				m_previewPlayer.FadeTo(AudioStream());
			}
			m_previewLoaded = true;
			// m_previewPlayer.Restore();
		}
	}
	else
	{
		// Wait at least 15 ticks before attempting to load song to prevent loading songs while scrolling very fast
		m_previewDelayTicks = 15;
		m_currentPreviewAudio = map;
		m_previewLoaded = false;
	}
}

/**
 * \brief When a difficulty is selected in the song wheel.
 */
void SongSelect::OnDifficultySelected(DifficultyIndex* diff)
{
	m_scoreList->Clear();
	uint32 place = 1;

	for (auto it = diff->scores.rbegin(); it != diff->scores.rend(); ++it)
	{
		ScoreIndex s = **it;

		WString grade = Utility::ConvertToWString(Scoring::CalculateGrade(s.score));

		auto text = std::make_shared<Label>();
		text->SetText(Utility::WSprintf(L"--%d--\n%08d\n%d%%\n%ls", place, s.score, (int)(s.gauge * 100), grade));
		text->SetFontSize(32);
		LayoutBox::Slot* slot = m_scoreList->Add(text);
		slot->fillX = true;
		slot->padding = Margin(10, 5, 0, 0);

		if (place++ > 9)
			break;
	}
}

// TODO: Fix some conflicts between search field and filter selection
void SongSelect::OnSearchTermChanged(const WString& search)
{
	if (search.empty())
		m_filterSelection->AdvanceSelection(0);
	else
	{
		String utf8Search = Utility::ConvertToUTF8(search);
		Map<int32, MapIndex*> filter = m_mapDatabase.FindMaps(utf8Search);
		m_selectionWheel->SetFilter(filter);
	}
}

void SongSelect::TickNavigation(float deltaTime)
{
	// Lock mouse to screen when active 
	if (g_gameConfig.GetEnum<Enum_InputDevice>(GameConfigKeys::LaserInputDevice) == InputDevice::Mouse && g_gameWindow->
		IsActive())
	{
		if (!m_lockMouse)
			m_lockMouse = g_input.LockMouse();
		//g_gameWindow->SetCursorVisible(false);
	}
	else
	{
		if (m_lockMouse)
			m_lockMouse.reset(); // unsure, was Release()
		g_gameWindow->SetCursorVisible(true);
	}

	// Song navigation using laser inputs
	/// TODO: Investigate strange behaviour further and clean up.

	const float diff_input = g_input.GetInputLaserDir(0);
	const float song_input = g_input.GetInputLaserDir(1);

	m_advanceDiff += diff_input;
	m_advanceSong += song_input;

	const int advanceDiffActual = static_cast<int>(Math::Floor(m_advanceDiff * Math::Sign(m_advanceDiff))) * Math::Sign(m_advanceDiff);;
	const int advanceSongActual = static_cast<int>(Math::Floor(m_advanceSong * Math::Sign(m_advanceSong))) * Math::Sign(m_advanceSong);;

	if (!m_filterSelection->Active)
	{
		if (advanceDiffActual != 0)
			m_selectionWheel->AdvanceDifficultySelection(advanceDiffActual);
		if (advanceSongActual != 0)
			m_selectionWheel->AdvanceSelection(advanceSongActual);
	}
	else
	{
		if (advanceDiffActual != 0)
			m_filterSelection->AdvanceSelection(advanceDiffActual);
		if (advanceSongActual != 0)
			m_filterSelection->AdvanceSelection(advanceSongActual);
	}

	m_advanceDiff -= advanceDiffActual;
	m_advanceSong -= advanceSongActual;
}

void SongSelect::m_OnButtonPressed(Input::Button buttonCode)
{
	if (m_suspended)
		return;
	if (g_gameConfig.GetEnum<Enum_InputDevice>(GameConfigKeys::ButtonInputDevice) == InputDevice::Keyboard &&
		m_searchField->HasInputFocus())
		return;

	if (buttonCode == Input::Button::BT_S && !m_filterSelection->Active && !IsSuspended())
	{
		// NOTE(local): if filter selection is active, back doesn't work.
		// For now that sounds right, but maybe it shouldn't matter
		if (g_input.Are3BTsHeld())
		{
			m_suspended = true;
			g_application->RemoveTickable(this);
		}
		else
		{
			bool autoplay = (g_gameWindow->GetModifierKeys() & ModifierKeys::Ctrl) == ModifierKeys::Ctrl;
			MapIndex* map = m_selectionWheel->GetSelection();
			if (map)
			{
				DifficultyIndex* diff = m_selectionWheel->GetSelectedDifficulty();

				Game* game = Game::Create(*diff);
				if (!game)
				{
					Logf("Failed to start game", Logger::Error);
					return;
				}
				game->GetScoring().autoplay = autoplay;

				// Transition to game
				g_application->AddTickable(new TransitionScreen(game));
			}
		}
	}
	else
	{
		List<SongFilter*> filters;
		switch (buttonCode)
		{
		case Input::Button::FX_1:
			if (!m_showScores)
			{
				m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&dynamic_cast<Canvas::Slot*>(m_scoreCanvas->slot)->padding.left, -200.0f, 0.2f)), true);
				m_showScores = !m_showScores;
			}
			else
			{
				// TODO: fix this shit
				m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&dynamic_cast<Canvas::Slot*>(m_scoreCanvas->slot)->padding.left, 0.0f, 0.2f)), true);
				m_showScores = !m_showScores;
			}
			break;
		case Input::Button::FX_0:
			if (!m_filterSelection->Active)
			{
				g_guiRenderer->SetInputFocus(nullptr);

				m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&dynamic_cast<Canvas::Slot*>(m_filterSelection->slot)->anchor.left, 0.0, 0.2f)), true);
				m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&dynamic_cast<Canvas::Slot*>(m_filterSelection->slot)->anchor.right, 1.0f, 0.2f)), true);
				m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&m_fadePanel->color.w, 0.75, 0.25)), true);
				m_filterSelection->Active = !m_filterSelection->Active;
			}
			else
			{
				m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&dynamic_cast<Canvas::Slot*>(m_filterSelection->slot)->anchor.left, -1.0f, 0.2f)), true);
				m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&dynamic_cast<Canvas::Slot*>(m_filterSelection->slot)->anchor.right, 0.0f, 0.2f)), true);
				m_canvas->AddAnimation(std::static_pointer_cast<IGUIAnimation>(std::make_shared<GUIAnimation<float>>(&m_fadePanel->color.w, 0.0, 0.25)), true);
				m_filterSelection->Active = !m_filterSelection->Active;
			}
			break;
		case Input::Button::BT_S:
			if (m_filterSelection->Active)
				m_filterSelection->ToggleSelectionMode();
		default:
			break;
		}
	}
}
