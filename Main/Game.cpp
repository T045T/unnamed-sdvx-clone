#include "stdafx.h"
#include "Game.hpp"
#include "Application.hpp"
#include <Beatmap/BeatmapPlayback.hpp>
#include <Shared/Profiling.hpp>
#include "Scoring.hpp"
#include <Audio/Audio.hpp>
#include "Track.hpp"
#include "Camera.hpp"
#include "Background.hpp"
#include "AudioPlayback.hpp"
#include "Input.hpp"
#include "SongSelect.hpp"
#include "ScoreScreen.hpp"
#include "TransitionScreen.hpp"
#include "AsyncAssetLoader.hpp"
#include "GameConfig.hpp"

#ifdef _WIN32
#include"SDL_keycode.h"
#else
#include "SDL2/SDL_keycode.h"
#endif

#include "GUI/GUI.hpp"
#include "GUI/HealthGauge.hpp"
#include "GUI/SettingsBar.hpp"
#include "GUI/PlayingSongInfo.hpp"
#include "Global.hpp"
#include "Audio/Global.hpp"

// Try load map helper
std::shared_ptr<Beatmap> TryLoadMap(const String& path)
{
	// Load map file
	Beatmap* newMap = new Beatmap();
	File mapFile;
	if (!mapFile.OpenRead(path))
	{
		delete newMap;
		return std::make_shared<Beatmap>();
	}
	FileReader reader(mapFile);
	if (!newMap->Load(reader))
	{
		delete newMap;
		return std::make_shared<Beatmap>();
	}

	return std::shared_ptr<Beatmap>(newMap);
}

/* 
	Game implementation class
*/
class Game_Impl : public Game
{
public:
	// Startup parameters
	String m_mapRootPath;
	String m_mapPath;
	DifficultyIndex m_diffIndex;

private:
	bool m_playing = true;
	bool m_started = false;
	bool m_paused = false;
	bool m_ended = false;

	bool m_renderDebugHUD = false;

	// Map object approach speed, scaled by BPM
	float m_hispeed = 1.0f;

	// Current lane toggle status
	bool m_hideLane = false;

	// Use m-mod and what m-mod speed
	bool m_usemMod = false;
	bool m_usecMod = false;
	float m_modSpeed = 400;

	// Game Canvas
	std::shared_ptr<Canvas> m_canvas;
	std::shared_ptr<HealthGauge> m_scoringGauge;
	std::shared_ptr<PlayingSongInfo> m_psi;
	std::shared_ptr<SettingsBar> m_settingsBar;
	std::shared_ptr<CommonGUIStyle> m_guiStyle;
	std::shared_ptr<Label> m_scoreText;

	Graphics::Font m_fontDivlit;

	// Texture of the map jacket image, if available
	Image m_jacketImage;
	Texture m_jacketTexture;

	// Combo colors
	Color m_comboColors[3];

	// The beatmap
	std::shared_ptr<Beatmap> m_beatmap;
	// Scoring system object
	Scoring m_scoring;
	// Beatmap playback manager (object and timing point selector)
	BeatmapPlayback m_playback;
	// Audio playback manager (music and FX))
	AudioPlayback m_audioPlayback;
	// Applied audio offset
	int32 m_audioOffset = 0;
	int32 m_fpsTarget = 0;
	// The play field
	Track* m_track = nullptr;

	// The camera watching the playfield
	Camera m_camera;

	MouseLockHandle m_lockMouse;

	// Current background visualization
	unique_ptr<Background> m_background;
	unique_ptr<Background> m_foreground;

	// Currently active timing point
	const TimingPoint* m_currentTiming;
	// Currently visible gameplay objects
	Vector<ObjectState*> m_currentObjectSet;
	MapTime m_lastMapTime;

	// Rate to sample gauge;
	MapTime m_gaugeSampleRate;
	float m_gaugeSamples[256] = {0.0f};


	// Combo gain animation
	Timer m_comboAnimation;

	Sample m_slamSample;
	Sample m_clickSamples[2];
	Sample* m_fxSamples;

	// Roll intensity, default = 1
	const float m_rollIntensityBase = 0.03f;
	float m_rollIntensity = m_rollIntensityBase;

	// Particle effects
	Material particleMaterial;
	Texture basicParticleTexture;
	Texture squareParticleTexture;
	ParticleSystem m_particleSystem;
	std::shared_ptr<ParticleEmitter> m_laserFollowEmitters[2];
	std::shared_ptr<ParticleEmitter> m_holdEmitters[6];

public:
	Game_Impl(const String& mapPath)
	{
		// Store path to map
		m_mapPath = Path::Normalize(mapPath);
		// Get Parent path
		m_mapRootPath = Path::RemoveLast(m_mapPath, nullptr);

		m_diffIndex.id = -1;
		m_diffIndex.mapId = -1;

		m_hispeed = g_gameConfig.GetFloat(GameConfigKeys::HiSpeed);
		m_usemMod = g_gameConfig.GetBool(GameConfigKeys::UseMMod);
		m_usecMod = g_gameConfig.GetBool(GameConfigKeys::UseCMod);
		m_modSpeed = g_gameConfig.GetFloat(GameConfigKeys::ModSpeed);
	}

	Game_Impl(const DifficultyIndex& difficulty)
	{
		// Store path to map
		m_mapPath = Path::Normalize(difficulty.path);
		m_diffIndex = difficulty;
		// Get Parent path
		m_mapRootPath = Path::RemoveLast(m_mapPath, nullptr);

		m_hispeed = g_gameConfig.GetFloat(GameConfigKeys::HiSpeed);
		m_usemMod = g_gameConfig.GetBool(GameConfigKeys::UseMMod);
		m_usecMod = g_gameConfig.GetBool(GameConfigKeys::UseCMod);
		m_modSpeed = g_gameConfig.GetFloat(GameConfigKeys::ModSpeed);
	}

	~Game_Impl()
	{
		delete m_track;

		// Save hispeed
		g_gameConfig.Set(GameConfigKeys::HiSpeed, m_hispeed);

		g_rootCanvas->Remove(std::dynamic_pointer_cast<GUIElementBase>(m_canvas));

		// In case the cursor was still hidden
		g_gameWindow->SetCursorVisible(true);
		g_input.OnButtonPressed.RemoveAll(this);
	}

	AsyncAssetLoader loader;

