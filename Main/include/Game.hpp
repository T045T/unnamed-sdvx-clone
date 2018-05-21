#pragma once
#include "AsyncLoadable.hpp"
#include <Beatmap/MapDatabase.hpp>
#include "HealthGauge.hpp"
#include "PlayingSongInfo.hpp"
#include "GUI/CommonGUIStyle.hpp"
#include "GUI/SettingsBar.hpp"
#include "AsyncAssetLoader.hpp"
#include "Audio/Sample.hpp"
#include "Background.hpp"
#include "Track.hpp"
#include "AudioPlayback.hpp"
#include "Camera.hpp"

/*
	Main game scene / logic manager
*/
class Game : public IAsyncLoadableApplicationTickable
{
public:
	Game(const DifficultyIndex& difficulty);
	Game(const String& mapPath);
	~Game();

	bool AsyncLoad() override;
	bool AsyncFinalize() override;
	bool Init() override;
	void Tick(float deltaTime) override;
	void Render(float deltaTime) override;
	void OnKeyPressed(int32 key) override;
	bool GetTickRate(int32& rate) override;

	// When the game is still going, false when the map is done, all ending sequences have played, etc.
	// also false when the player leaves the game
	bool IsPlaying() const;

	class Track& GetTrack();
	class Camera& GetCamera();
	class BeatmapPlayback& GetPlayback();
	class Scoring& GetScoring();

	// Samples of the gauge for the performance graph
	float* GetGaugeSamples();

	// Map jacket image
	Texture GetJacketImage();

	// Difficulty data
	const DifficultyIndex& GetDifficultyIndex() const;

	// The beatmap
	shared_ptr<class Beatmap> GetBeatmap();

	// The folder that contians the map
	const String& GetMapRootPath() const;

	// Full path to map
	const String& GetMapPath() const;

private:
	// Startup parameters
	String m_mapRootPath;
	String m_mapPath;
	DifficultyIndex m_diffIndex;

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
	shared_ptr<Canvas> m_canvas;
	shared_ptr<HealthGauge> m_scoringGauge;
	shared_ptr<PlayingSongInfo> m_psi;
	shared_ptr<SettingsBar> m_settingsBar;
	shared_ptr<CommonGUIStyle> m_guiStyle;
	shared_ptr<Label> m_scoreText;

	Font m_fontDivlit;

	// Texture of the map jacket image, if available
	Image m_jacketImage;
	Texture m_jacketTexture;

	// Combo colors
	Color m_comboColors[3];

	// The beatmap
	shared_ptr<Beatmap> m_beatmap;
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
	shared_ptr<ParticleEmitter> m_laserFollowEmitters[2];
	shared_ptr<ParticleEmitter> m_holdEmitters[6];

	AsyncAssetLoader loader;

	void m_OnButtonPressed(Input::Button buttonCode);

	bool InitHUD();
	bool InitSFX();
	bool InitGameplay();
	bool SkipIntro();
	void TickGameplay(float deltaTime);
	void ApplyAudioLeadin();
	void RenderDebugHUD(float deltaTime);
	void SkipOutro();
	void FinishGame();
	void Restart();

	shared_ptr<ParticleEmitter> CreateTrailEmitter(const Color& color);
	shared_ptr<ParticleEmitter> CreateHoldEmitter(const Color& color, float width);
	shared_ptr<ParticleEmitter> CreateExplosionEmitter(const Color& color, Vector3 dir);
	shared_ptr<ParticleEmitter> CreateHitEmitter(const Color& color, float width);

	void OnLaserSlamHit(LaserObjectState* object);
	void OnButtonHit(Input::Button button, ScoreHitRating rating, ObjectState* hitObject, bool late);
	void OnButtonMiss(Input::Button button, bool hitEffect);
	void OnObjectHold(Input::Button, ObjectState* object);
	void OnObjectReleased(Input::Button, ObjectState* object);
	void OnLaneToggleChanged(LaneHideTogglePoint* tp);
	void OnEventChanged(EventKey key, EventData data);
	void OnFXBegin(HoldObjectState* object);
	void OnFXEnd(HoldObjectState* object);
};
