#include "stdafx.h"
#include "Gamepad.hpp"

/**
 * \throws std::runtime_error If failed to open joystick
 */
Gamepad::Gamepad(uint32 deviceIndex)
	: m_deviceIndex(deviceIndex)
{
	Logf("Trying to open joystick %d", Logger::Info, deviceIndex);

	m_joystick = SDL_JoystickOpen(deviceIndex);
	if (!m_joystick)
	{
		Logf("Failed to open joystick %d", Logger::Error, deviceIndex);
		throw std::runtime_error("Failed to open joystick");
	}

	for (int32 i = 0; i < SDL_JoystickNumButtons(m_joystick); i++)
		m_buttonStates.Add(0);
	for (int32 i = 0; i < SDL_JoystickNumAxes(m_joystick); i++)
		m_axisState.Add(0.0f);

	const String deviceName = SDL_JoystickName(m_joystick);
	Logf("Joystick device \"%s\" opened with %d buttons and %d axes", Logger::Info,
		deviceName, m_buttonStates.size(), m_axisState.size());
}

Gamepad::~Gamepad()
{
	SDL_JoystickClose(m_joystick);
}

void Gamepad::HandleInputEvent(uint32 buttonIndex, uint8 newState)
{
	m_buttonStates[buttonIndex] = newState;
	if (newState != 0)
		OnButtonPressed.Call(buttonIndex);
	else
		OnButtonReleased.Call(buttonIndex);
}

void Gamepad::HandleAxisEvent(uint32 axisIndex, int16 newValue)
{
	m_axisState[axisIndex] = static_cast<float>(newValue) / static_cast<float>(0x7fff);
}

void Gamepad::HandleHatEvent(uint32 hadIndex, uint8 newValue)
{
	// unused for now, maybe use this if required
}

bool Gamepad::GetButton(uint8 button) const
{
	if (button >= m_buttonStates.size())
		return false;
	return m_buttonStates[button] != 0;
}

float Gamepad::GetAxis(uint8 idx) const
{
	if (idx >= m_axisState.size())
		return 0.0f;
	return m_axisState[idx];
}

SDL_Joystick* Gamepad::get_joystick() const
{
	return m_joystick;
}

uint32 Gamepad::NumButtons() const
{
	return static_cast<uint32>(m_buttonStates.size());
}

uint32 Gamepad::NumAxes() const
{
	return static_cast<uint32>(m_axisState.size());
}