	/**
	 * \throws std::runtime_error if failed to load jacket image
	 */
	bool AsyncLoad() override
	{
		ProfilerScope $("AsyncLoad Game");

		if (!Path::FileExists(m_mapPath))
		{
			Logf("Couldn't find map at %s", Logger::Error, m_mapPath);
			return false;
		}

		m_beatmap = TryLoadMap(m_mapPath);

		// Check failure of above loading attempts
		if (!m_beatmap)
		{
			Logf("Failed to load map", Logger::Warning);
			return false;
		}

		// Enable debug functionality
		if (g_application->GetAppCommandLine().Contains("-debug"))
		{
			m_renderDebugHUD = true;
		}

		const BeatmapSettings& mapSettings = m_beatmap->GetMapSettings();

		// Try to load beatmap jacket image
		String jacketPath = m_mapRootPath + "/" + mapSettings.jacketPath;
		m_jacketImage = make_shared<ImageRes>(jacketPath);

		m_gaugeSamples[256] = {0.0f};
		MapTime firstObjectTime = m_beatmap->GetLinearObjects().front()->time;
		ObjectState*const* lastObj = &m_beatmap->GetLinearObjects().back();
		MapTime lastObjectTime = (*lastObj)->time;

		if ((*lastObj)->type == ObjectType::Hold)
		{
			HoldObjectState* lastHold = (HoldObjectState*)(*lastObj);
			lastObjectTime += lastHold->duration;
		}
		else if ((*lastObj)->type == ObjectType::Laser)
		{
			LaserObjectState* lastHold = (LaserObjectState*)(*lastObj);
			lastObjectTime += lastHold->duration;
		}

		// Load combo colors
		Image comboColorPalette;
		comboColorPalette = g_application->LoadImage("combocolors.png");
		assert(comboColorPalette->GetSize().x >= 3);
		for (uint32 i = 0; i < 3; i++)
			m_comboColors[i] = comboColorPalette->GetBits()[i];

		m_gaugeSampleRate = lastObjectTime / 256;

		// Move this somewhere else?
		// Set hi-speed for m-Mod
		// Uses the "mode" of BPMs in the chart, should use median?
		if (m_usemMod)
		{
			Map<double, MapTime> bpmDurations;
			const Vector<TimingPoint*>& timingPoints = m_beatmap->GetLinearTimingPoints();
			MapTime lastMT = 0;
			MapTime largestMT = -1;
			double useBPM = -1;
			double lastBPM = -1;
			for (TimingPoint* tp : timingPoints)
			{
				double thisBPM = tp->GetBPM();
				if (!bpmDurations.count(lastBPM))
				{
					bpmDurations[lastBPM] = 0;
				}
				MapTime timeSinceLastTP = tp->time - lastMT;
				bpmDurations[lastBPM] += timeSinceLastTP;
				if (bpmDurations[lastBPM] > largestMT)
				{
					useBPM = lastBPM;
					largestMT = bpmDurations[lastBPM];
				}
				lastMT = tp->time;
				lastBPM = thisBPM;
			}
			bpmDurations[lastBPM] += lastObjectTime - lastMT;

			if (bpmDurations[lastBPM] > largestMT)
			{
				useBPM = lastBPM;
			}

			m_hispeed = m_modSpeed / useBPM;
		}
		else if (m_usecMod)
		{
			m_hispeed = m_modSpeed / m_beatmap->GetLinearTimingPoints().front()->GetBPM();
		}

		// Initialize input/scoring
		if (!InitGameplay())
			return false;

		// Load beatmap audio
		if (!m_audioPlayback.Init(m_playback, m_mapRootPath))
			return false;

		// Get fps limit
		m_fpsTarget = g_gameConfig.GetInt(GameConfigKeys::FPSTarget);

		ApplyAudioLeadin();

		// Load audio offset
		m_audioOffset = g_gameConfig.GetInt(GameConfigKeys::GlobalOffset);
		m_playback.audioOffset = m_audioOffset;

		/// TODO: Check if debugmute is enabled
		g_audio->SetGlobalVolume(g_gameConfig.GetFloat(GameConfigKeys::MasterVolume));

		if (!InitSFX())
			return false;

		// Intialize track graphics
		m_track = new Track();
		loader.AddLoadable(*m_track, "Track");

		// Load particle textures
		loader.AddTexture(basicParticleTexture, "particle_flare.png");
		loader.AddTexture(squareParticleTexture, "particle_square.png");

		if (!InitHUD())
			return false;

		if (!loader.Load())
			return false;

		// Always hide mouse during gameplay no matter what input mode.
		g_gameWindow->SetCursorVisible(false);

		return true;
	}

	/**
	 * \throws std::runtime_error if failed to create jacket texture
	 * \throws std::runtime_error if failed to create ParticleSystem
	 */
	bool AsyncFinalize() override
	{
		if (m_jacketImage)
		{
			m_jacketTexture = make_shared<TextureRes>(m_jacketImage);
			m_psi->SetJacket(m_jacketTexture);
		}

		if (!loader.Finalize())
			return false;

		m_scoringGauge->fillMaterial->opaque = false;

		// Load particle material
		m_particleSystem = make_shared<ParticleSystemRes>(g_gl);
		CheckedLoad(particleMaterial = g_application->LoadMaterial("particle"));
		particleMaterial->blendMode = MaterialBlendMode::Additive;
		particleMaterial->opaque = false;

		// Background 
		/// TODO: Load this async
		CheckedLoad(m_background = make_unique<Background>(this));
		CheckedLoad(m_foreground = make_unique<Background>(this, true));

		// Do this here so we don't get input events while still loading
		m_scoring.SetPlayback(m_playback);
		m_scoring.SetInput(&g_input);
		m_scoring.Reset(); // Initialize

		g_input.OnButtonPressed.Add(this, &Game_Impl::m_OnButtonPressed);

		return true;
	}

	bool Init() override
	{
		// Add to root canvas to be rendered (this makes the HUD visible)
		Canvas::Slot* rootSlot = g_rootCanvas->Add(std::dynamic_pointer_cast<GUIElementBase>(m_canvas));
		if (g_aspectRatio < 640.f / 480.f)
		{
			Vector2 canvasRes = GUISlotBase::ApplyFill(FillMode::Fit, Vector2(640, 480),
														Rect(0, 0, g_resolution.x, g_resolution.y)).size;

			Vector2 topLeft = Vector2(g_resolution / 2 - canvasRes / 2);

			Vector2 bottomRight = topLeft + canvasRes;
			rootSlot->allowOverflow = true;
			topLeft /= g_resolution;
			bottomRight /= g_resolution;

			rootSlot->anchor = Anchor(topLeft.x, Math::Min(topLeft.y, 0.20f), bottomRight.x, bottomRight.y);
		}
		else
			rootSlot->anchor = Anchors::Full;
		return true;
	}

	// Restart map
	virtual void Restart()
	{
		m_camera = Camera();

		bool audioReinit = m_audioPlayback.Init(m_playback, m_mapRootPath);
		assert(audioReinit);

		// Audio leadin
		ApplyAudioLeadin();

		m_paused = false;
		m_started = false;
		m_ended = false;
		m_hideLane = false;
		m_playback.Reset(m_lastMapTime);
		m_scoring.Reset();

		for (uint32 i = 0; i < 2; i++)
		{
			if (m_laserFollowEmitters[i])
			{
				m_laserFollowEmitters[i].reset();
			}
		}
		for (uint32 i = 0; i < 6; i++)
		{
			if (m_holdEmitters[i])
			{
				m_holdEmitters[i].reset();
			}
		}
		m_track->ClearEffects();
		m_particleSystem->reset();
	}

