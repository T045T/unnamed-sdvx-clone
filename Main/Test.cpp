#include "stdafx.h"
#include "Test.hpp"
#include "Application.hpp"
#include <Shared/Profiling.hpp>
#include "Scoring.hpp"
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

#ifdef _WIN32
#include "SDL_keycode.h"
#else
#include "SDL2/SDL_keycode.h"
#endif

class Test_Impl : public Test
{
private:
	std::shared_ptr<CommonGUIStyle> m_guiStyle;
	std::shared_ptr<SettingsBar> m_settings;

	WString m_currentText;
	float a = 0.1f; // 0 - 1
	float b = 2.0f; // 0 - 10
	float c = 1.0f; // 0 - 5
	float d = 0.0f; // -2 - 2
	int e = 0;
	std::shared_ptr<Gamepad> m_gamepad;
	Vector<String> m_textSettings;

public:
	static void StaticFunc(int32 arg)
	{ }

	static int32 StaticFunc1(int32 arg)
	{
		return arg * 2;
	}

	static int32 StaticFunc2(int32 arg)
	{
		return arg * 2;
	}

	bool Init()
	{
		m_guiStyle = g_commonGUIStyle;

		m_gamepad = g_gameWindow->OpenGamepad(0);

		{
			auto box0 = std::make_shared<ScrollBox>(m_guiStyle);
			Canvas::Slot* slot = g_rootCanvas->Add(box0);
			slot->anchor = Anchor(0.0f, 0.0f);
			slot->offset = Rect(Vector2(10.0f, 10.0f), Vector2(500, 500.0f));
			slot->autoSizeX = false;
			slot->autoSizeY = false;

			{
				auto box = std::make_shared<LayoutBox>();
				box->layoutDirection = LayoutBox::Vertical;
				box0->SetContent(box);

				LayoutBox::Slot* btnSlot;
				auto btn0 = std::make_shared<Button>(m_guiStyle);
				btn0->SetText(L"TestButton0");
				btn0->SetFontSize(32);
				btnSlot = box->Add(btn0);
				btnSlot->padding = Margin(2);

				auto btn1 = std::make_shared<Button>(m_guiStyle);
				btn1->SetText(L"This is a button with slightly\nlarger text");
				btn1->SetFontSize(32);
				btnSlot = box->Add(btn1);
				btnSlot->padding = Margin(2);

				auto fld = std::make_shared<TextInputField>(m_guiStyle);
				fld->SetText(L"textinput");
				fld->SetFontSize(32);
				btnSlot = box->Add(fld);
				btnSlot->fillX = true;
				btnSlot->padding = Margin(2);

				auto sld = std::make_shared<Slider>(m_guiStyle);
				btnSlot = box->Add(sld);
				btnSlot->fillX = true;
				btnSlot->padding = Margin(2);

				sld = std::make_shared<Slider>(m_guiStyle);
				btnSlot = box->Add(sld);
				btnSlot->fillX = true;
				btnSlot->padding = Margin(2);

				sld = std::make_shared<Slider>(m_guiStyle);
				btnSlot = box->Add(sld);
				btnSlot->fillX = true;
				btnSlot->padding = Margin(2);

				// Duplicate items
				btn0 = std::make_shared<Button>(m_guiStyle);
				btn0->SetText(L"TestButton0");
				btn0->SetFontSize(32);
				btnSlot = box->Add(btn0);
				btnSlot->padding = Margin(2);

				btn1 = std::make_shared<Button>(m_guiStyle);
				btn1->SetText(L"This is a button with slightly\nlarger text");
				btn1->SetFontSize(32);
				btnSlot = box->Add(btn1);
				btnSlot->padding = Margin(2);

				fld = std::make_shared<TextInputField>(m_guiStyle);
				fld->SetText(L"textinput");
				fld->SetFontSize(32);
				btnSlot = box->Add(fld);
				btnSlot->fillX = true;
				btnSlot->padding = Margin(2);

				sld = std::make_shared<Slider>(m_guiStyle);
				btnSlot = box->Add(sld);
				btnSlot->fillX = true;
				btnSlot->padding = Margin(2);

				sld = std::make_shared<Slider>(m_guiStyle);
				btnSlot = box->Add(sld);
				btnSlot->fillX = true;
				btnSlot->padding = Margin(2);

				sld = std::make_shared<Slider>(m_guiStyle);
				btnSlot = box->Add(sld);
				btnSlot->fillX = true;
				btnSlot->padding = Margin(2);
			}
		}

		// Setting bar
		{
			auto sb = std::make_shared<SettingsBar>(m_guiStyle);
			m_settings = std::shared_ptr<SettingsBar>(sb);
			sb->AddSetting(&a, 0.0f, 1.0f, "A");
			sb->AddSetting(&b, 0.0f, 10.0f, "B");
			sb->AddSetting(&c, 0.0f, 5.0f, "C");
			sb->AddSetting(&d, -2.0f, 2.0f, "D");
			m_textSettings.Add("Setting 1");
			m_textSettings.Add("Set 2");
			m_textSettings.Add("3");
			sb->AddSetting(&e, m_textSettings, m_textSettings.size(), "E");

			Canvas::Slot* slot = g_rootCanvas->Add(sb);
			slot->anchor = Anchor(0.75f, 0.0f, 1.0f, 1.0f);
			slot->autoSizeX = false;
			slot->autoSizeY = false;
		}

		// Spinner
		{
			auto spinner = std::make_shared<Spinner>(m_guiStyle);
			Canvas::Slot* slot = g_rootCanvas->Add(spinner);
			slot->anchor = Anchor(0.9f, 0.9f);
			slot->autoSizeX = true;
			slot->autoSizeY = true;
			slot->alignment = Vector2(1.0f, 1.0f);
		}
		return true;
	}

	~Test_Impl()
	{ }

	virtual void OnKeyPressed(int32 key) override
	{
		if (key == SDLK_TAB)
		{
			m_settings->SetShow(!m_settings->IsShown());
		}
	}

	virtual void OnKeyReleased(int32 key) override
	{ }

	virtual void Render(float deltaTime) override
	{ }

	virtual void Tick(float deltaTime) override
	{ }
};

Test* Test::Create()
{
	Test_Impl* impl = new Test_Impl();
	return impl;
}
