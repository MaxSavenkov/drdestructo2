#include "stdafx.h"
#include "BaseMenu.h"
#include "JoystickMapping.h"
#include "IRender.h"
#include "IInput.h"

void BaseMenu::AddEntry( MenuEntry *entry )
{
	  // Entries are sorted by their m_order param
	EntryVector::iterator iter = m_entries.begin();
	while( iter != m_entries.end() && (*iter)->GetOrder() < entry->GetOrder() )
		++iter;

	m_entries.insert( iter, entry );
}

void BaseMenu::Render( IRender *pRender )
{
	for ( EntryVector::iterator iter = m_entries.begin();
		  iter != m_entries.end();
		  ++iter)
		(*iter)->Render( pRender, m_x, m_y );
}

void BaseMenu::ProcessInput( IInput *pInput )
{
	  // If we have a selected menu entry, it may want to process some input before we do it (like textfield entry)
	if ( m_selected >= 0 && m_selected < (int)m_entries.size() )
		m_entries[ m_selected ]->ProcessInput( pInput );

	/*
		Input handling is quite simple here.

		For keyboard and gamepad, Up moves cursor up a position and wraps it to the bottom if we were at the top-most position
		Down move cursor down.

		For mouse and touch, we check which menu item we hit

		Escape calls Back(), which will probably close this menu (but this depends on implementation)
	*/

	pInput->BeginRead();
	while( true )
	{
		InputEvent ev = pInput->GetEvent();
		if ( ev.m_type == InputEvent::EV_INVALID )
			break;

		bool handled = false;

		if ( ev.m_type == InputEvent::EV_KEYBOARD && ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_REPEAT )
		{
			if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_UP )
			{
				CursorUp();
				handled = true;
			}
			else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_DOWN )
			{
				CursorDown();
				handled = true;
			}
			else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_LEFT )
			{
				CursorLeft();
				handled = true;
			}
			else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_RIGHT )
			{
				CursorRight();
				handled = true;
			}
			else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ESCAPE )
			{
				Back();
				handled = true;
			}
		}


		if ( ev.m_type == InputEvent::EV_GAMEPAD )
		{
			const JoystickMapping & mapping = GetJoystickMapping(0);

			if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_AXIS && fabsf(ev.m_gamepadEvent.m_pos) > 0.00001f )
			{
				handled = true;

				const int stick = ev.m_gamepadEvent.m_stick;
				const int axis = ev.m_gamepadEvent.m_axis;
				const int sign = ev.m_gamepadEvent.m_pos > 0 ? 1 : -1;

				JoystickMapping::EControl joyControl = mapping.FindStick( stick, axis, sign );
				if ( joyControl  == JoystickMapping::CONTROL_DPAD_UP || joyControl  == JoystickMapping::CONTROL_LEFTSTICK_UP )
					CursorUp();
				else if ( joyControl == JoystickMapping::CONTROL_DPAD_DOWN || joyControl  == JoystickMapping::CONTROL_LEFTSTICK_DOWN )
					CursorDown();
				else if ( joyControl == JoystickMapping::CONTROL_DPAD_LEFT || joyControl  == JoystickMapping::CONTROL_LEFTSTICK_LEFT )
					CursorLeft();
				else if ( joyControl == JoystickMapping::CONTROL_DPAD_RIGHT || joyControl  == JoystickMapping::CONTROL_LEFTSTICK_RIGHT )
					CursorRight();
				else
					handled = false;
			}
			else if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_DOWN )
			{
				handled = true;
				JoystickMapping::EControl joyControl = mapping.FindButton( ev.m_gamepadEvent.m_button );				
				if ( joyControl  == JoystickMapping::CONTROL_DPAD_UP || joyControl  == JoystickMapping::CONTROL_LEFTSTICK_UP )
					CursorUp();
				else if ( joyControl == JoystickMapping::CONTROL_DPAD_DOWN || joyControl  == JoystickMapping::CONTROL_LEFTSTICK_DOWN )
					CursorDown();
				else if ( joyControl == JoystickMapping::CONTROL_DPAD_LEFT || joyControl  == JoystickMapping::CONTROL_LEFTSTICK_LEFT )
					CursorLeft();
				else if ( joyControl == JoystickMapping::CONTROL_DPAD_RIGHT || joyControl  == JoystickMapping::CONTROL_LEFTSTICK_RIGHT )
					CursorRight();
				else if ( joyControl == JoystickMapping::CONTROL_B )
					Back();
				else
					handled = false;				
			}
		}

		if ( ev.m_type == InputEvent::EV_MOUSE )
		{
			if ( ev.m_mouseEvent.m_type == MouseEvent::MOUSE_MOVE )
			{
				const int mx = ev.m_mouseEvent.m_x;
				const int my = ev.m_mouseEvent.m_y;

				SelectAt( mx, my );
				handled = true;
			}
			else if ( ev.m_mouseEvent.m_type == MouseEvent::MOUSE_B1_UP )
			{
				const int mx = ev.m_mouseEvent.m_x;
				const int my = ev.m_mouseEvent.m_y;

				ActivateAt( mx, my );
				handled = true;
			}
		}

		if ( ev.m_type == InputEvent::EV_TOUCH )
		{
			if ( ev.m_touchEvent.m_type == TouchEvent::TOUCH_MOVE )
			{
				const int mx = ev.m_touchEvent.m_x;
				const int my = ev.m_touchEvent.m_y;

				SelectAt( mx, my );
				handled = true;
			}
			else if ( ev.m_touchEvent.m_type == TouchEvent::TOUCH_BEGIN )
			{
				const int mx = ev.m_touchEvent.m_x;
				const int my = ev.m_touchEvent.m_y;

				ActivateAt( mx, my );
				handled = true;
			}
		}

		if ( handled )
			pInput->PopEvent();
		else
			pInput->NextEvent();
	}
}

