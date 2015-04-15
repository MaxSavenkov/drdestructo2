#include "stdafx.h"
#include "AllegroInput5.h"
#include "JoystickMapping.h"
#include "LogCommon.h"

bool AllegroInput5::Init( bool keyboard, bool mouse, bool gamepad, bool touch )
{
	// There is a slight confusion about these flags. For now, they do not GUARANTEE that a given input method is available!
	// They only indicate that al_intall_*() call was successfull, with the exception of m_hadGamepad, that also checks that
	// at least one joystick is available. This is not quite right, but it's mostly relevant for mobile devies, so I'll fix
	// this later :)
	m_hasKeyboard = keyboard;
	m_hasMouse = mouse;
	m_hasGamepad = gamepad;
	m_hasTouch = touch;

	m_queue = al_create_event_queue();

	if ( keyboard )
	{
		if ( !al_install_keyboard() )
		{
			GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroInput: Failed to install keyboard driver" );
			m_hasKeyboard = false;
		}
		else
		{
			GetLog().Log( CommonLog(), LL_INFO, "AllegroInput: Installed keyboard driver" );
			al_register_event_source( m_queue, al_get_keyboard_event_source() );
		}
	}

	if ( mouse )
	{
		if ( !al_install_mouse() )
		{
			GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroInput: Failed to install mouse driver" );
			m_hasMouse = false;
		}
		else
		{
			GetLog().Log( CommonLog(), LL_INFO, "AllegroInput: Installed mouse driver" );		
			al_register_event_source( m_queue, al_get_mouse_event_source() );
		}
	}

	if ( gamepad )
	{
		if ( !al_install_joystick() )
		{
			GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroInput: Failed to install gamepad driver" );
			m_hasGamepad = false;
		}
		else
		{
			 // Note: only the first joystick is supported. This should be fixed. Sometime.
			ALLEGRO_JOYSTICK *pJoystick = al_get_joystick( 0 );
			if ( !pJoystick )
			{
				GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroInput: Joystick #0 not found" );
				m_hasGamepad = false;
			}
			else
			{
				GetLog().Log( CommonLog(), LL_INFO, "AllegroInput: Installed gamepad driver" );
				al_register_event_source( m_queue, al_get_joystick_event_source() );
			}
		}
	}

	if ( touch )
	{
		if ( !al_install_touch_input() )
		{
			GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroInput: Failed to install touch input driver" );
			m_hasTouch = false;
		}
		else
		{
			GetLog().Log( CommonLog(), LL_INFO, "AllegroInput: Installed touch input driver" );
			al_register_event_source( m_queue, al_get_touch_input_event_source() );
		}
	}

	return true;
}

AllegroInput5::~AllegroInput5()
{
	al_destroy_event_queue( m_queue );
}

// Simulates behaviour of Allegro 4.x function readkey() for debugging purposes
void AllegroInput5::WaitAnyKey()
{
	Clear();
	while( true )
	{		
		Update(0);
		BeginRead();
		
		while( true )
		{
			InputEvent ev = GetEvent();
			InputEvent::EType type = ev.m_type;
			if ( type == InputEvent::EV_KEYBOARD && ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_UP )
			{
				Clear();
				return;
			}
			else if ( type == InputEvent::EV_INVALID )
				break;

			PopEvent();
		}
	}
}

// Pushes new event into queue
void AllegroInput5::AddEvent( InputEvent ev )
{
	m_events.push_back( ev );
	if ( m_events.size() > MAX_EVENTS )
		m_events.erase( m_events.begin() );
}

