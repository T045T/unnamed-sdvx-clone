#pragma once
#include "ApplicationTickable.hpp"
#include "GUI/Canvas.hpp"
#include "GUI/CommonGUIStyle.hpp"
#include "SongSelect.hpp"

class TitleScreen : public IApplicationTickable
{
public:
	bool Init() override;
	void OnSuspend() override;
	void OnRestore() override;

private:
	shared_ptr<CommonGUIStyle> m_guiStyle;
	shared_ptr<Canvas> m_canvas;

	static void Start();
	static void Settings();
	static void Exit();
};