bool BaseMenu::CursorUp()
{
	if ( !m_entries.empty() )
	{
		if ( m_selected >= 0 && m_selected < (int)m_entries.size() )
		{
			const int newSelected = m_entries[ m_selected ]->GetNextUp();
			if ( newSelected < 0 )
				return false;

			m_entries[ m_selected ]->SetSelected( false );
			
			m_selected = newSelected;

			m_entries[ m_selected ]->SetSelected( true );
		}
	}

	return true;
}

bool BaseMenu::CursorDown()
{
	if ( !m_entries.empty() )
	{
		if ( m_selected >= 0 && m_selected < (int)m_entries.size() )
		{
			const int newSelected = m_entries[ m_selected ]->GetNextDown();
			if ( newSelected < 0 )
				return false;

			m_entries[ m_selected ]->SetSelected( false );
			
			m_selected = newSelected;

			m_entries[ m_selected ]->SetSelected( true );
		}
	}

	return true;
}

bool BaseMenu::CursorLeft()
{
	if ( !m_entries.empty() )
	{
		if ( m_selected >= 0 && m_selected < (int)m_entries.size() )
		{
			const int newSelected = m_entries[ m_selected ]->GetNextLeft();
			if ( newSelected < 0 )
				return false;

			m_entries[ m_selected ]->SetSelected( false );
			
			m_selected = newSelected;

			m_entries[ m_selected ]->SetSelected( true );
		}
	}

	return true;
}

bool BaseMenu::CursorRight()
{
	if ( !m_entries.empty() )
	{
		if ( m_selected >= 0 && m_selected < (int)m_entries.size() )
		{
			const int newSelected = m_entries[ m_selected ]->GetNextRight();
			if ( newSelected < 0 )
				return false;

			m_entries[ m_selected ]->SetSelected( false );
			
			m_selected = newSelected;

			m_entries[ m_selected ]->SetSelected( true );
		}
	}

	return true;
}

void BaseMenu::SelectAt( int x, int y )
{
	for ( EntryVector::const_iterator iter = m_entries.begin();
			iter != m_entries.end();
			++iter)
	{
		const MenuEntry *pEntry = *iter;
		const int index = ( iter - m_entries.begin() );
		const int x1 = m_x + pEntry->GetX1();
		const int x2 = m_x + pEntry->GetX2();
		const int y1 = m_y + pEntry->GetY1();
		const int y2 = m_y + pEntry->GetY2();

		//GetRender().DrawRect( x, y1, x + m_width, y2, Color(255,255,255) );
						
		if ( y >= y1 && y < y2 && x >= x1 && x < x2 )
		{
			if ( m_selected >= 0 && m_selected < (int)m_entries.size() )
				m_entries[ m_selected ]->SetSelected( false );

			m_selected = index;

			m_entries[ m_selected ]->SetSelected( true );
			break;
		}
	}
}

void BaseMenu::ActivateAt( int x, int y )
{
	if ( m_selected >= 0 && m_selected < (int)m_entries.size() )
	{
		MenuEntry *pEntry = m_entries[ m_selected ];
		const int x1 = m_x + pEntry->GetX1();
		const int x2 = m_x + pEntry->GetX2();
		const int y1 = m_y + pEntry->GetY1();
		const int y2 = m_y + pEntry->GetY2();

		if ( x >= x1 && x <= x2 && y >= y1 && y <= y2 )
			m_entries[ m_selected ]->Activate();
	}
}

void BaseMenu::Update( float dt )
{
	for ( EntryVector::iterator iter = m_entries.begin();
		  iter != m_entries.end();
		  ++iter)
	{
		(*iter)->Update( dt );
	}
}

void BaseMenu::SetSelected( int index )
{
	if ( index < 0 || index >= (int)m_entries.size() )
		return;

	m_selected = index;

	for ( size_t i = 0; i < m_entries.size(); ++i )
	{
		m_entries[ i ]->SetSelected( false );
	}

	m_entries[ index ]->SetSelected( true );
}

void BaseMenu::SetSelected( const MenuEntry *entry )
{
	for ( size_t i = 0; i < m_entries.size(); ++i )
	{
		if ( m_entries[ i ] == entry )
		{
			SetSelected( i );
			return;
		}
	}
}