void AllegroInput5::Update( int dt )
{
	if ( m_doubleClickTimer > 0 )
		m_doubleClickTimer -= dt;
	if ( m_repeatTimer > 0 )
		m_repeatTimer -= dt;

	  // Get current transformation, to tranform reported scaled/shifted mouse coordinates into something the game can use
	const ALLEGRO_TRANSFORM *pTransform = al_get_current_inverse_transform();

	const JoystickMapping & mapping = GetJoystickMapping(0);

	ALLEGRO_EVENT ev;
	while( al_get_next_event( m_queue, &ev ) )
	{
		if ( m_hasKeyboard )
		{
			if ( ev.any.type == ALLEGRO_EVENT_KEY_DOWN )
				AddEvent( InputEvent::KeyboardEvent( KeyboardEvent::KBD_KEY_DOWN, ev.keyboard.keycode, ev.keyboard.unichar, ev.keyboard.modifiers ) );
			else if ( ev.any.type == ALLEGRO_EVENT_KEY_UP )
				AddEvent( InputEvent::KeyboardEvent( KeyboardEvent::KBD_KEY_UP, ev.keyboard.keycode, ev.keyboard.unichar, ev.keyboard.modifiers ) );
			else if ( ev.any.type == ALLEGRO_EVENT_KEY_CHAR && m_repeatTimer <= 0 )
			{
				  // Please note that some event params (like modifiers) are ONLY accessible in KEY_CHAR event!
				AddEvent( InputEvent::KeyboardEvent( KeyboardEvent::KBD_KEY_REPEAT, ev.keyboard.keycode, ev.keyboard.unichar, ev.keyboard.modifiers ) );
				m_repeatTimer = 100;
			}
		}

		if ( m_hasMouse )
		{
			float exf = ev.mouse.x;
			float eyf = ev.mouse.y;
			float edxf = ev.mouse.dx;
			float edyf = ev.mouse.dy;

			al_transform_coordinates( pTransform, &exf, &eyf );
			al_transform_coordinates( pTransform, &edxf, &edyf );

			int ex = (int)floorf( exf );
			int ey = (int)floorf( eyf );
			int edx = (int)floorf( edxf );
			int edy = (int)floorf( edyf );

			if ( ev.any.type == ALLEGRO_EVENT_MOUSE_AXES )
			{
				if ( edx != 0 || edy != 0 )
					AddEvent( InputEvent::MouseEvent( MouseEvent::MOUSE_MOVE, ex, ey, ev.mouse.dw ) );
				if ( ev.mouse.dw != 0 )
					AddEvent( InputEvent::MouseEvent( MouseEvent::MOUSE_WHEEL, ex, ey, ev.mouse.dw ) );
			}
			else if ( ev.any.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN )
			{
				MouseEvent::EType type = MouseEvent::MOUSE_B1_DOWN;
				if ( ev.mouse.button == 1 )
				{
					if ( m_doubleClickTimer > 0 )
					{
						m_doubleClickTimer = 0;
						type = MouseEvent::MOUSE_B1_DOUBLECLICK;
					}
					else
						type = MouseEvent::MOUSE_B1_DOWN;
				}
				else if ( ev.mouse.button == 2 )
					type = MouseEvent::MOUSE_B2_DOWN;
				else if ( ev.mouse.button == 3 )
					type = MouseEvent::MOUSE_B3_DOWN;
				AddEvent( InputEvent::MouseEvent( type, ex, ey, 0 ) );
			}
			else if ( ev.any.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP )
			{
				MouseEvent::EType type = MouseEvent::MOUSE_B1_UP;
				if ( ev.mouse.button == 1 )
				{
					m_doubleClickTimer = 250;
					type = MouseEvent::MOUSE_B1_UP;
				}
				else if ( ev.mouse.button == 2 )
					type = MouseEvent::MOUSE_B2_UP;
				else if ( ev.mouse.button == 3 )
					type = MouseEvent::MOUSE_B3_UP;
				AddEvent( InputEvent::MouseEvent( type, ex, ey, 0 ) );
			}
		}

		if ( m_hasGamepad )
		{
			if ( ev.any.type == ALLEGRO_EVENT_JOYSTICK_AXIS )
			{
				const int stick = ev.joystick.stick;
				const int axis  = ev.joystick.axis;
			
				if ( stick >= (int)m_gamepadSticks.size() )
					m_gamepadSticks.resize( stick + 1 );

				GamepadStick & s = m_gamepadSticks[ stick ];
				if ( axis >= (int)s.m_axis.size() )
					s.m_axis.resize( axis + 1 );

				GamepadAxis & a = s.m_axis[ axis ];

				  // Some axes on some systems has their neutral position at -1 instead of 0, so we have to adjust for it
				const float adjPos = ev.joystick.pos - mapping.GetAxisBase( ev.joystick.stick, ev.joystick.axis );
				const bool nearZero = fabs(adjPos) < 0.01f;
				
				  // We only want to send GP_AXIS event once for each direction change, like it was a digital axis or a keyboard key
				if ( nearZero )
				{
					  // If the stick is back to its neutral position, and wasn't there before, send an event about it
					if ( !a.m_unlocked )
						AddEvent( InputEvent::GamepadEvent( GamepadEvent::GP_AXIS, ev.joystick.stick, ev.joystick.axis, 0 ) );
					a.m_unlocked = true;
				}
				else
				{
					  // If the stick is outside its neutral position, and this wasn't reported before, send an event about it
					if ( a.m_unlocked )
						AddEvent( InputEvent::GamepadEvent( GamepadEvent::GP_AXIS, ev.joystick.stick, ev.joystick.axis, adjPos ) );
					a.m_unlocked = false;
				}
			}
			else if ( ev.any.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN )
			{
				AddEvent( InputEvent::GamepadEvent( GamepadEvent::GP_BUTTON_DOWN, ev.joystick.button ) );
			}
			else if ( ev.any.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP )
			{
				AddEvent( InputEvent::GamepadEvent( GamepadEvent::GP_BUTTON_UP, ev.joystick.button ) );
			}
		}

		if ( m_hasTouch )
		{
			float exf = ev.touch.x;
			float eyf = ev.touch.y;
			float edxf = ev.touch.dx;
			float edyf = ev.touch.dy;

			al_transform_coordinates( pTransform, &exf, &eyf );
            edxf *= pTransform->m[0][0];
            edyf *= pTransform->m[0][0];

			int ex = (int)floorf( exf );
			int ey = (int)floorf( eyf );
			int edx = (int)floorf( edxf );
			int edy = (int)floorf( edyf );

			if ( ev.any.type == ALLEGRO_EVENT_TOUCH_BEGIN )
				AddEvent( InputEvent::TouchEvent( TouchEvent::TOUCH_BEGIN, ev.touch.id, ev.touch.primary, ev.touch.timestamp, ex, ey, edx, edy ) );
			else if ( ev.any.type == ALLEGRO_EVENT_TOUCH_MOVE )
				AddEvent( InputEvent::TouchEvent( TouchEvent::TOUCH_MOVE, ev.touch.id, ev.touch.primary, ev.touch.timestamp, ex, ey, edx, edy ) );
			else if ( ev.any.type == ALLEGRO_EVENT_TOUCH_END )
				AddEvent( InputEvent::TouchEvent( TouchEvent::TOUCH_END, ev.touch.id, ev.touch.primary, ev.touch.timestamp, ex, ey, edx, edy ) );
			else if ( ev.any.type == ALLEGRO_EVENT_TOUCH_CANCEL )
				AddEvent( InputEvent::TouchEvent( TouchEvent::TOUCH_CANCEL, ev.touch.id, ev.touch.primary, ev.touch.timestamp, ex, ey, edx, edy ) );

			//GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroInput: event=%i id=%i x=%f y=%f", ev.any.type, ev.touch.id, ev.touch.x, ev.touch.y );
		}
	}

	if ( m_hasKeyboard )
	{
		  // Since Allegro events don't have KEY_PRESSED, we do it other way.
		ALLEGRO_KEYBOARD_STATE state;
		al_get_keyboard_state( &state );

		for ( int i = 0; i < ALLEGRO_KEY_MAX; ++i )
		{
			  // We don't know key code for state :(
			if ( al_key_down( &state, i ) && m_keyStates[ i ] )
				AddEvent( InputEvent::KeyboardEvent( KeyboardEvent::KBD_KEY_PRESSED, i, 0, 0 ) );

			m_keyStates[ i ] = al_key_down( &state, i )  != 0;
		}
	}
}

InputEvent AllegroInput5::GetEvent()
{
	if ( m_eventIndex >= m_events.size() )
		return InputEvent();

	InputEvent ev = m_events[ m_eventIndex ];
	return ev;
}

void AllegroInput5::NextEvent()
{
	++m_eventIndex;
}

void AllegroInput5::PopEvent()
{
	if ( m_eventIndex < m_events.size() )
		m_events.erase( m_events.begin() + m_eventIndex );
}

bool AllegroInput5::HasHardwareKeyboardInstalled()
{
#if defined( ALLEGRO_IPHONE)
	return false;
#elif defined( ALLEGRO_ANDROID )
	// TODO: Check for hardware keyboard presense via JNI
	//http://stackoverflow.com/questions/2415558/how-to-detect-hardware-keyboard-presence
	return m_hasKeyboard;
#else
	return m_hasKeyboard;
#endif
}

bool AllegroInput5::HasGamepadInstalled()
{
	return m_hasGamepad;
}

bool AllegroInput5::HasTouchInstalled()
{
	return m_hasTouch;
}
