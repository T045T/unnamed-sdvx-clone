#pragma once
#include "ApplicationTickable.hpp"
#include "GameConfig.hpp"

class SettingsScreen : public IApplicationTickable
{
protected:
	SettingsScreen() = default;
public:
	virtual ~SettingsScreen() = default;
	static SettingsScreen* Create();
};

class ButtonBindingScreen : public IApplicationTickable
{
protected:
	ButtonBindingScreen() = default;
public:
	virtual ~ButtonBindingScreen() = default;
	static ButtonBindingScreen* Create(GameConfigKeys key, bool gamepad = false, int controllerIndex = 0);
};

class LaserSensCalibrationScreen : public IApplicationTickable
{
protected:
	LaserSensCalibrationScreen() = default;
public:
	virtual ~LaserSensCalibrationScreen() = default;
	static LaserSensCalibrationScreen* Create();
	Delegate<float> SensSet;
};
