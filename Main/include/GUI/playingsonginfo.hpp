#pragma once
#include "stdafx.h"
#include <GUI/GUIElement.hpp>
#include <GUI/Panel.hpp>
#include <GUI/LayoutBox.hpp>
#include <GUI/Canvas.hpp>
#include "Beatmap/Beatmap.hpp"

class Game;

class SongTitleArtist : public Panel
{
public:
	SongTitleArtist(String title, String artist, class PlayingSongInfo* info);
	void PreRender(GUIRenderData rd, GUIElementBase*& inputElement) override;
	void Render(GUIRenderData rd) override;
	float progress = 0.5f;
	float BPM = 120.f;
	float hiSpeed = 1.5f;

private:

	//Sizes should add up to 0.9
	const float m_titleSize = 0.3f;
	const float m_artistSize = 0.2f;
	const float m_speedSize = 0.4f;

	WString m_title;
	WString m_artist;
	class PlayingSongInfo* m_psi;
};

class SongProgressBar : public GUIElementBase
{
public:
	SongProgressBar();
	void Render(GUIRenderData rd) override;
	Vector2 GetDesiredSize(GUIRenderData rd) override;
	float progress = 0.5f;
};

class PlayingSongInfo : public Canvas
{
public:
	PlayingSongInfo(Game& game);
	void PreRender(GUIRenderData rd, GUIElementBase*& inputElement) override;
	void Render(GUIRenderData rd) override;
	Vector2 GetDesiredSize(GUIRenderData rd) override;
	void SetProgress(float progress);
	void SetBPM(float bpm);
	void SetHiSpeed(float hiSpeed);
	void SetJacket(Texture jacket);

	Material progressMaterial;

private:


	std::shared_ptr<LayoutBox> m_layout;
	std::shared_ptr<Panel> m_jacket;
	std::shared_ptr<SongTitleArtist> m_titleArtist;
	//std::shared_ptr<Panel> m_titlePanel;
	//std::shared_ptr<Panel> m_artistPanel;
	//Text m_title;
	//Text m_artist;
	BeatmapSettings m_settings;
	float m_progress = 0.5f;
	String m_jacketPath;
	Texture m_jacketImage;
};
