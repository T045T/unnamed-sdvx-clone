#include "stdafx.h"
#include "SettingsBar.hpp"
#include "Label.hpp"
#include "Slider.hpp"
#include "Button.hpp"
#include "GUIRenderer.hpp"

SettingsBar::SettingsBar(std::shared_ptr<CommonGUIStyle> style)
	: ScrollBox(style)
{
	m_style = style;
	m_container = std::make_shared<LayoutBox>();
	m_container->layoutDirection = LayoutBox::Vertical;
	ScrollBox::SetContent(m_container);
}

SettingsBar::~SettingsBar()
{
	ClearSettings();
}

void SettingsBar::PreRender(GUIRenderData rd, GUIElementBase*& inputElement)
{
	rd.area = padding.Apply(rd.area);
	ScrollBox::PreRender(rd, inputElement);

	if (!m_shown)
	{
		GUIElementBase* dummy = nullptr;
		ScrollBox::PreRender(rd, dummy);
	}
	else
	{
		ScrollBox::PreRender(rd, inputElement);
	}
}

void SettingsBar::Render(GUIRenderData rd)
{
	if (m_shown)
	{
		// Background
		rd.guiRenderer->RenderRect(rd.area, Color(0.2f, 0.2f, 0.2f, 0.8f));
		rd.area = padding.Apply(rd.area);
		// Content
		ScrollBox::Render(rd);
	}
}

SettingBarSetting* SettingsBar::AddSetting(float* target, float min, float max, const String& name)
{
	SettingBarSetting* setting = new SettingBarSetting();
	setting->name = Utility::ConvertToWString(name);
	const wchar_t* nw = *setting->name;
	const char* na = *name;
	setting->floatSetting.target = target;
	setting->floatSetting.min = min;
	setting->floatSetting.max = max;
	float v = (target[0] - min) / (max - min);

	auto box = std::make_shared<LayoutBox>();
	box->layoutDirection = LayoutBox::Vertical;
	LayoutBox::Slot* slot = m_container->Add(box);
	slot->fillX = true;

	{
		// The label
		auto label = setting->label = std::make_shared<Label>();
		box->Add(label);

		// The slider
		auto slider = std::make_shared<Slider>(m_style);
		slider->SetValue(v);
		slider->OnSliding.Add(setting, &SettingBarSetting::m_SliderUpdate);
		slider->OnValueSet.Add(setting, &SettingBarSetting::m_SliderUpdate);
		LayoutBox::Slot* sliderSlot = box->Add(slider);
		sliderSlot->fillX = true;
		m_sliders.Add(setting, slider);
	}

	m_settings.Add(setting, box);
	setting->m_SliderUpdate(v); // Initial update
	return setting;
}

SettingBarSetting* SettingsBar::AddSetting(int* target, Vector<String> options, int optionsCount, const String& name)
{
	SettingBarSetting* setting = new SettingBarSetting();
	setting->name = Utility::ConvertToWString(name);
	const wchar_t* nw = *setting->name;
	const char* na = *name;
	setting->textSetting.target = target;
	setting->textSetting.options = new Vector<String>(options);
	setting->textSetting.optionsCount = optionsCount;

	auto box = std::make_shared<LayoutBox>();
	box->layoutDirection = LayoutBox::Vertical;
	LayoutBox::Slot* slot = m_container->Add(box);
	slot->fillX = true;
	auto buttonBox = std::make_shared<LayoutBox>();

	buttonBox->layoutDirection = LayoutBox::Horizontal;

	// Create Visuals
	{
		// Name Label
		auto nameLabel = std::make_shared<Label>();
		nameLabel->SetFontSize(25);
		nameLabel->SetText(Utility::WSprintf(L"%ls: ", setting->name));
		box->Add(nameLabel);

		// Prev Button
		auto prevButton = std::make_shared<Button>(m_style);
		prevButton->SetText(L"<");
		prevButton->OnPressed.Add(setting, &SettingBarSetting::m_PrevTextSetting);
		LayoutBox::Slot* prevButtonSlot = buttonBox->Add(prevButton);
		prevButtonSlot->fillX = true;

		// Value label
		auto label = setting->label = std::make_shared<Label>();
		label->SetFontSize(40);
		LayoutBox::Slot* valueLabelSlot = buttonBox->Add(label);
		valueLabelSlot->fillX = false;

		// Next Button
		auto nextButton = std::make_shared<Button>(m_style);
		nextButton->SetText(L">");
		nextButton->OnPressed.Add(setting, &SettingBarSetting::m_NextTextSetting);
		LayoutBox::Slot* nextButtonSlot = buttonBox->Add(nextButton);
		nextButtonSlot->fillX = true;
	}
	LayoutBox::Slot* buttonBoxSlot = box->Add(buttonBox);
	buttonBoxSlot->fillX = true;
	m_settings.Add(setting, box);

	setting->m_UpdateTextSetting(0);
	return setting;
}

void SettingsBar::SetValue(SettingBarSetting* setting, float value)
{
	float v = (value - setting->floatSetting.min) / (setting->floatSetting.max - setting->floatSetting.min);
	m_sliders[setting]->SetValue(v);
}

void SettingsBar::SetValue(SettingBarSetting* setting, int value)
{
	int offset = value - (*setting->textSetting.target);
	setting->m_UpdateTextSetting(offset);
}

void SettingsBar::ClearSettings()
{
	for (auto& s : m_settings)
	{
		delete s.first;
		m_container->Remove(s.second);
	}
	m_settings.clear();
}

void SettingsBar::SetShow(bool shown)
{
	m_shown = shown;
}

void SettingBarSetting::m_SliderUpdate(float val)
{
	floatSetting.target[0] = val * (floatSetting.max - floatSetting.min) + floatSetting.min;
	label->SetText(Utility::WSprintf(L"%ls (%f):", name, floatSetting.target[0]));
}

void SettingBarSetting::m_UpdateTextSetting(int change)
{
	textSetting.target[0] += change;
	textSetting.target[0] %= textSetting.optionsCount;
	if (textSetting.target[0] < 0)
		textSetting.target[0] = textSetting.optionsCount - 1;
	WString display = Utility::ConvertToWString((*textSetting.options)[textSetting.target[0]]);
	label->SetText(Utility::WSprintf(L"%ls", display));
}

void SettingBarSetting::m_PrevTextSetting()
{
	m_UpdateTextSetting(-1);
}

void SettingBarSetting::m_NextTextSetting()
{
	m_UpdateTextSetting(1);
}
