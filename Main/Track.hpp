#pragma once
#include "Scoring.hpp"
#include "AsyncLoadable.hpp"

/*
	The object responsible for drawing the track.
*/
class Track : Unique, public IAsyncLoadable
{
public:
	// Size constants of various elements
	static const float trackWidth;
	static const float buttonWidth;
	static const float laserWidth;
	static const float fxbuttonWidth;
	static const float buttonTrackWidth;

	float trackLength;
	float trackTickLength;
	float buttonLength;
	float fxbuttonLength;

	// Laser color setting
	Color laserColors[2] = {};

	// Hit effect color settings
	// 0 = Miss
	// 1 = Good
	// 2 = Perfect
	// 3 = Idle
	Color hitColors[4] = {};

	class AsyncAssetLoader* loader = nullptr;

public:
	Track();
	~Track();
	virtual bool AsyncLoad() override;
	virtual bool AsyncFinalize() override;
	void Tick(class BeatmapPlayback& playback, float deltaTime);

	// Just the board with tick lines
	void DrawBase(RenderQueue& rq);
	// Draws an object
	void DrawObjectState(RenderQueue& rq, class BeatmapPlayback& playback, ObjectState* obj, bool active = false);
	// Things like the laser pointers, hit bar and effect
	void DrawOverlays(RenderQueue& rq);
	// Draws a plane over the track
	void DrawTrackOverlay(RenderQueue& rq, Texture texture, float heightOffset = 0.05f, float widthScale = 1.0f);
	// Draws the dark overlay behind the critical line
	void DrawDarkTrack(RenderQueue& rq);
	// Draw a centered sprite at pos, relative from the track
	void DrawSprite(RenderQueue& rq, Vector3 pos, Vector2 size, Texture tex, Color color = Color::White,
					float tilt = 0.0f);
	void DrawCombo(RenderQueue& rq, uint32 score, Color color, float scale = 1.0f);

	Vector3 TransformPoint(const Vector3& p);

	// Adds a sprite effect to the track
	struct TimedEffect* AddEffect(struct TimedEffect* effect);
	void ClearEffects();

	void SetViewRange(float newRange);
	void SendLaserAlert(uint8 laserIdx);
	void SetLaneHide(bool hidden, double duration);
	float GetViewRange() const;

	// Normal/FX button X-axis placement
	float GetButtonPlacement(uint32 buttonIdx);

	// Laser positions, as shown on the overlay
	float laserPositions[2];
	// Current lasers are extended
	bool lasersAreExtend[2] = {false, false};
	float laserPointerOpacity[2] = {1.0f};
	float laserAlertOpacity[2] = {1.0f};

	float laserSpeedOffset = 0.90;

	float zoomTop = .0f;
	float zoomBottom = .0f;
	float roll = .0f;
	Vector3 shakeOffset;

	// Visible time elements on the playfield track
	// a single unit is 1 beat in distance
	Vector2 trackViewRange;

	/* Track base graphics */
	Mesh trackMesh;
	Mesh trackDarkMesh;
	Mesh trackTickMesh;
	Material trackMaterial; // Also used for buttons
	Texture trackTexture;
	Texture trackDarkTexture;
	Texture trackTickTexture;

	/* Object graphics */
	Mesh buttonMesh;
	Texture buttonTexture;
	Texture buttonHoldTexture;
	Mesh fxbuttonMesh;
	Texture fxbuttonTexture;
	Texture fxbuttonHoldTexture;
	Material holdButtonMaterial;
	Material buttonMaterial;
	Texture laserTexture;
	Texture laserTailTextures[2]; // Entry and exit textures
	Material laserMaterial;
	Texture laserAlertTextures[2];

	/* Overlay graphics */
	Material trackOverlay;

	/* Scoring and feedback elements */
	Texture scoreBarTexture;
	Texture scoreHitTexture;
	Texture laserPointerTexture;
	Texture scoreHitTextures[3];  // Ok, Miss, Perfect
	Texture scoreTimeTextures[2]; // Early, Late
	// Combo counter sprite sheet
	Texture comboSpriteSheet;
	Mesh comboSpriteMeshes[10];
	/* Reusable sprite mesh and material */
	Mesh centeredTrackMesh;
	Material spriteMaterial;

	// For flickering objects, like hold objects that are active
	float objectGlow;
	// 20Hz flickering. 0 = Miss, 1 = Inactive, 2 & 3 = Active alternating.
	int objectGlowState;

	// Early/Late indicator
	struct TimedHitEffect* timedHitEffect;

	// Track Origin position
	Transform trackOrigin;

private:
	// Laser track generators
	class LaserTrackBuilder* m_laserTrackBuilder[2] = {0};

	const TimingPoint* m_lastTimingPoint;

	// Bar tick locations
	Vector<float> m_barTicks;

	// Active effects
	Vector<struct TimedEffect*> m_hitEffects;

	// Distance of seen objects on the track
	float m_viewRange;

	MapTime m_lastMapTime = 0;

	float m_alertTimer[2] = {10.0f, 10.0f};

	// Camera variables Landscape, Portrait
	float m_basePitch[2] = {-35.f, -47.f};
	float m_baseRadius[2] = {0.3f, 0.275f};
	float m_pitchOffset[2] = {0.05f, 0.265f}; // how far from the bottom of the screen should the crit line be
	float m_fov[2] = {70.f, 90.f};


	// How much the track is hidden. 1.0 = fully hidden, 0.0 = fully visible
	float m_trackHide = 0.0f;
	float m_trackHideSpeed = 0.0f;
};

// Base class for sprite effects on the track
struct TimedEffect
{
	TimedEffect(float duration);;
	virtual ~TimedEffect() = default;
	void Reset(float duration);

	float GetRate() const
	{
		return time / duration;
	}

	virtual void Draw(class RenderQueue& rq) = 0;
	virtual void Tick(float deltaTime);

	Track* track;
	float duration;
	float time;
};

// Button hit effect
struct ButtonHitEffect : public TimedEffect
{
	ButtonHitEffect(uint32 buttonCode, Color color);;
	virtual void Draw(class RenderQueue& rq) override;

	uint32 buttonCode;
	Color color;
};

// Button hit rating effect
struct ButtonHitRatingEffect : public TimedEffect
{
	ButtonHitRatingEffect(uint32 buttonCode, ScoreHitRating rating);;
	virtual void Draw(class RenderQueue& rq) override;

	uint32 buttonCode;
	ScoreHitRating rating;
};

struct TimedHitEffect : public TimedEffect
{
	TimedHitEffect(bool late);
	virtual void Draw(class RenderQueue& rq) override;

	bool late;
};