	virtual void Tick(float deltaTime) override
	{
		// Lock mouse to screen when playing
		if (g_gameConfig.GetEnum<Enum_InputDevice>(GameConfigKeys::LaserInputDevice) == InputDevice::Mouse)
		{
			if (!m_paused && g_gameWindow->IsActive())
			{
				if (!m_lockMouse)
					m_lockMouse = g_input.LockMouse();
				g_gameWindow->SetCursorVisible(false);
			}
			else
			{
				if (m_lockMouse)
					m_lockMouse.reset();
				g_gameWindow->SetCursorVisible(true);
			}
		}

		if (!m_paused)
			TickGameplay(deltaTime);
	}

	virtual void Render(float deltaTime) override
	{
		// 8 beats (2 measures) in view at 1x hi-speed
		m_track->SetViewRange(8.0f / (m_hispeed));

		// Get render state from the camera
		float rollA = m_scoring.GetLaserRollOutput(0);
		float rollB = m_scoring.GetLaserRollOutput(1);
		m_camera.SetTargetRoll(rollA + rollB);
		m_camera.SetRollIntensity(m_rollIntensity);

		// Set track zoom
		if (!m_settingsBar->IsShown()) // Overridden settings?
		{
			m_camera.zoomBottom = m_playback.GetZoom(0);
			m_camera.zoomTop = m_playback.GetZoom(1);
			m_track->roll = m_camera.GetRoll();
		}
		m_track->zoomBottom = m_camera.zoomBottom;
		m_track->zoomTop = m_camera.zoomTop;
		m_camera.track = m_track;
		m_camera.Tick(deltaTime, m_playback);
		m_track->Tick(m_playback, deltaTime);
		RenderState rs = m_camera.CreateRenderState(true);

		// Draw BG first
		m_background->Render(deltaTime);

		// Main render queue
		RenderQueue renderQueue(g_gl, rs);

		// Get objects in range
		MapTime msViewRange = m_playback.ViewDistanceToDuration(m_track->GetViewRange());
		m_currentObjectSet = m_playback.GetObjectsInRange(msViewRange);

		// Draw the base track + time division ticks
		m_track->DrawBase(renderQueue);

		// Sort objects to draw
		m_currentObjectSet.Sort([](const TObjectState<void>* a, const TObjectState<void>* b)
		{
			auto ObjectRenderPriorty = [](const TObjectState<void>* a)
			{
				if (a->type == ObjectType::Single || a->type == ObjectType::Hold)
					return (((ButtonObjectState*)a)->index < 4) ? 1 : 0;
				else
					return 2;
			};
			uint32 renderPriorityA = ObjectRenderPriorty(a);
			uint32 renderPriorityB = ObjectRenderPriorty(b);
			return renderPriorityA < renderPriorityB;
		});

		for (auto& object : m_currentObjectSet)
		{
			m_track->DrawObjectState(renderQueue, m_playback, object, m_scoring.IsObjectHeld(object));
		}

		m_track->DrawDarkTrack(renderQueue);

		// Use new camera for scoring overlay
		//	this is because otherwise some of the scoring elements would get clipped to
		//	the track's near and far planes
		rs = m_camera.CreateRenderState(false);
		RenderQueue scoringRq(g_gl, rs);

		// Copy over laser position and extend info
		for (uint32 i = 0; i < 2; i++)
		{
			if (m_scoring.IsLaserHeld(i))
			{
				m_track->laserPositions[i] = m_scoring.laserTargetPositions[i];
				m_track->lasersAreExtend[i] = m_scoring.lasersAreExtend[i];
			}
			else
			{
				m_track->laserPositions[i] = m_scoring.laserPositions[i];
				m_track->lasersAreExtend[i] = m_scoring.lasersAreExtend[i];
			}
			m_track->laserPositions[i] = m_scoring.laserPositions[i];
			m_track->laserPointerOpacity[i] = (1.0f - Math::Clamp<float>(m_scoring.timeSinceLaserUsed[i] / 0.5f - 1.0f, 0, 1));
		}
		m_track->DrawOverlays(scoringRq);
		float comboZoom = Math::Max(0.0f, (1.0f - (m_comboAnimation.SecondsAsFloat() / 0.2f)) * 0.5f);
		m_track->DrawCombo(scoringRq, m_scoring.currentComboCounter, m_comboColors[m_scoring.comboState], 1.0f + comboZoom);

		// Render queues
		renderQueue.Process();
		scoringRq.Process();

		// Set laser follow particle visiblity
		for (uint32 i = 0; i < 2; i++)
		{
			if (m_scoring.IsLaserHeld(i))
			{
				if (!m_laserFollowEmitters[i])
					m_laserFollowEmitters[i] = CreateTrailEmitter(m_track->laserColors[i]);

				// Set particle position to follow laser
				float followPos = m_scoring.laserTargetPositions[i];
				if (m_scoring.lasersAreExtend[i])
					followPos = followPos * 2.0f - 0.5f;

				m_laserFollowEmitters[i]->position = m_track->TransformPoint(
					Vector3(m_track->trackWidth * followPos - m_track->trackWidth * 0.5f, 0.f, 0.f));
			}
			else
			{
				if (m_laserFollowEmitters[i])
				{
					m_laserFollowEmitters[i].reset();
				}
			}
		}

		// Set hold button particle visibility
		for (uint32 i = 0; i < 6; i++)
		{
			if (m_scoring.IsObjectHeld(i))
			{
				if (!m_holdEmitters[i])
				{
					Color hitColor = (i < 4) ? Color::White : Color::FromHSV(20, 0.7f, 1.0f);
					float hitWidth = (i < 4) ? m_track->buttonWidth : m_track->fxbuttonWidth;
					m_holdEmitters[i] = CreateHoldEmitter(hitColor, hitWidth);
					m_holdEmitters[i]->position.x = m_track->GetButtonPlacement(i);
				}
			}
			else
			{
				if (m_holdEmitters[i])
				{
					m_holdEmitters[i].reset();
				}
			}
		}

		// Render particle effects last
		RenderParticles(rs, deltaTime);

		// Render foreground
		m_foreground->Render(deltaTime);

		// Render debug hud if enabled
		if (m_renderDebugHUD)
		{
			RenderDebugHUD(deltaTime);
		}
	}

