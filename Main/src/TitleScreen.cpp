#include "stdafx.h"
#include "TitleScreen.hpp"
#include "GUI/Button.hpp"
#include "GUI/LayoutBox.hpp"
#include "GUI/Label.hpp"
#include "Global.hpp"
#include "SettingsScreen.hpp"

bool TitleScreen::Init()
{
	m_guiStyle = g_commonGUIStyle;
	m_canvas = make_shared<Canvas>();

	//GUI Buttons
	{
		auto box = make_shared<LayoutBox>();
		Canvas::Slot* slot = m_canvas->Add(box);
		slot->anchor = Anchor(0.5f, 0.5f);
		slot->autoSizeX = true;
		slot->autoSizeY = true;
		slot->alignment = Vector2(0.5, 0.5);

		box->layoutDirection = LayoutBox::Vertical;

		auto titleLabel = make_shared<Label>();
		titleLabel->SetText(L"unnamed_sdvx_clone");
		titleLabel->SetFontSize(100);
		box->Add(titleLabel);

		LayoutBox::Slot* btnSlot;
		auto startBtn = make_shared<Button>(m_guiStyle);
		startBtn->OnPressed.Add(&TitleScreen::Start);
		startBtn->SetText(L"Start");
		startBtn->SetFontSize(32);
		btnSlot = box->Add(startBtn);
		btnSlot->padding = Margin(2);
		btnSlot->fillX = true;

		auto settingsBtn = make_shared<Button>(m_guiStyle);
		settingsBtn->OnPressed.Add(&TitleScreen::Settings);
		settingsBtn->SetText(L"Settings");
		settingsBtn->SetFontSize(32);
		btnSlot = box->Add(settingsBtn);
		btnSlot->padding = Margin(2);
		btnSlot->fillX = true;

		auto exitBtn = make_shared<Button>(m_guiStyle);
		exitBtn->OnPressed.Add(&TitleScreen::Exit);
		exitBtn->SetText(L"Exit");
		exitBtn->SetFontSize(32);
		btnSlot = box->Add(exitBtn);
		btnSlot->padding = Margin(2);
		btnSlot->fillX = true;
	}

	return true;
}

void TitleScreen::OnSuspend()
{
	g_rootCanvas->Remove(std::dynamic_pointer_cast<GUIElementBase>(m_canvas));
}

void TitleScreen::OnRestore()
{
	auto slot = g_rootCanvas->Add(std::dynamic_pointer_cast<GUIElementBase>(m_canvas));
	slot->anchor = Anchors::Full;
	g_gameWindow->SetCursorVisible(true);
}

void TitleScreen::Start()
{
	g_application->AddTickable(new SongSelect());
}

void TitleScreen::Settings()
{
	g_application->AddTickable(new SettingsScreen());
}

void TitleScreen::Exit()
{
	g_application->Shutdown();
}