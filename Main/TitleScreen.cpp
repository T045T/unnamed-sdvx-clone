#include "stdafx.h"
#include "TitleScreen.hpp"
#include "Application.hpp"
#include "TransitionScreen.hpp"
#include "SettingsScreen.hpp"
#include <Shared/Profiling.hpp>
#include "Scoring.hpp"
#include "SongSelect.hpp"
#include <Audio/Audio.hpp>
#include "Track.hpp"
#include "Camera.hpp"
#include "Background.hpp"
#include <GUI/GUI.hpp>
#include <GUI/CommonGUIStyle.hpp>
#include <GUI/Button.hpp>
#include <GUI/Slider.hpp>
#include <GUI/ScrollBox.hpp>
#include <GUI/SettingsBar.hpp>
#include <GUI/Spinner.hpp>
#include "HealthGauge.hpp"
#include "Shared/Jobs.hpp"
#include "ScoreScreen.hpp"
#include "Shared/Enum.hpp"
#include "Global.hpp"

class TitleScreen_Impl : public TitleScreen
{
private:
	std::shared_ptr<CommonGUIStyle> m_guiStyle;
	std::shared_ptr<Canvas> m_canvas;


	void Exit()
	{
		g_application->Shutdown();
	}

	void Start()
	{
		g_application->AddTickable(SongSelect::Create());
	}

	void Settings()
	{
		g_application->AddTickable(SettingsScreen::Create());
	}

public:
	bool Init()
	{
		m_guiStyle = g_commonGUIStyle;
		m_canvas = std::make_shared<Canvas>();

		//GUI Buttons
		{
			auto box = std::make_shared<LayoutBox>();
			Canvas::Slot* slot = m_canvas->Add(box);
			slot->anchor = Anchor(0.5f, 0.5f);
			slot->autoSizeX = true;
			slot->autoSizeY = true;
			slot->alignment = Vector2(0.5, 0.5);

			box->layoutDirection = LayoutBox::Vertical;

			auto titleLabel = std::make_shared<Label>();
			titleLabel->SetText(L"unnamed_sdvx_clone");
			titleLabel->SetFontSize(100);
			box->Add(titleLabel);

			LayoutBox::Slot* btnSlot;
			auto startBtn = std::make_shared<Button>(m_guiStyle);
			startBtn->OnPressed.Add(this, &TitleScreen_Impl::Start);
			startBtn->SetText(L"Start");
			startBtn->SetFontSize(32);
			btnSlot = box->Add(startBtn);
			btnSlot->padding = Margin(2);
			btnSlot->fillX = true;

			auto settingsBtn = std::make_shared<Button>(m_guiStyle);
			settingsBtn->OnPressed.Add(this, &TitleScreen_Impl::Settings);
			settingsBtn->SetText(L"Settings");
			settingsBtn->SetFontSize(32);
			btnSlot = box->Add(settingsBtn);
			btnSlot->padding = Margin(2);
			btnSlot->fillX = true;

			auto exitBtn = std::make_shared<Button>(m_guiStyle);
			exitBtn->OnPressed.Add(this, &TitleScreen_Impl::Exit);
			exitBtn->SetText(L"Exit");
			exitBtn->SetFontSize(32);
			btnSlot = box->Add(exitBtn);
			btnSlot->padding = Margin(2);
			btnSlot->fillX = true;
		}

		return true;
	}

	~TitleScreen_Impl()
	{}


	virtual void OnSuspend()
	{
		g_rootCanvas->Remove(std::dynamic_pointer_cast<GUIElementBase>(m_canvas));
	}

	virtual void OnRestore()
	{
		Canvas::Slot* slot = g_rootCanvas->Add(std::dynamic_pointer_cast<GUIElementBase>(m_canvas));
		slot->anchor = Anchors::Full;
		g_gameWindow->SetCursorVisible(true);
	}
};

TitleScreen* TitleScreen::Create()
{
	TitleScreen_Impl* impl = new TitleScreen_Impl();
	return impl;
}