	// Initialize HUD elements/layout
	/**
	 * \throws std::runtime_error if failed to create font
	 */
	bool InitHUD()
	{
		String skin = g_gameConfig.GetString(GameConfigKeys::Skin);
		CheckedLoad(m_fontDivlit = make_shared<FontRes>(g_gl, "skins/" + skin + "/fonts/divlit_custom.ttf"));
		m_guiStyle = g_commonGUIStyle;

		// Game GUI canvas
		m_canvas = std::make_shared<Canvas>();

		Vector2 canvasRes = GUISlotBase::ApplyFill(FillMode::Fit, Vector2(640, 480),
													Rect(0, 0, g_resolution.x, g_resolution.y)).size;
		Vector2 topLeft = Vector2(g_resolution / 2 - canvasRes / 2);
		Vector2 bottomRight = topLeft + canvasRes;
		topLeft.y = Math::Min(topLeft.y, g_resolution.y * 0.2f);
		canvasRes.y = bottomRight.y - topLeft.y;

		float scale = canvasRes.x / 640.f;

		if (g_aspectRatio < 1.0)
		{
			//Top Fill
			{
				auto topPanel = std::make_shared<Panel>();
				loader.AddTexture(topPanel->texture, "fill_top.png");
				topPanel->color = Color::White;
				topPanel->imageFillMode = FillMode::Fit;
				topPanel->imageAlignment = Vector2(0.5, 0.0);
				Canvas::Slot* topSlot = m_canvas->Add(topPanel);

				float topPanelTop = topLeft.y / canvasRes.y;

				topSlot->anchor = Anchor(0.0, -topPanelTop, 1.0, 1.0);
				topSlot->alignment = Vector2(0.5, 1.0);
				topSlot->allowOverflow = true;
			}

			//Bottom Fill
			{
				auto bottomPanel = std::make_shared<Panel>();
				loader.AddTexture(bottomPanel->texture, "fill_bottom.png");
				bottomPanel->color = Color::White;
				bottomPanel->imageFillMode = FillMode::Fit;
				bottomPanel->imageAlignment = Vector2(0.5, 1.0);
				Canvas::Slot* bottomSlot = m_canvas->Add(bottomPanel);

				float canvasBottom = topLeft.y + canvasRes.y;
				float pixelsTobottom = g_resolution.y - canvasBottom;
				float bottomPanelbottom = pixelsTobottom / canvasRes.y;

				bottomSlot->anchor = Anchor(0.0, 0.0, 1.0, 1.0 + bottomPanelbottom);
				bottomSlot->alignment = Vector2(0.5, 1.0);
				bottomSlot->allowOverflow = true;
			}
		}

		{
			// Gauge
			m_scoringGauge = std::make_shared<HealthGauge>();
			loader.AddTexture(m_scoringGauge->fillTexture, "gauge_fill.png");
			loader.AddTexture(m_scoringGauge->frontTexture, "gauge_front.png");
			loader.AddTexture(m_scoringGauge->backTexture, "gauge_back.png");
			loader.AddTexture(m_scoringGauge->maskTexture, "gauge_mask.png");
			loader.AddMaterial(m_scoringGauge->fillMaterial, "gauge");

			Canvas::Slot* slot = m_canvas->Add(std::dynamic_pointer_cast<GUIElementBase>(m_scoringGauge));
			slot->anchor = Anchor(0.0, 0.25, 1.0, 0.8);
			slot->alignment = Vector2(1.0f, 0.5f);
			slot->autoSizeX = true;
			slot->autoSizeY = true;
		}

		// Setting bar
		{
			uint8 portrait = g_aspectRatio > 1.0f ? 0 : 1;

			auto sb = std::make_shared<SettingsBar>(m_guiStyle);
			m_settingsBar = std::shared_ptr<SettingsBar>(sb);
			sb->AddSetting(&m_camera.zoomBottom, -1.0f, 1.0f, "Bottom Zoom");
			sb->AddSetting(&m_camera.zoomTop, -1.0f, 1.0f, "Top Zoom");
			sb->AddSetting(&(m_track->roll), 0.0f, 1.0f, "Track roll");
			sb->AddSetting(m_camera.pitchOffsets + portrait, 0.0f, 1.0f, "Crit Line Height");
			sb->AddSetting(m_camera.fovs + portrait, 0.0f, 180.0f, "FOV");
			sb->AddSetting(m_camera.baseRadius + portrait, 0.0f, 2.0f, "Base distance to track");
			sb->AddSetting(m_camera.basePitch + portrait, 0.0f, -180.0f, "Base pitch");
			sb->AddSetting(&(m_track->trackLength), 4.0f, 20.0f, "Track Length");
			sb->AddSetting(&m_hispeed, 0.25f, 16.0f, "HiSpeed multiplier");
			sb->AddSetting(&m_scoring.laserDistanceLeniency, 1.0f / 32.0f, 1.0f, "Laser Distance Leniency");
			m_settingsBar->SetShow(false);

			Canvas::Slot* settingsSlot = m_canvas->Add(sb);
			settingsSlot->anchor = Anchor(0.75f, 0.0f, 1.0f, 1.0f);
			settingsSlot->autoSizeX = false;
			settingsSlot->autoSizeY = false;
			settingsSlot->SetZOrder(2);
		}

		// Score
		{
			auto scorePanel = std::make_shared<Panel>();
			loader.AddTexture(scorePanel->texture, "scoring_base.png");
			scorePanel->color = Color::White;
			scorePanel->imageFillMode = FillMode::Fit;

			Canvas::Slot* scoreSlot = m_canvas->Add(scorePanel);
			scoreSlot->anchor = Anchor(0.75, 0.0, 1.0, 1.0);
			scoreSlot->alignment = Vector2(1.0f, 0.0f);
			scoreSlot->autoSizeX = true;
			scoreSlot->autoSizeY = true;

			m_scoreText = std::shared_ptr<Label>(new Label());
			m_scoreText->SetFontSize(32 * scale);
			m_scoreText->SetText(Utility::WSprintf(L"%08d", 0));
			m_scoreText->SetFont(m_fontDivlit);
			m_scoreText->SetTextOptions(FontRes::Monospace);
			// Padding for this specific font
			Margin textPadding = Margin(0, 10, 0, 0);

			Panel::Slot* slot = scorePanel->SetContent(std::dynamic_pointer_cast<GUIElementBase>(m_scoreText));
			slot->padding = (Margin(20, 0, 10, 30) + textPadding) * scale;

			slot->alignment = Vector2(0.5f, 0.5f);
		}

		// Song info
		{
			auto psi = std::make_shared<PlayingSongInfo>(*this);
			m_psi = std::shared_ptr<PlayingSongInfo>(psi);
			loader.AddMaterial(m_psi->progressMaterial, "progressBar");
			Canvas::Slot* psiSlot = m_canvas->Add(psi);
			psiSlot->autoSizeY = true;
			psiSlot->autoSizeX = true;
			psiSlot->anchor = Anchors::TopLeft;
			psiSlot->alignment = Vector2(0.0f, 0.0f);
			psiSlot->padding = Margin(10, 10, 0, 0);
		}

		return true;
	}

