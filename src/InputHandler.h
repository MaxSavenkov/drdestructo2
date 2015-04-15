#pragma once
#include "IInput.h"
#include "FastDelegate/FastDelegate.h"
#include "FastDelegate/FastDelegateBind.h"
#include "JoystickMapping.h"

/*
	Class for handling user input. Gets input events,
	calls handler function for registred events.

	It was designed to be far more flexible than it is now,
	but never completed.

	Basically, a map from input event to a handler function.
	Be aware, that BOTH input event and it's handler should be unique!
	I.e., you can't register the same handler for several events
	(it's a stupid design decision, but to change it now would be
	too troublesome).
*/
class InputHandler
{
public:
	typedef fastdelegate::FastDelegate<bool (const InputEvent & )> Handler;

private:

	struct KeyboardKey
	{
		int keycode;
		int flags;

		bool operator < ( const KeyboardKey & other ) const
		{
			if ( keycode != other.keycode )
				return keycode < other.keycode;

			  // Ignore flags for now, they work strangely
			//if ( flags != other.flags )
			//	return flags < other.flags;

			return false;
		}
	};
	
	static const int KEYBOARD_HANDLERS_COUNT = KeyboardEvent::KBD_KEY_REPEAT + 1;
	typedef std::map<KeyboardKey, Handler> HandlersMap;
	HandlersMap m_handlers[ KEYBOARD_HANDLERS_COUNT ];

	typedef std::map<TouchEvent::EType, Handler> TouchHandlersMap;
	TouchHandlersMap m_touchHandlers;

	friend struct GamepadEventRemover;
	struct GamepadKey
	{
		GamepadEvent::EType m_type;
		int m_stick;
		int m_axis;
		int m_sign;
		int m_button;
		int m_baseValue;

		Handler m_handler;
	};
	typedef std::vector<GamepadKey> GamepadHandlersVector;
	GamepadHandlersVector m_gamepadHandlers;

public:
	void ProcessInput( IInput & input );

	void AddKeyboardHandler( const Handler & handler, KeyboardEvent::EType type, int keycode, int flags = 0 );
	template<typename T>
	void AddKeyboardHandler( T *pObject, bool (T::*func)(const InputEvent &), KeyboardEvent::EType type, int keycode, int flags = 0 )
	{
		Handler h;
		h.bind( pObject, func );
		AddKeyboardHandler( h, type, keycode, flags );
	}
	void AddTouchHandler( const Handler & handler, TouchEvent::EType type );
	template<typename T>
	void AddTouchHandler( T *pObject, bool (T::*func)(const InputEvent &), TouchEvent::EType type )
	{
		Handler h;
		h.bind( pObject, func );
		AddTouchHandler( h, type );
	}

	void AddGamepadHandler( const Handler & handler, GamepadEvent::EType, int stick, int axis, int sign, int baseValue, int button );
	template<typename T>
	void AddGamepadAxisHandler( T *pObject, bool (T::*func)(const InputEvent &), int stick, int axis, int baseValue, int sign )
	{
		Handler h;
		h.bind( pObject, func );
		AddGamepadHandler( h, GamepadEvent::GP_AXIS, stick, axis, sign, baseValue, -1 );
	}
	template<typename T>
	void AddGamepadButtonHandler( T *pObject, bool (T::*func)(const InputEvent &), bool up, int button )
	{
		Handler h;
		h.bind( pObject, func );
		AddGamepadHandler( h, up ? GamepadEvent::GP_BUTTON_UP : GamepadEvent::GP_BUTTON_DOWN, -1, -1, 0, 0, button );
	}
	void AddGamepadHandlerFromConfig( const char *name, const Handler & handler );
	template<typename T>
	void AddGamepadHandlerFromConfig( const char *name, T *pObject, bool (T::*func)(const InputEvent &) )
	{
		Handler h;
		h.bind( pObject, func );
		AddGamepadHandlerFromConfig( name, h );
	}
	void AddGamepadHandlerForControl( JoystickMapping::EControl control, const Handler & handler );
	template<typename T>
	void AddGamepadHandlerForControl( JoystickMapping::EControl control, T *pObject, bool (T::*func)(const InputEvent &) )
	{
		Handler h;
		h.bind( pObject, func );
		AddGamepadHandlerForControl( control, h );
	}

	void RemoveKeyboardHandler( KeyboardEvent::EType type, int keycode, int flags = 0 );
	void RemoveKeyboardHandler( KeyboardEvent::EType type, const Handler & handler );
	template<typename T>
	void RemoveKeyboardHandler( KeyboardEvent::EType type, T *pObject, bool (T::*func)(const InputEvent &) )
	{
		Handler h;
		h.bind( pObject, func );
		RemoveKeyboardHandler( type, h );
	}
};
