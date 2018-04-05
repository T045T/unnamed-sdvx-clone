#pragma once
#include "GUIElement.hpp"
#include "LayoutBox.hpp"
#include "ScrollBox.hpp"
#include "CommonGUIStyle.hpp"
#include "Label.hpp"

struct SettingBarSetting
{
	enum class Type
	{
		Float,
		Text,
		Button
	};

	Type type = Type::Float;

	union
	{
		struct
		{
			float* target;
			float min;
			float max;
		} floatSetting;

		struct
		{
			int* target;
			Vector<String>* options;
			int optionsCount;
		} textSetting;
	};

	WString name;
	std::shared_ptr<Label> label;

protected:
	friend class SettingsBar;
	void m_SliderUpdate(float val);
	void m_UpdateTextSetting(int change);
	void m_NextTextSetting();
	void m_PrevTextSetting();
};

class SettingsBar : public ScrollBox
{
public:
	SettingsBar(std::shared_ptr<CommonGUIStyle> style);
	~SettingsBar();

	virtual void PreRender(GUIRenderData rd, GUIElementBase*& inputElement) override;
	virtual void Render(GUIRenderData rd) override;

	shared_ptr<SettingBarSetting> AddSetting(float* target, float min, float max, const String& name);
	shared_ptr<SettingBarSetting> AddSetting(int* target, Vector<String> options, int optionsCount, const String& name);
	void SetValue(shared_ptr<SettingBarSetting> setting, float value);
	void SetValue(shared_ptr<SettingBarSetting> setting, int value);
	void ClearSettings();

	void SetShow(bool shown);

	bool IsShown() const
	{
		return m_shown;
	}

	Margini padding = Margini(5, 5, 0, 5);

private:
	bool m_shown = true;
	shared_ptr<LayoutBox> m_container;
	shared_ptr<CommonGUIStyle> m_style;
	Map<shared_ptr<SettingBarSetting>, GUIElement> m_settings;
	Map<shared_ptr<SettingBarSetting>, shared_ptr<Slider>> m_sliders;
};