	// Wait before start of map
	void ApplyAudioLeadin()
	{
		// Select the correct first object to set the intial playback position
		// if it starts before a certain time frame, the song starts at a negative time (lead-in)
		ObjectState*const* firstObj = &m_beatmap->GetLinearObjects().front();
		while ((*firstObj)->type == ObjectType::Event && firstObj != &m_beatmap->GetLinearObjects().back())
		{
			firstObj++;
		}
		m_lastMapTime = 0;
		MapTime firstObjectTime = (*firstObj)->time;
		if (firstObjectTime < 1000)
		{
			// Set start time
			m_lastMapTime = firstObjectTime - 5000;
			m_audioPlayback.SetPosition(m_lastMapTime);
		}

		// Reset playback
		m_playback.Reset(m_lastMapTime);
	}

	// Loads sound effects
	bool InitSFX()
	{
		CheckedLoad(m_slamSample = g_application->LoadSample("laser_slam"));
		CheckedLoad(m_clickSamples[0] = g_application->LoadSample("click-01"));
		CheckedLoad(m_clickSamples[1] = g_application->LoadSample("click-02"));

		auto samples = m_beatmap->GetSamplePaths();
		m_fxSamples = new Sample[samples.size()];
		for (int i = 0; i < samples.size(); i++)
		{
			CheckedLoad(m_fxSamples[i] = g_application->LoadSample(m_mapRootPath + "/" + samples[i], true));
		}

		return true;
	}

	bool InitGameplay()
	{
		// Playback and timing
		m_playback = BeatmapPlayback(*m_beatmap);
		m_playback.OnEventChanged.Add(this, &Game_Impl::OnEventChanged);
		m_playback.OnLaneToggleChanged.Add(this, &Game_Impl::OnLaneToggleChanged);
		m_playback.OnFXBegin.Add(this, &Game_Impl::OnFXBegin);
		m_playback.OnFXEnd.Add(this, &Game_Impl::OnFXEnd);
		m_playback.OnLaserAlertEntered.Add(this, &Game_Impl::OnLaserAlertEntered);
		m_playback.Reset();

		/// TODO: c-mod is broken, might need something in the viewrange calculation stuff
		// If c-mod is used
		if (m_usecMod)
		{
			m_playback.OnTimingPointChanged.Add(this, &Game_Impl::OnTimingPointChanged);
		}
		// Register input bindings
		m_scoring.OnButtonMiss.Add(this, &Game_Impl::OnButtonMiss);
		m_scoring.OnLaserSlamHit.Add(this, &Game_Impl::OnLaserSlamHit);
		m_scoring.OnButtonHit.Add(this, &Game_Impl::OnButtonHit);
		m_scoring.OnComboChanged.Add(this, &Game_Impl::OnComboChanged);
		m_scoring.OnObjectHold.Add(this, &Game_Impl::OnObjectHold);
		m_scoring.OnObjectReleased.Add(this, &Game_Impl::OnObjectReleased);
		m_scoring.OnScoreChanged.Add(this, &Game_Impl::OnScoreChanged);

		m_playback.hittableObjectEnter = Scoring::missHitTime;
		m_playback.hittableObjectLeave = Scoring::goodHitTime;

		if (g_application->GetAppCommandLine().Contains("-autobuttons"))
		{
			m_scoring.autoplayButtons = true;
		}

		return true;
	}

	// Processes input and Updates scoring, also handles audio timing management
	void TickGameplay(float deltaTime)
	{
		if (!m_started)
		{
			// Start playback of audio in first gameplay tick
			m_audioPlayback.Play();
			m_started = true;

			if (g_application->GetAppCommandLine().Contains("-autoskip"))
			{
				SkipIntro();
			}
		}

		const BeatmapSettings& beatmapSettings = m_beatmap->GetMapSettings();

		// Update beatmap playback
		MapTime playbackPositionMs = m_audioPlayback.GetPosition() - m_audioOffset;
		m_playback.Update(playbackPositionMs);

		MapTime delta = playbackPositionMs - m_lastMapTime;
		int32 beatStart = 0;
		uint32 numBeats = m_playback.CountBeats(m_lastMapTime, delta, beatStart, 1);
		if (numBeats > 0)
		{
			// Click Track
			//uint32 beat = beatStart % m_playback.GetCurrentTimingPoint().measure;
			//if(beat == 0)
			//{
			//	m_clickSamples[0]->Play();
			//}
			//else
			//{
			//	m_clickSamples[1]->Play();
			//}
		}

		/// #Scoring
		// Update music filter states
		m_audioPlayback.SetLaserFilterInput(m_scoring.GetLaserOutput(),
											m_scoring.IsLaserHeld(0, false) || m_scoring.IsLaserHeld(1, false));
		m_audioPlayback.Tick(deltaTime);

		// Link FX track to combo counter for now
		m_audioPlayback.SetFXTrackEnabled(m_scoring.currentComboCounter > 0);

		// Update scoring
		m_scoring.Tick(deltaTime);

		// Update scoring gauge
		m_scoringGauge->rate = m_scoring.currentGauge;

		int32 gaugeSampleSlot = playbackPositionMs;
		gaugeSampleSlot /= m_gaugeSampleRate;
		gaugeSampleSlot = Math::Clamp(gaugeSampleSlot, (int32)0, (int32)255);
		m_gaugeSamples[gaugeSampleSlot] = m_scoring.currentGauge;

		// Get the current timing point
		m_currentTiming = &m_playback.GetCurrentTimingPoint();

		// Update song info display
		ObjectState*const* lastObj = &m_beatmap->GetLinearObjects().back();
		m_psi->SetProgress((float)playbackPositionMs / (*lastObj)->time);
		m_psi->SetHiSpeed(m_hispeed);
		m_psi->SetBPM((float)m_currentTiming->GetBPM());

		// Update hispeed
		if (g_input.GetButton(Input::Button::BT_S))
		{
			for (int i = 0; i < 2; i++)
			{
				m_hispeed += g_input.GetInputLaserDir(i);
				m_hispeed = Math::Clamp(m_hispeed, 0.1f, 16.f);
				if (m_usecMod || m_usemMod)
				{
					g_gameConfig.Set(GameConfigKeys::ModSpeed, m_hispeed * (float)m_currentTiming->GetBPM());
				}
			}
		}

		m_lastMapTime = playbackPositionMs;

		if (m_audioPlayback.HasEnded())
		{
			FinishGame();
		}
	}

	// Called when game is finished and the score screen should show up
	void FinishGame()
	{
		if (m_ended)
			return;

		// Transition to score screen
		auto transition = new TransitionScreen(new ScoreScreen(this));
		transition->OnLoadingComplete.Add(this, &Game_Impl::OnScoreScreenLoaded);
		g_application->AddTickable(transition);

		m_ended = true;
	}

