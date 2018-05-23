#pragma once
#include "AsyncLoadable.hpp"
#include "Audio/Sample.hpp"
#include "Beatmap/MapDatabase.hpp"
#include "SongSelectStyle.hpp"
#include "AsyncAssetLoader.hpp"
#include "GUI/CommonGUIStyle.hpp"
#include "GUI/Canvas.hpp"
#include "GUI/Panel.hpp"
#include "GUI/LayoutBox.hpp"
#include "HealthGauge.hpp"

class ScoreScreen : public IAsyncLoadableApplicationTickable
{
public:
	ScoreScreen(class Game* game);
	~ScoreScreen();

	bool AsyncLoad() override;
	bool AsyncFinalize() override;
	bool Init() override;

	void OnKeyPressed(int32 key) override;
	void OnKeyReleased(int32 key) override;
	void OnSuspend() override;
	void OnRestore() override;

	void Render(float deltaTime) override;
	void Tick(float deltaTime) override;

private:
	shared_ptr<CommonGUIStyle> m_guiStyle;
	shared_ptr<Canvas> m_canvas;
	shared_ptr<HealthGauge> m_gauge;
	shared_ptr<Panel> m_jacket;
	shared_ptr<Canvas> m_timingStatsCanvas;
	shared_ptr<LayoutBox> m_itemBox;
	MapDatabase m_mapDatabase;

	// Things for score screen
	Font m_specialFont;
	Sample m_applause;
	Texture m_categorizedHitTextures[4];

	bool m_autoplay;
	bool m_autoButtons;
	bool m_startPressed;
	uint32 m_score;
	uint32 m_maxCombo;
	uint32 m_categorizedHits[3];
	float m_finalGaugeValue;
	float* m_gaugeSamples;
	String m_jacketPath;
	uint32 m_timedHits[2];
	float m_meanHitDelta;
	MapTime m_medianHitDelta;

	shared_ptr<SongSelectStyle> m_songSelectStyle;

	BeatmapSettings m_beatmapSettings;
	Texture m_jacketImage;
	Texture m_graphTex;

	AsyncAssetLoader loader;
};
