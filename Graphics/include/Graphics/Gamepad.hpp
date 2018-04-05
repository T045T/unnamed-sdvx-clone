#pragma once
#include <SDL_joystick.h>

/*
	Gamepad Abstraction
*/
class Gamepad
{
public:
	Gamepad(uint32 deviceIndex);
	~Gamepad();

	// Gamepad button event
	Delegate<uint8> OnButtonPressed;
	// Gamepad button event
	Delegate<uint8> OnButtonReleased;

	bool GetButton(uint8 button) const;
	float GetAxis(uint8 idx) const;
	SDL_Joystick* get_joystick() const;

	uint32 NumButtons() const;
	uint32 NumAxes() const;

	void HandleInputEvent(uint32 buttonIndex, uint8 newState);
	void HandleAxisEvent(uint32 axisIndex, int16 newValue);
	void HandleHatEvent(uint32 hadIndex, uint8 newValue);

private:
	uint32 m_deviceIndex;
	SDL_Joystick* m_joystick;

	Vector<float> m_axisState;
	Vector<uint8> m_buttonStates;
};
