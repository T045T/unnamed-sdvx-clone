#pragma once
#include "ApplicationTickable.hpp"
#include "GameConfig.hpp"
#include "Global.hpp"
#include "GUI/SettingsBar.hpp"
#include "GUI/Panel.hpp"
#include "GUI/Button.hpp"

class SettingsScreen : public IApplicationTickable
{
public:
	bool Init() override;
	void Tick(float deltatime) override;
	void OnSuspend() override;
	void OnRestore() override;

private:
	shared_ptr<CommonGUIStyle> m_guiStyle;
	shared_ptr<Canvas> m_canvas;
	shared_ptr<SettingsBar> m_settings;
	shared_ptr<SettingBarSetting> m_sensSetting;

	Vector<String> m_speedMods = {"XMod", "MMod", "CMod"};
	Vector<String> m_laserModes = {"Keyboard", "Mouse", "Controller"};
	Vector<String> m_buttonModes = {"Keyboard", "Controller"};
	Vector<String> m_gamePads;
	Vector<String> m_skins;

	Vector<GameConfigKeys> m_keyboardKeys = {
		GameConfigKeys::Key_BTS,
		GameConfigKeys::Key_BT0,
		GameConfigKeys::Key_BT1,
		GameConfigKeys::Key_BT2,
		GameConfigKeys::Key_BT3,
		GameConfigKeys::Key_FX0,
		GameConfigKeys::Key_FX1
	};

	Vector<GameConfigKeys> m_keyboardLaserKeys = {
		GameConfigKeys::Key_Laser0Neg,
		GameConfigKeys::Key_Laser0Pos,
		GameConfigKeys::Key_Laser1Neg,
		GameConfigKeys::Key_Laser1Pos,
	};

	Vector<GameConfigKeys> m_controllerKeys = {
		GameConfigKeys::Controller_BTS,
		GameConfigKeys::Controller_BT0,
		GameConfigKeys::Controller_BT1,
		GameConfigKeys::Controller_BT2,
		GameConfigKeys::Controller_BT3,
		GameConfigKeys::Controller_FX0,
		GameConfigKeys::Controller_FX1
	};

	Vector<GameConfigKeys> m_controllerLaserKeys = {
		GameConfigKeys::Controller_Laser0Axis,
		GameConfigKeys::Controller_Laser1Axis,

	};

	shared_ptr<Button> m_buttonButtons[7];
	shared_ptr<Button> m_laserButtons[2];
	shared_ptr<Panel> m_laserColorPanels[2];

	Texture m_whiteTex;

	int m_speedMod = 0;
	int m_laserMode = 0;
	int m_buttonMode = 0;
	int m_selectedGamepad = 0;
	int m_selectedSkin = 0;
	float m_modSpeed = 400.f;
	float m_hispeed = 1.f;
	float m_laserSens = 1.0f;
	float m_masterVolume = 1.0f;
	float m_laserColors[2] = {0.25f, 0.75f};

	//TODO: Use argument instead of many functions if possible.
	void SetKey_BTA();
	void SetKey_BTB();
	void SetKey_BTC();
	void SetKey_BTD();
	void SetKey_FXL();
	void SetKey_FXR();
	void SetKey_ST();
	void SetLL();
	void SetRL();

	void CalibrateSens();
	void SetSens(float sens);

	void Exit();
};

class ButtonBindingScreen : public IApplicationTickable
{
public:
	ButtonBindingScreen(GameConfigKeys key, bool gamepad = false, int controllerindex = 0);

	bool Init() override;
	void Tick(float deltatime) override;
	void OnKeyPressed(int32 key) override;
	void OnSuspend() override;
	void OnRestore() override;

	void OnButtonPressed(uint8 key);

private:
	shared_ptr<CommonGUIStyle> m_guiStyle;
	shared_ptr<Canvas> m_canvas;
	shared_ptr<Gamepad> m_gamepad;
	shared_ptr<Label> m_prompt;
	GameConfigKeys m_key;
	bool m_isGamepad;
	int m_gamepadIndex;
	bool m_completed = false;
	bool m_knobs = false;
	Vector<float> m_gamepadAxes;
};

class LaserSensCalibrationScreen : public IApplicationTickable
{
public:
	~LaserSensCalibrationScreen();

	Delegate<float> SensSet;

	bool Init() override;
	void Tick(float deltatime) override;
	void OnKeyPressed(int32 key) override;
	void OnSuspend() override;
	void OnRestore() override;

	void OnButtonPressed(Input::Button button);

private:
	shared_ptr<CommonGUIStyle> m_guiStyle;
	shared_ptr<Canvas> m_canvas;
	shared_ptr<Gamepad> m_gamepad;
	shared_ptr<Label> m_prompt;
	bool m_state = false;
	float m_delta = 0.f;
	float m_currentSetting = 0.f;
	bool m_firstStart = false;
};