	void OnScoreScreenLoaded(IAsyncLoadableApplicationTickable* tickable)
	{
		// Remove self
		g_application->RemoveTickable(this);
	}

	void RenderParticles(const RenderState& rs, float deltaTime)
	{
		// Render particle effects
		m_particleSystem->render(rs, deltaTime);
	}

	std::shared_ptr<ParticleEmitter> CreateTrailEmitter(const Color& color)
	{
		std::shared_ptr<ParticleEmitter> emitter = m_particleSystem->add_emitter();
		emitter->material = particleMaterial;
		emitter->texture = basicParticleTexture;
		emitter->loops = 0;
		emitter->duration = 5.0f;
		emitter->SetSpawnRate(PPRandomRange<float>(250, 300));
		emitter->SetStartPosition(PPBox({0.5f, 0.0f, 0.0f}));
		emitter->SetStartSize(PPRandomRange<float>(0.25f, 0.4f));
		emitter->SetScaleOverTime(PPRange<float>(2.0f, 1.0f));
		emitter->SetFadeOverTime(PPRangeFadeIn<float>(1.0f, 0.0f, 0.4f));
		emitter->SetLifetime(PPRandomRange<float>(0.17f, 0.2f));
		emitter->SetStartDrag(PPConstant<float>(0.0f));
		emitter->SetStartVelocity(PPConstant<Vector3>({0, -4.0f, 2.0f}));
		emitter->SetSpawnVelocityScale(PPRandomRange<float>(0.9f, 2));
		emitter->SetStartColor(PPConstant<Color>((Color)(color * 0.7f)));
		emitter->SetGravity(PPConstant<Vector3>(Vector3(0.0f, 0.0f, -9.81f)));
		emitter->position.y = 0.0f;
		emitter->position = m_track->TransformPoint(emitter->position);
		emitter->scale = 0.3f;
		return emitter;
	}

	std::shared_ptr<ParticleEmitter> CreateHoldEmitter(const Color& color, float width)
	{
		std::shared_ptr<ParticleEmitter> emitter = m_particleSystem->add_emitter();
		emitter->material = particleMaterial;
		emitter->texture = basicParticleTexture;
		emitter->loops = 0;
		emitter->duration = 5.0f;
		emitter->SetSpawnRate(PPRandomRange<float>(50, 100));
		emitter->SetStartPosition(PPBox({width, 0.0f, 0.0f}));
		emitter->SetStartSize(PPRandomRange<float>(0.3f, 0.35f));
		emitter->SetScaleOverTime(PPRange<float>(1.2f, 1.0f));
		emitter->SetFadeOverTime(PPRange<float>(1.0f, 0.0f));
		emitter->SetLifetime(PPRandomRange<float>(0.10f, 0.15f));
		emitter->SetStartDrag(PPConstant<float>(0.0f));
		emitter->SetStartVelocity(PPConstant<Vector3>({0.0f, 0.0f, 0.0f}));
		emitter->SetSpawnVelocityScale(PPRandomRange<float>(0.2f, 0.2f));
		emitter->SetStartColor(PPConstant<Color>((Color)(color * 0.6f)));
		emitter->SetGravity(PPConstant<Vector3>(Vector3(0.0f, 0.0f, -4.81f)));
		emitter->position.y = 0.0f;
		emitter->position = m_track->TransformPoint(emitter->position);
		emitter->scale = 1.0f;
		return emitter;
	}

	std::shared_ptr<ParticleEmitter> CreateExplosionEmitter(const Color& color, const Vector3 dir)
	{
		std::shared_ptr<ParticleEmitter> emitter = m_particleSystem->add_emitter();
		emitter->material = particleMaterial;
		emitter->texture = basicParticleTexture;
		emitter->loops = 1;
		emitter->duration = 0.2f;
		emitter->SetSpawnRate(PPRange<float>(200, 0));
		emitter->SetStartPosition(PPSphere(0.1f));
		emitter->SetStartSize(PPRandomRange<float>(0.7f, 1.1f));
		emitter->SetFadeOverTime(PPRangeFadeIn<float>(0.9f, 0.0f, 0.0f));
		emitter->SetLifetime(PPRandomRange<float>(0.22f, 0.3f));
		emitter->SetStartDrag(PPConstant<float>(0.2f));
		emitter->SetSpawnVelocityScale(PPRandomRange<float>(1.0f, 4.0f));
		emitter->SetScaleOverTime(PPRange<float>(1.0f, 0.4f));
		emitter->SetStartVelocity(PPConstant<Vector3>(dir * 5.0f));
		emitter->SetStartColor(PPConstant<Color>(color));
		emitter->SetGravity(PPConstant<Vector3>(Vector3(0.0f, 0.0f, -9.81f)));
		emitter->position.y = 0.0f;
		emitter->position = m_track->TransformPoint(emitter->position);
		emitter->scale = 0.4f;
		return emitter;
	}

	std::shared_ptr<ParticleEmitter> CreateHitEmitter(const Color& color, float width)
	{
		std::shared_ptr<ParticleEmitter> emitter = m_particleSystem->add_emitter();
		emitter->material = particleMaterial;
		emitter->texture = basicParticleTexture;
		emitter->loops = 1;
		emitter->duration = 0.15f;
		emitter->SetSpawnRate(PPRange<float>(50, 0));
		emitter->SetStartPosition(PPBox(Vector3(width * 0.5f, 0.0f, 0)));
		emitter->SetStartSize(PPRandomRange<float>(0.3f, 0.1f));
		emitter->SetFadeOverTime(PPRangeFadeIn<float>(0.7f, 0.0f, 0.0f));
		emitter->SetLifetime(PPRandomRange<float>(0.35f, 0.4f));
		emitter->SetStartDrag(PPConstant<float>(6.0f));
		emitter->SetSpawnVelocityScale(PPConstant<float>(0.0f));
		emitter->SetScaleOverTime(PPRange<float>(1.0f, 0.4f));
		emitter->SetStartVelocity(PPCone(Vector3(0, 0, -1), 90.0f, 1.0f, 4.0f));
		emitter->SetStartColor(PPConstant<Color>(color));
		emitter->position.y = 0.0f;
		return emitter;
	}

