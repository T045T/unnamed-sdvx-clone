#pragma once
#include "GUIElement.hpp"
#include "LayoutBox.hpp"
#include "ScrollBox.hpp"
#include "CommonGUIStyle.hpp"

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
	class Label* label;

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

	SettingBarSetting* AddSetting(float* target, float min, float max, const String& name);
	SettingBarSetting* AddSetting(int* target, Vector<String> options, int optionsCount, const String& name);
	void SetValue(SettingBarSetting* setting, float value);
	void SetValue(SettingBarSetting* setting, int value);
	void ClearSettings();

	void SetShow(bool shown);

	bool IsShown() const
	{
		return m_shown;
	}

	Margini padding = Margini(5, 5, 0, 5);

private:
	bool m_shown = true;
	class LayoutBox* m_container;
	std::shared_ptr<CommonGUIStyle> m_style;
	Map<SettingBarSetting*, GUIElement> m_settings;
	Map<SettingBarSetting*, Slider*> m_sliders;
};
