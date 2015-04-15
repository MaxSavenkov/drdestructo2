#pragma once

struct KeyboardEvent
{
	enum EType
	{
		KBD_KEY_DOWN,
		KBD_KEY_PRESSED,
		KBD_KEY_UP,
		KBD_KEY_REPEAT,
	};

	  // My own event type, because Allegro lacks some useful events
	EType m_type;
	  // Allegro key code
	int m_keycode;
	  // The following two fields are only valid is SOME events
	int m_flags;
	int m_unichar;
};

struct MouseEvent
{
	enum EType
	{
		MOUSE_POS,
		MOUSE_MOVE,
		MOUSE_B1_DOWN,
		MOUSE_B1_PRESSED,
		MOUSE_B1_UP,
		MOUSE_B2_DOWN,
		MOUSE_B2_PRESSED,
		MOUSE_B2_UP,
		MOUSE_B3_DOWN,
		MOUSE_B3_PRESSED,
		MOUSE_B3_UP,
		MOUSE_B1_DOUBLECLICK,
		MOUSE_WHEEL,
	};

	EType m_type;
	int m_x, m_y;
	int m_deltaWheel;
};

struct GamepadEvent
{
	enum EType
	{
		GP_AXIS,
		GP_BUTTON_UP,
		GP_BUTTON_DOWN,
	};
	EType m_type;
	
	int m_stick;
	int m_axis;
	float m_pos;
	
	int m_button;
};

struct TouchEvent
{
	enum EType
	{
		TOUCH_BEGIN,
		TOUCH_MOVE,
		TOUCH_END,
		TOUCH_CANCEL,
	};
	EType m_type;
	int m_id;
	bool m_primary;
	int m_timestamp;
	int m_x, m_y;
	int m_dx, m_dy;
};

struct InputEvent
{
	enum EType
	{
		EV_INVALID,
		EV_KEYBOARD,
		EV_MOUSE,
		EV_GAMEPAD,
		EV_TOUCH,
	};

	InputEvent()
		: m_type( EV_INVALID )
	{}

	EType m_type;

	  // Aren't unions nice? :)
	union
	{
		KeyboardEvent m_keyboardEvent;
		MouseEvent m_mouseEvent;
		GamepadEvent m_gamepadEvent;
		TouchEvent m_touchEvent;
	};

	static InputEvent KeyboardEvent( KeyboardEvent::EType type, int code, int unichar, int flags )
	{
		InputEvent ev;
		ev.m_type = EV_KEYBOARD;
		ev.m_keyboardEvent.m_type = type;
		ev.m_keyboardEvent.m_keycode = code;
		ev.m_keyboardEvent.m_flags = flags;
		ev.m_keyboardEvent.m_unichar = unichar;
		return ev;
	}

	static InputEvent MouseEvent( MouseEvent::EType type, int x, int y, int dWheel )
	{
		InputEvent ev;
		ev.m_type = EV_MOUSE;
		ev.m_mouseEvent.m_type = type;
		ev.m_mouseEvent.m_x = x;
		ev.m_mouseEvent.m_y = y;
		ev.m_mouseEvent.m_deltaWheel = dWheel;
		return ev;
	}

	static InputEvent GamepadEvent( GamepadEvent::EType type, int stick, int axis, float pos )
	{
		InputEvent ev;
		ev.m_type = EV_GAMEPAD;
		ev.m_gamepadEvent.m_type = type;
		ev.m_gamepadEvent.m_stick = stick;
		ev.m_gamepadEvent.m_axis = axis;
		ev.m_gamepadEvent.m_pos = pos;
		return ev;
	}

	static InputEvent GamepadEvent( GamepadEvent::EType type, int button )
	{
		InputEvent ev;
		ev.m_type = EV_GAMEPAD;
		ev.m_gamepadEvent.m_type = type;
		ev.m_gamepadEvent.m_stick = -1;
		ev.m_gamepadEvent.m_axis = -1;
		ev.m_gamepadEvent.m_pos = 0.0f;
		ev.m_gamepadEvent.m_button = button;
		return ev;
	}

	static InputEvent TouchEvent( TouchEvent::EType type, int id, bool primary, int timestamp, int x, int y, int dx, int dy )
	{
		InputEvent ev;
		ev.m_type = EV_TOUCH;
		ev.m_touchEvent.m_type = type;
		ev.m_touchEvent.m_id = id;
		ev.m_touchEvent.m_primary = primary;
		ev.m_touchEvent.m_timestamp = timestamp;
		ev.m_touchEvent.m_x = x;
		ev.m_touchEvent.m_y = y;
		ev.m_touchEvent.m_dx = dx;
		ev.m_touchEvent.m_dy = dy;
		return ev;
	}
};

// See AllegroInput5 for some comments
class IInput
{
public:
	virtual bool Init( bool keyboard, bool mouse, bool gamepad, bool touch ) = 0;
	virtual void WaitAnyKey() = 0;
	virtual InputEvent GetEvent() = 0;
	virtual void BeginRead() = 0;
	virtual void PopEvent() = 0;
	virtual void NextEvent() = 0;
	virtual void Update( int dt ) = 0;
	virtual void Clear() = 0;

	virtual bool HasHardwareKeyboardInstalled() = 0;
	virtual bool HasGamepadInstalled() = 0;
	virtual bool HasTouchInstalled() = 0;
};

IInput & GetInput();