	// Main GUI/HUD Rendering loop
	virtual void RenderDebugHUD(float deltaTime)
	{
		// Render debug overlay elements
		RenderQueue& debugRq = g_guiRenderer->Begin();
		auto RenderText = [&](const String& text, const Vector2& pos, const Color& color = Color::White)
		{
			return g_guiRenderer->RenderText(text, pos, color);
		};

		Vector2 canvasRes = GUISlotBase::ApplyFill(FillMode::Fit, Vector2(640, 480),
													Rect(0, 0, g_resolution.x, g_resolution.y)).size;
		Vector2 topLeft = Vector2(g_resolution / 2 - canvasRes / 2);
		Vector2 bottomRight = topLeft + canvasRes;
		topLeft.y = Math::Min(topLeft.y, g_resolution.y * 0.2f);

		const BeatmapSettings& bms = m_beatmap->GetMapSettings();
		const TimingPoint& tp = m_playback.GetCurrentTimingPoint();
		Vector2 textPos = topLeft + Vector2i(5, 0);
		textPos.y += RenderText(bms.title, textPos).y;
		textPos.y += RenderText(bms.artist, textPos).y;
		textPos.y += RenderText(Utility::Sprintf("%.2f FPS", g_application->GetRenderFPS()), textPos).y;
		textPos.y += RenderText(Utility::Sprintf("Audio Offset: %d ms", g_audio->get_audio_latency()), textPos).y;

		float currentBPM = (float)(60000.0 / tp.beatDuration);
		textPos.y += RenderText(Utility::Sprintf("BPM: %.1f", currentBPM), textPos).y;
		textPos.y += RenderText(Utility::Sprintf("Time Signature: %d/4", tp.numerator), textPos).y;
		textPos.y += RenderText(Utility::Sprintf("Laser Effect Mix: %f", m_audioPlayback.GetLaserEffectMix()), textPos).y;
		textPos.y += RenderText(Utility::Sprintf("Laser Filter Input: %f", m_scoring.GetLaserOutput()), textPos).y;

		textPos.y += RenderText(
			Utility::Sprintf("Score: %d (Max: %d)", m_scoring.currentHitScore, m_scoring.mapTotals.maxScore), textPos).y;
		textPos.y += RenderText(Utility::Sprintf("Actual Score: %d", m_scoring.CalculateCurrentScore()), textPos).y;

		textPos.y += RenderText(Utility::Sprintf("Health Gauge: %f", m_scoring.currentGauge), textPos).y;

		textPos.y += RenderText(Utility::Sprintf("Roll: %f(x%f) %s",
												m_camera.GetRoll(), m_rollIntensity, m_camera.rollKeep ? "[Keep]" : ""), textPos).y;

		textPos.y += RenderText(Utility::Sprintf("Track Zoom Top: %f", m_camera.zoomTop), textPos).y;
		textPos.y += RenderText(Utility::Sprintf("Track Zoom Bottom: %f", m_camera.zoomBottom), textPos).y;

		Vector2 buttonStateTextPos = Vector2(g_resolution.x - 200.0f, 100.0f);
		RenderText(g_input.GetControllerStateString(), buttonStateTextPos);

		if (m_scoring.autoplay)
			textPos.y += RenderText("Autoplay enabled", textPos, Color::Blue).y;

		// List recent hits and their delay
		Vector2 tableStart = textPos;
		uint32 hitsShown = 0;
		// Show all hit debug info on screen (up to a maximum)
		for (auto it = m_scoring.hitStats.rbegin(); it != m_scoring.hitStats.rend(); it++)
		{
			if (hitsShown++ > 16) // Max of 16 entries to display
				break;

			static Color hitColors[] = {
				Color::Red,
				Color::Yellow,
				Color::Green,
			};
			Color c = hitColors[(size_t)(*it)->rating];
			if ((*it)->hasMissed && (*it)->hold > 0)
				c = Color(1, 0.65f, 0);
			String text;

			MultiObjectState* obj = *(*it)->object;
			if (obj->type == ObjectType::Single)
			{
				text = Utility::Sprintf("[%d] %d", obj->button.index, (*it)->delta);
			}
			else if (obj->type == ObjectType::Hold)
			{
				text = Utility::Sprintf("Hold [%d] [%d/%d]", obj->button.index, (*it)->hold, (*it)->holdMax);
			}
			else if (obj->type == ObjectType::Laser)
			{
				text = Utility::Sprintf("Laser [%d] [%d/%d]", obj->laser.index, (*it)->hold, (*it)->holdMax);
			}
			textPos.y += RenderText(text, textPos, c).y;
		}

		g_guiRenderer->End();
	}

	void OnLaserSlamHit(LaserObjectState* object)
	{
		float slamSize = fabs(object->points[1] - object->points[0]);
		CameraShake shake(0.2f, 0.5f, 170.0f);
		shake.amplitude = Vector3(0.04f * slamSize, 0.01f, 0.0f); // Mainly x-axis
		m_camera.AddCameraShake(shake);
		m_slamSample->Play();

		if (object->spin.type != 0)
		{
			m_camera.SetSpin(object->GetDirection(), object->spin.duration, object->spin.type, m_playback);
		}

		float dir = Math::Sign(object->points[1] - object->points[0]);
		float laserPos = m_track->trackWidth * object->points[1] - m_track->trackWidth * 0.5f;
		std::shared_ptr<ParticleEmitter> ex = CreateExplosionEmitter(m_track->laserColors[object->index], Vector3(dir, 0, 0));
		ex->position = Vector3(laserPos, 0.0f, -0.05f);
		ex->position = m_track->TransformPoint(ex->position);
	}

	void OnButtonHit(Input::Button button, ScoreHitRating rating, ObjectState* hitObject, bool late)
	{
		ButtonObjectState* st = (ButtonObjectState*)hitObject;
		uint32 buttonIdx = (uint32)button;
		Color c = m_track->hitColors[(size_t)rating];

		// The color effect in the button lane
		m_track->AddEffect(new ButtonHitEffect(buttonIdx, c));

		if (st != nullptr && st->hasSample)
		{
			m_fxSamples[st->sampleIndex]->Play();
		}

		if (rating != ScoreHitRating::Idle)
		{
			// Floating text effect
			m_track->AddEffect(new ButtonHitRatingEffect(buttonIdx, rating));

			if (rating == ScoreHitRating::Good)
			{
				m_track->timedHitEffect->late = late;
				m_track->timedHitEffect->Reset(0.75f);
			}

			// Create hit effect particle
			Color hitColor = (buttonIdx < 4) ? Color::White : Color::FromHSV(20, 0.7f, 1.0f);
			float hitWidth = (buttonIdx < 4) ? m_track->buttonWidth : m_track->fxbuttonWidth;
			std::shared_ptr<ParticleEmitter> emitter = CreateHitEmitter(hitColor, hitWidth);
			emitter->position.x = m_track->GetButtonPlacement(buttonIdx);
			emitter->position.z = -0.05f;
			emitter->position.y = 0.0f;
			emitter->position = m_track->TransformPoint(emitter->position);
		}
	}

	void OnButtonMiss(Input::Button button, bool hitEffect)
	{
		uint32 buttonIdx = (uint32)button;
		if (hitEffect)
		{
			Color c = m_track->hitColors[0];
			m_track->AddEffect(new ButtonHitEffect(buttonIdx, c));
		}
		m_track->AddEffect(new ButtonHitRatingEffect(buttonIdx, ScoreHitRating::Miss));
	}

