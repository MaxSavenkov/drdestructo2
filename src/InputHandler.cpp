#include "stdafx.h"
#include "InputHandler.h"
#include "ISystem.h"

void InputHandler::AddKeyboardHandler( const Handler & handler, KeyboardEvent::EType type, int keycode, int flags )
{
	HandlersMap & map = m_handlers[ type ];
	for ( HandlersMap::iterator iter = map.begin(); iter != map.end(); ++iter )
	{
		if ( iter->second == handler )
		{
			map.erase( iter );
			break;
		}
	}

	KeyboardKey key;
	key.keycode = keycode;
	key.flags = flags;
	map.insert( std::make_pair( key, handler ) );
}

void InputHandler::AddTouchHandler( const Handler & handler, TouchEvent::EType type )
{
	TouchHandlersMap & map = m_touchHandlers;
	for ( TouchHandlersMap::iterator iter = map.begin(); iter != map.end(); ++iter )
	{
		if ( iter->second == handler )
		{
			map.erase( iter );
			break;
		}
	}

	map.insert( std::make_pair( type, handler ) );
}

struct GamepadEventRemover
{
	InputHandler::GamepadKey m_newKey;

	bool operator()( const InputHandler::GamepadKey & key )
	{
		if ( m_newKey.m_handler == key.m_handler )
		{
			if ( m_newKey.m_button < 0 || key.m_button < 0 )
				return true;

			if ( m_newKey.m_button != key.m_button )
				return true;

			if ( m_newKey.m_type == key.m_type )
				return true;
		}

		if ( m_newKey.m_type != key.m_type )
			return false;

		if ( m_newKey.m_type == GamepadEvent::GP_AXIS )
			return m_newKey.m_stick == key.m_stick && m_newKey.m_axis == key.m_axis && m_newKey.m_sign == key.m_sign;

		if ( m_newKey.m_type == GamepadEvent::GP_BUTTON_DOWN || m_newKey.m_type == GamepadEvent::GP_BUTTON_UP )
			return m_newKey.m_button == key.m_button;

		return false;
	}
};

void InputHandler::AddGamepadHandler( const Handler & handler, GamepadEvent::EType type, int stick, int axis, int sign, int baseValue, int button )
{
	GamepadKey k;
	k.m_type = type;
	k.m_stick = stick;
	k.m_axis = axis;
	k.m_sign = sign;
	k.m_button = button;
	k.m_baseValue = baseValue;
	k.m_handler = handler;

	GamepadEventRemover pred;
	pred.m_newKey = k;

	GamepadHandlersVector::iterator newEnd = std::remove_if( m_gamepadHandlers.begin(), m_gamepadHandlers.end(), pred );
	if ( newEnd != m_gamepadHandlers.end() )
		m_gamepadHandlers.erase( newEnd, m_gamepadHandlers.end() );

	m_gamepadHandlers.push_back( k );
}

void InputHandler::AddGamepadHandlerFromConfig( const char *name, const Handler & handler )
{
	ISystem & system = GetSystem();

	JoystickMapping::EControl control = (JoystickMapping::EControl)system.GetConfigValue_Int( "DD_Controls", name, JoystickMapping::CONTROL_INVALID );

	const JoystickMapping & mapping = GetJoystickMapping(0);

	int button = -1;
	int stick = -1;
	int axis = -1;
	int sign = 0;
	int baseValue = 0;

	if ( mapping.GetControl( control, button, stick, axis, sign, baseValue ) )
	{
		if ( button >= 0 )
		{
			AddGamepadHandler( handler, GamepadEvent::GP_BUTTON_DOWN, -1, -1, 0, 0, button );
			AddGamepadHandler( handler, GamepadEvent::GP_BUTTON_UP, -1, -1, 0, 0, button );
		}
		else
			AddGamepadHandler( handler, GamepadEvent::GP_AXIS, stick, axis, sign, baseValue, -1 );
	}
}

