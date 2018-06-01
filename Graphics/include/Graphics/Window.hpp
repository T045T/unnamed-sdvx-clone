#pragma once
#include <Graphics/Keys.hpp>
#include <Graphics/Gamepad.hpp>
#include <Graphics/KeyMap.hpp>
#include <SDL2/SDL.h>

namespace Graphics
{
	// Windowed or bordered window style
	enum class WindowStyle
	{
		Windowed,
		Borderless
	};

	// Text input data
	struct TextComposition
	{
		WString composition;
		int32 cursor;
		int32 selectionLength;
	};

	/*
		Simple window class that manages window messages, window style and input
		Renamed from Window to DesktopWindow to avoid conflicts with libX11 on Linux
	*/
	class Window : Unique
	{
	public:
		Window(Vector2i size = Vector2i(800, 600));
		~Window();

		void Show() const;
		void Hide() const;

		bool Update();
		void* Handle() const;
		void SetCaption(const WString& cap);
		void Close();

		Vector2i GetMousePos() const;
		void set_mouse_pos(const Vector2i& pos) const;
		void SetRelativeMouseMode(bool enabled) const;
		bool GetRelativeMouseMode() const;

		// Sets cursor to use
		void SetCursor(shared_ptr<class ImageRes> image, Vector2i hotspot = Vector2i(0, 0));
		void SetCursorVisible(bool visible) const;

		void SetWindowStyle(WindowStyle style);

		Vector2i get_window_pos() const;
		void set_window_pos(const Vector2i& pos) const;

		Vector2i GetWindowSize() const;

		void SetVSync(int8 setting) const;

		// Window is active
		bool IsActive() const;
		// Set window client area size
		void SetWindowSize(const Vector2i& size) const;
		void SwitchFullscreen(uint32 monitorID = -1);
		bool IsFullscreen() const;

		ModifierKeys GetModifierKeys() const;

		// Start allowing text input
		void StartTextInput() const;
		// Stop allowing text input
		void StopTextInput() const;
		// Used to get current IME working data
		const TextComposition& GetTextComposition() const;

		// Get the text currently in the clipboard
		WString GetClipboard() const;

		// The number of available gamepad devices
		int32 GetNumGamepads() const;

		// List of gamepad device names
		Vector<String> GetGamepadDeviceNames() const;

		// Open a gamepad within the range of the number of gamepads
		shared_ptr<Gamepad> OpenGamepad(int32 deviceIndex);

		Delegate<int32> OnKeyPressed;
		Delegate<int32> OnKeyReleased;
		Delegate<MouseButton> OnMousePressed;
		Delegate<MouseButton> OnMouseReleased;
		Delegate<int32, int32> OnMouseMotion;
		// Mouse scroll wheel 
		//	Positive for scroll down
		//	Negative for scroll up
		Delegate<int32> OnMouseScroll;
		// Called for the initial an repeating presses of a key
		Delegate<int32> OnKeyRepeat;
		Delegate<const WString&> OnTextInput;
		Delegate<const TextComposition&> OnTextComposition;
		Delegate<const Vector2i&> OnResized;

	private:
		SDL_Window* m_window;

		SDL_Cursor* currentCursor = nullptr;

		// Window Input State
		Map<SDL_Keycode, uint8> m_keyStates;
		KeyMap m_keyMapping;
		ModifierKeys m_modKeys = ModifierKeys::None;

		// Gamepad input
		Map<int32, shared_ptr<Gamepad>> m_gamepads;
		Map<SDL_JoystickID, shared_ptr<Gamepad>> m_joystickMap;

		// Text input / IME stuff
		TextComposition m_textComposition;

		// Various window state
		bool m_active = true;
		bool m_closed = false;
		bool m_fullscreen = false;
		uint32 m_style;
		Vector2i m_clntSize;
		WString m_caption = L"Window";

		void HandleKeyEvent(SDL_Keycode code, uint8 newState, int32 repeat);
	};
}