	void OnComboChanged(uint32 newCombo)
	{
		m_comboAnimation.Restart();
	}

	void OnScoreChanged(uint32 newScore)
	{
		// Update score text
		if (m_scoreText)
		{
			m_scoreText->SetText(Utility::WSprintf(L"%08d", newScore));
		}
	}

	// These functions control if FX button DSP's are muted or not
	void OnObjectHold(Input::Button, ObjectState* object)
	{
		if (object->type == ObjectType::Hold)
		{
			HoldObjectState* hold = (HoldObjectState*)object;
			if (hold->effectType != EffectType::None)
			{
				m_audioPlayback.SetEffectEnabled(hold->index - 4, true);
			}
		}
	}

	void OnObjectReleased(Input::Button, ObjectState* object)
	{
		if (object->type == ObjectType::Hold)
		{
			HoldObjectState* hold = (HoldObjectState*)object;
			if (hold->effectType != EffectType::None)
			{
				m_audioPlayback.SetEffectEnabled(hold->index - 4, false);
			}
		}
	}


	void OnTimingPointChanged(TimingPoint* tp)
	{
		m_hispeed = m_modSpeed / tp->GetBPM();
	}

	void OnLaneToggleChanged(LaneHideTogglePoint* tp)
	{
		// Calculate how long the transition should be in seconds
		double duration = m_currentTiming->beatDuration * 4.0f * (tp->duration / 192.0f) * 0.001f;
		m_track->SetLaneHide(!m_hideLane, duration);
		m_hideLane = !m_hideLane;
	}

	void OnEventChanged(EventKey key, EventData data)
	{
		if (key == EventKey::LaserEffectType)
		{
			m_audioPlayback.SetLaserEffect(data.effectVal);
		}
		else if (key == EventKey::LaserEffectMix)
		{
			m_audioPlayback.SetLaserEffectMix(data.floatVal);
		}
		else if (key == EventKey::TrackRollBehaviour)
		{
			m_camera.rollKeep = (data.rollVal & TrackRollBehaviour::Keep) == TrackRollBehaviour::Keep;
			int32 i = (uint8)data.rollVal & 0x3;
			if (i == 0)
				m_rollIntensity = 0;
			else
			{
				m_rollIntensity = m_rollIntensityBase + (float)(i - 1) * 0.0125f;
			}
		}
		else if (key == EventKey::SlamVolume)
		{
			m_slamSample->set_volume(data.floatVal);
		}
	}

	// These functions register / remove DSP's for the effect buttons
	// the actual hearability of these is toggled in the tick by wheneter the buttons are held down
	void OnFXBegin(HoldObjectState* object)
	{
		assert(object->index >= 4 && object->index <= 5);
		m_audioPlayback.SetEffect(object->index - 4, object, m_playback);
	}

	void OnFXEnd(HoldObjectState* object)
	{
		assert(object->index >= 4 && object->index <= 5);
		uint32 index = object->index - 4;
		m_audioPlayback.ClearEffect(index, object);
	}

	void OnLaserAlertEntered(LaserObjectState* object)
	{
		if (m_scoring.timeSinceLaserUsed[object->index] > 3.0f)
		{
			m_track->SendLaserAlert(object->index);
		}
	}

	virtual void OnKeyPressed(int32 key) override
	{
		if (key == SDLK_PAUSE)
		{
			m_audioPlayback.TogglePause();
			m_paused = m_audioPlayback.IsPaused();
		}
		else if (key == SDLK_RETURN) // Skip intro
		{
			if (!SkipIntro())
				SkipOutro();
		}
		else if (key == SDLK_PAGEUP)
		{
			m_audioPlayback.Advance(5000);
		}
		else if (key == SDLK_ESCAPE)
		{
			FinishGame();
		}
		else if (key == SDLK_F5) // Restart map
		{
			// Restart
			Restart();
		}
		else if (key == SDLK_F8)
		{
			m_renderDebugHUD = !m_renderDebugHUD;
			m_psi->visibility = m_renderDebugHUD ? Visibility::Collapsed : Visibility::Visible;
		}
		else if (key == SDLK_TAB)
		{
			g_gameWindow->SetCursorVisible(!m_settingsBar->IsShown());
			m_settingsBar->SetShow(!m_settingsBar->IsShown());
		}
	}

	void m_OnButtonPressed(Input::Button buttonCode)
	{
		if (buttonCode == Input::Button::BT_S)
		{
			if (g_input.Are3BTsHeld())
				FinishGame();
		}
	}

	// Skips ahead to the right before the first object in the map
	bool SkipIntro()
	{
		ObjectState*const* firstObj = &m_beatmap->GetLinearObjects().front();
		while ((*firstObj)->type == ObjectType::Event && firstObj != &m_beatmap->GetLinearObjects().back())
		{
			firstObj++;
		}
		MapTime skipTime = (*firstObj)->time - 1000;
		if (skipTime > m_lastMapTime)
		{
			m_audioPlayback.SetPosition(skipTime);
			return true;
		}
		return false;
	}

	// Skips ahead at the end to the score screen
	void SkipOutro()
	{
		// Just to be sure
		if (m_beatmap->GetLinearObjects().empty())
		{
			FinishGame();
			return;
		}

		// Check if last object has passed
		ObjectState*const* lastObj = &m_beatmap->GetLinearObjects().back();
		MapTime timePastEnd = m_lastMapTime - (*lastObj)->time;
		if (timePastEnd > 250)
		{
			FinishGame();
		}
	}

	bool IsPlaying() const override
	{
		return m_playing;
	}

	bool GetTickRate(int32& rate) override
	{
		if (!m_audioPlayback.IsPaused())
		{
			rate = m_fpsTarget;
			return true;
		}
		return false; // Default otherwise
	}

	Texture GetJacketImage() override
	{
		return m_jacketTexture;
	}

	std::shared_ptr<Beatmap> GetBeatmap() override
	{
		return m_beatmap;
	}

	class Track& GetTrack() override
	{
		return *m_track;
	}

	class Camera& GetCamera() override
	{
		return m_camera;
	}

	class BeatmapPlayback& GetPlayback() override
	{
		return m_playback;
	}

	class Scoring& GetScoring() override
	{
		return m_scoring;
	}

	float* GetGaugeSamples() override
	{
		return m_gaugeSamples;
	}

	const String& GetMapRootPath() const override
	{
		return m_mapRootPath;
	}

	const String& GetMapPath() const override
	{
		return m_mapPath;
	}

	const DifficultyIndex& GetDifficultyIndex() const override
	{
		return m_diffIndex;
	}
};

Game* Game::Create(const DifficultyIndex& difficulty)
{
	auto impl = new Game_Impl(difficulty);
	return impl;
}

Game* Game::Create(const String& difficulty)
{
	auto impl = new Game_Impl(difficulty);
	return impl;
}
