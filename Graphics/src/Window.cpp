#include "stdafx.h"
#include "Window.hpp"
#include "KeyMap.hpp"
#include "Image.hpp"

namespace Graphics
{
	/* SDL Instance singleton */
	class SDL
	{
	protected:
		SDL()
		{
			SDL_SetMainReady();
			int r = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
			if (r != 0)
			{
				Logf("SDL_Init Failed: %s", Logger::Error, SDL_GetError());
				assert(false);
			}
		}

	public:
		~SDL()
		{
			SDL_Quit();
		}

		static SDL& Main()
		{
			static SDL sdl;
			return sdl;
		}
	};

	Window::Window(Vector2i size)
	{
		SDL::Main();

		// Initialize button mapping
		m_keyMapping.AddRangeMapping('a', 'z', Key::A);
		m_keyMapping.AddRangeMapping('0', '9', Key::Top0);
		m_keyMapping.AddRangeMapping(SDLK_F1, SDLK_F12, Key::F1);
		m_keyMapping.AddMapping(SDLK_PRINTSCREEN, Key::PrntScr);
		m_keyMapping.AddMapping(SDLK_SCROLLLOCK, Key::ScrollLock);
		m_keyMapping.AddMapping(SDLK_PAUSE, Key::Pause);
		m_keyMapping.AddMapping(SDLK_ESCAPE, Key::Escape);
		m_keyMapping.AddMapping(SDLK_BACKQUOTE, Key::Tilde);
		m_keyMapping.AddMapping(SDLK_PAGEUP, Key::PageUp);
		m_keyMapping.AddMapping(SDLK_PAGEDOWN, Key::PageDown);
		m_keyMapping.AddMapping(SDLK_RETURN, Key::Return);
		m_keyMapping.AddMapping(SDLK_PLUS, Key::Plus);
		m_keyMapping.AddMapping(SDLK_MINUS, Key::Minus);
		m_keyMapping.AddMapping(SDLK_LEFT, Key::ArrowLeft);
		m_keyMapping.AddMapping(SDLK_RIGHT, Key::ArrowRight);
		m_keyMapping.AddMapping(SDLK_UP, Key::ArrowUp);
		m_keyMapping.AddMapping(SDLK_DOWN, Key::ArrowDown);
		m_keyMapping.AddMapping(SDLK_SPACE, Key::Space);
		m_keyMapping.AddMapping(SDLK_BACKSPACE, Key::Backspace);
		m_keyMapping.AddMapping(SDLK_TAB, Key::Tab);

		m_clntSize = size;

		m_caption = L"Window";
		String titleUtf8 = Utility::ConvertToUTF8(m_caption);
		m_window = SDL_CreateWindow(*titleUtf8, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			m_clntSize.x, m_clntSize.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		assert(m_window);

		uint32 numJoysticks = SDL_NumJoysticks();
		if (numJoysticks == 0)
		{
			Logf("No joysticks found", Logger::Warning);
		}
		else
		{
			Logf("Listing %d Joysticks:", Logger::Info, numJoysticks);
			for (uint32 i = 0; i < numJoysticks; i++)
			{
				SDL_Joystick* joystick = SDL_JoystickOpen(i);
				if (!joystick)
				{
					Logf("[%d] <failed to open>", Logger::Warning, i);
					continue;
				}
				String deviceName = SDL_JoystickName(joystick);

				Logf("[%d] \"%s\" (%d buttons, %d axes, %d hats)", Logger::Info,
					i, deviceName, SDL_JoystickNumButtons(joystick), SDL_JoystickNumAxes(joystick), SDL_JoystickNumHats(joystick));

				SDL_JoystickClose(joystick);
			}
		}
	}

	Window::~Window()
	{
		// Release gamepads
		for (auto it : m_gamepads)
			it.second.reset();

		SDL_DestroyWindow(m_window);
	}

	/**
	 * \brief Show window
	 */
	void Window::Show() const
	{
		SDL_ShowWindow(m_window);
	}

	/**
	 * \brief Hide window
	 */
	void Window::Hide() const
	{
		SDL_HideWindow(m_window);
	}

	/**
	 * \brief Call every frame to update the window message loop
	 * \return false if the window received a close message
	 */
	bool Window::Update()
	{
		SDL_Event evt;
		while (SDL_PollEvent(&evt))
		{
			if (evt.type == SDL_KEYDOWN)
			{
				if (m_textComposition.composition.empty())
				{
					// Ignore key input when composition is being typed
					HandleKeyEvent(evt.key.keysym.sym, 1, evt.key.repeat);
				}
			}
			else if (evt.type == SDL_KEYUP)
			{
				HandleKeyEvent(evt.key.keysym.sym, 0, 0);
			}
			else if (evt.type == SDL_JOYBUTTONDOWN)
			{
				const auto gp = m_joystickMap.Find(evt.jbutton.which);
				if (gp)
					gp[0]->HandleInputEvent(evt.jbutton.button, true);
			}
			else if (evt.type == SDL_JOYBUTTONUP)
			{
				const auto gp = m_joystickMap.Find(evt.jbutton.which);
				if (gp)
					gp[0]->HandleInputEvent(evt.jbutton.button, false);
			}
			else if (evt.type == SDL_JOYAXISMOTION)
			{
				const auto gp = m_joystickMap.Find(evt.jaxis.which);
				if (gp)
					gp[0]->HandleAxisEvent(evt.jaxis.axis, evt.jaxis.value);
			}
			else if (evt.type == SDL_JOYHATMOTION)
			{
				const auto gp = m_joystickMap.Find(evt.jhat.which);
				if (gp)
					gp[0]->HandleHatEvent(evt.jhat.hat, evt.jhat.value);
			}
			else if (evt.type == SDL_MOUSEBUTTONDOWN)
			{
				switch (evt.button.button)
				{
				case SDL_BUTTON_LEFT:
					OnMousePressed.Call(MouseButton::Left);
					break;
				case SDL_BUTTON_MIDDLE:
					OnMousePressed.Call(MouseButton::Middle);
					break;
				case SDL_BUTTON_RIGHT:
					OnMousePressed.Call(MouseButton::Right);
					break;
				default:
					break;
				}
			}
			else if (evt.type == SDL_MOUSEBUTTONUP)
			{
				switch (evt.button.button)
				{
				case SDL_BUTTON_LEFT:
					OnMouseReleased.Call(MouseButton::Left);
					break;
				case SDL_BUTTON_MIDDLE:
					OnMouseReleased.Call(MouseButton::Middle);
					break;
				case SDL_BUTTON_RIGHT:
					OnMouseReleased.Call(MouseButton::Right);
					break;
				default:
					break;
				}
			}
			else if (evt.type == SDL_MOUSEWHEEL)
			{
				if (evt.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
					OnMouseScroll.Call(evt.wheel.y);
				else
					OnMouseScroll.Call(-evt.wheel.y);
			}
			else if (evt.type == SDL_MOUSEMOTION)
			{
				OnMouseMotion.Call(evt.motion.xrel, evt.motion.yrel);
			}
			else if (evt.type == SDL_QUIT)
			{
				m_closed = true;
			}
			else if (evt.type == SDL_WINDOWEVENT)
			{
				if (evt.window.windowID == SDL_GetWindowID(m_window))
				{
					if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						Vector2i newSize(evt.window.data1, evt.window.data2);
						OnResized.Call(newSize);
					}
				}
			}
			else if (evt.type == SDL_TEXTINPUT)
			{
				WString wstr = Utility::ConvertToWString(evt.text.text);
				OnTextInput.Call(wstr);
			}
			else if (evt.type == SDL_TEXTEDITING)
			{
				SDL_Rect scr;
				SDL_GetWindowPosition(m_window, &scr.x, &scr.y);
				SDL_GetWindowSize(m_window, &scr.w, &scr.h);
				SDL_SetTextInputRect(&scr);

				m_textComposition.composition = Utility::ConvertToWString(evt.edit.text);
				m_textComposition.cursor = evt.edit.start;
				m_textComposition.selectionLength = evt.edit.length;
				OnTextComposition.Call(m_textComposition);
			}
		}
		return !m_closed;
	}

	/**
	 * \return native handle to the window
	 */
	void* Window::Handle() const
	{
		return m_window;
	}

	/**
	 * \brief Set window title
	 */
	void Window::SetCaption(const WString& cap)
	{
		m_caption = cap;
		String titleUtf8 = Utility::ConvertToUTF8(m_caption);
		SDL_SetWindowTitle(m_window, *titleUtf8);
	}

	/**
	 * \brief Close the window
	 */
	void Window::Close()
	{
		m_closed = true;
	}

	Vector2i Window::GetMousePos() const
	{
		Vector2i res;
		SDL_GetMouseState(&res.x, &res.y);
		return res;
	}

	void Window::SetCursor(shared_ptr<class ImageRes> image, Vector2i hotspot /*= Vector2i(0,0)*/)
	{
#ifdef _WIN32
		if (currentCursor)
		{
			SDL_FreeCursor(currentCursor);
			currentCursor = nullptr;
		}

		if (image)
		{
			Vector2i size = image->GetSize();
			void* bits = image->GetBits();
			SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(bits, size.x, size.y, 32, size.x * 4,
				0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
			if (surf)
				currentCursor = SDL_CreateColorCursor(surf, hotspot.x, hotspot.y);
		}
		SDL_SetCursor(currentCursor);
#endif
		/// NOTE: Cursor transparency is broken on linux
	}

	void Window::SetCursorVisible(bool visible) const
	{
		SDL_ShowCursor(visible);
	}

	/**
	 * \brief Switches between borderless and windowed
	 */
	void Window::SetWindowStyle(WindowStyle style) const
	{
		SetWindowStyle(style);
	}

	/**
	 * \brief Get full window position
	 */
	Vector2i Window::get_window_pos() const
	{
		Vector2i res;
		SDL_GetWindowPosition(m_window, &res.x, &res.y);
		return res;
	}

	/**
	 * \brief Set full window position
	 */
	void Window::set_window_pos(const Vector2i& pos) const
	{
		SDL_SetWindowPosition(m_window, pos.x, pos.y);
	}

	/**
	 * \brief Window Client area size
	 */
	Vector2i Window::GetWindowSize() const
	{
		Vector2i res;
		SDL_GetWindowSize(m_window, &res.x, &res.y);
		return res;
	}

	void Window::SetVSync(int8 setting) const
	{
		if (SDL_GL_SetSwapInterval(setting) == -1)
			Logf("Failed to set VSync: %s", Logger::Error, SDL_GetError());
	}

	void Window::SetWindowSize(const Vector2i& size) const
	{
		SDL_SetWindowSize(m_window, size.x, size.y);
	}

	/**
	 * \brief Switches to fullscreen
	 * \param monitorID TODO: unused
	 */
	void Window::SwitchFullscreen(uint32 monitorID)
	{
		if (m_fullscreen)
		{
			SDL_SetWindowFullscreen(m_window, 0);
			m_fullscreen = false;
		}
		else
		{
			SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
			m_fullscreen = true;
		}
	}

	bool Window::IsFullscreen() const
	{
		return m_fullscreen;
	}

	ModifierKeys Window::GetModifierKeys() const
	{
		return m_modKeys;
	}

	bool Window::IsActive() const
	{
		return SDL_GetWindowFlags(m_window) & SDL_WindowFlags::SDL_WINDOW_INPUT_FOCUS;
	}

	void Window::StartTextInput() const
	{
		SDL_StartTextInput();
	}

	void Window::StopTextInput() const
	{
		SDL_StopTextInput();
	}

	const TextComposition& Window::GetTextComposition() const
	{
		return m_textComposition;
	}

	WString Window::GetClipboard() const
	{
		char* utf8Clipboard = SDL_GetClipboardText();
		WString ret = Utility::ConvertToWString(utf8Clipboard);
		SDL_free(utf8Clipboard);

		return ret;
	}

	int32 Window::GetNumGamepads() const
	{
		return SDL_NumJoysticks();
	}

	Vector<String> Window::GetGamepadDeviceNames() const
	{
		Vector<String> ret;
		uint32 numJoysticks = SDL_NumJoysticks();
		for (uint32 i = 0; i < numJoysticks; i++)
		{
			SDL_Joystick* joystick = SDL_JoystickOpen(i);
			if (!joystick)
			{
				continue;
			}
			String deviceName = SDL_JoystickName(joystick);
			ret.Add(deviceName);

			SDL_JoystickClose(joystick);
		}
		return ret;
	}

	shared_ptr<Gamepad> Window::OpenGamepad(int32 deviceIndex)
	{
		auto gamepad = *m_gamepads.Find(deviceIndex);
		if (gamepad)
			return gamepad;

		try
		{
			gamepad = make_shared<Gamepad>(deviceIndex);
			SDL_JoystickEventState(SDL_ENABLE);
			m_gamepads.Add(deviceIndex, gamepad);
			m_joystickMap.Add(SDL_JoystickInstanceID(gamepad->get_joystick()), gamepad);
		}
		catch (std::runtime_error &err)
		{
			Logf("Failed to create gamepad %i (%s)", Logger::Warning, deviceIndex, err.what());
		}
		
		return gamepad;
	}

	/**
	 * \brief Handles input
	 */
	void Window::HandleKeyEvent(SDL_Keycode code, uint8 newState, int32 repeat)
	{
		SDL_Keymod m = SDL_GetModState();
		m_modKeys = ModifierKeys::None;
		if ((m & KMOD_ALT) != 0)
		{
			(uint8&)m_modKeys |= (uint8)ModifierKeys::Alt;
		}
		if ((m & KMOD_CTRL) != 0)
		{
			(uint8&)m_modKeys |= (uint8)ModifierKeys::Ctrl;
		}
		if ((m & KMOD_SHIFT) != 0)
		{
			(uint8&)m_modKeys |= (uint8)ModifierKeys::Shift;
		}

		uint8& currentState = m_keyStates[code];
		if (currentState != newState)
		{
			currentState = newState;
			if (newState == 1)
				OnKeyPressed.Call(code);
			else
				OnKeyReleased.Call(code);
		}

		if (currentState == 1)
			OnKeyRepeat.Call(code);
	}

	void Window::set_mouse_pos(const Vector2i& pos) const
	{
		SDL_WarpMouseInWindow(m_window, pos.x, pos.y);
	}

	void Window::SetRelativeMouseMode(bool enabled) const
	{
		if (SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE) != 0)
			Logf("SetRelativeMouseMode failed: %s", Logger::Severity::Warning, SDL_GetError());
	}

	bool Window::GetRelativeMouseMode() const
	{
		return SDL_GetRelativeMouseMode() == SDL_TRUE;
	}
}

namespace Graphics
{
	ImplementBitflagEnum(ModifierKeys);
}