void InputHandler::AddGamepadHandlerForControl( JoystickMapping::EControl control, const Handler & handler )
{
	ISystem & system = GetSystem();

	const JoystickMapping & mapping = GetJoystickMapping(0);

	int button = -1;
	int stick = -1;
	int axis = -1;
	int sign = 0;
	int baseValue = 0;

	if ( mapping.GetControl( control, button, stick, axis, sign, baseValue ) )
	{
		if ( button >= 0 )
		{
			AddGamepadHandler( handler, GamepadEvent::GP_BUTTON_DOWN, -1, -1, 0, 0, button );
			AddGamepadHandler( handler, GamepadEvent::GP_BUTTON_UP, -1, -1, 0, 0, button );
		}
		else
			AddGamepadHandler( handler, GamepadEvent::GP_AXIS, stick, axis, sign, baseValue, -1 );
	}
}

void InputHandler::RemoveKeyboardHandler( KeyboardEvent::EType type, int keycode, int flags )
{
	HandlersMap & map = m_handlers[ type ];
	for ( HandlersMap::iterator iter = map.begin(); iter != map.end(); ++iter )
	{
		if ( iter->first.keycode == keycode && iter->first.flags == flags )
		{
			map.erase( iter );
			break;
		}
	}
}

void InputHandler::RemoveKeyboardHandler( KeyboardEvent::EType type, const Handler & handler )
{
	HandlersMap & map = m_handlers[ type ];
	for ( HandlersMap::iterator iter = map.begin(); iter != map.end(); ++iter )
	{
		if ( iter->second == handler )
		{
			map.erase( iter );
			break;
		}
	}
}

void InputHandler::ProcessInput( IInput & input )
{
	input.BeginRead();
	while( true )
	{
		InputEvent e = input.GetEvent();
		if ( e.m_type == InputEvent::EV_INVALID )
			return;

		bool handled = false;

		if ( e.m_type == InputEvent::EV_KEYBOARD )
		{
			KeyboardKey key;
			key.keycode = e.m_keyboardEvent.m_keycode;
			key.flags = e.m_keyboardEvent.m_flags;
			
			HandlersMap::iterator handler = m_handlers[ e.m_keyboardEvent.m_type ].find( key );
			if ( handler != m_handlers[ e.m_keyboardEvent.m_type ].end() )
				handled = handler->second( e );
		}
		else if ( e.m_type == InputEvent::EV_TOUCH )
		{
			TouchHandlersMap::iterator handler = m_touchHandlers.find( e.m_touchEvent.m_type );
			if ( handler != m_touchHandlers.end() )
				handled = handler->second( e );			
		}
		else if ( e.m_type == InputEvent::EV_GAMEPAD )
		{
			for ( GamepadHandlersVector::iterator iter = m_gamepadHandlers.begin();
				  iter != m_gamepadHandlers.end();
				  ++iter )
			{
				if ( iter->m_type != e.m_gamepadEvent.m_type )
					continue;

				const int gamepadPos = e.m_gamepadEvent.m_pos - iter->m_baseValue;

				int sign = 0;
				if ( gamepadPos > 0 ) // >= because we need to call at least one axis handler with 0 to cancel previous input.
					sign = 1;
				else if ( gamepadPos < 0 )
					sign = -1;

				if ( iter->m_type == GamepadEvent::GP_AXIS )
				{
					if ( iter->m_stick == e.m_gamepadEvent.m_stick && iter->m_axis == e.m_gamepadEvent.m_axis && ( sign == 0 || iter->m_sign == sign ) )
					{
						handled = iter->m_handler( e );
						break;
					}
				}
				else
				{
					if ( iter->m_button == e.m_gamepadEvent.m_button )
					{
						handled = iter->m_handler( e );
						break;
					}
				}
			}
		}

		if ( handled )
			input.PopEvent();
		else
			input.NextEvent();
	}
}
