#include "stdafx.h"
#include "MenuHelpers.h"
#include "JoystickMapping.h"
#include "IInput.h"

void GameMenu::AddEntry( MenuEntry *pEntry )
{
      // Find a place for the new entry by its GetOrder()
	int insertPos = 0;
	for ( insertPos = 0; insertPos < (int)m_entries.size(); ++insertPos )
	{
		const MenuEntry *pPrevEntry = m_entries[insertPos];
		if ( pPrevEntry->GetOrder() >= pEntry->GetOrder() )
			break;
	}

	m_entries.insert( m_entries.begin() + insertPos, pEntry );

	  // Position the new entry according to its placement type
	int verticalY = 100;
	int footerX = 100;

	const int verticalX = 30;
	const int footerY = m_height - 37;

	int lastVertical = -1;
	int firstFooter = -1;

	for ( int i = 0; i < (int)m_entries.size(); ++i )
	{
		MenuEntry *e = m_entries[i];
		int x = 0, y = 0;

		const int placement = e->GetPlacementType();
		switch( placement )
		{
			case PLACEMENT_VERTICAL:
				x = verticalX;
				y = verticalY;
				verticalY += 64;
				lastVertical = i;
				break;

			case PLACEMENT_FOOTER:
				x = footerX;
				footerX += 250;
				y = footerY;
				if ( firstFooter < 0 ) firstFooter = i;
				break;

		}
		e->SetX( x );
		e->SetY( y );

		  // Set entry's neighbors (entries to which you can go from this ony using left/right/up/down buttons)
		if ( placement == PLACEMENT_VERTICAL )
		{
			e->SetNextUp( i == 0 ? -1 : i - 1 );
			e->SetNextDown( i == m_entries.size() - 1 ? -1 : i + 1 );
		}
		else if ( placement == PLACEMENT_FOOTER )
		{
			e->SetNextUp( lastVertical );
			e->SetNextLeft( i == firstFooter ? -1 : i - 1 );
			e->SetNextRight( i == m_entries.size() - 1 ? -1 : i + 1 );
		}
	}

	  // From the last vertical entry, you can go to the first footer entry by pressing Down
	if ( lastVertical >= 0 )
		m_entries[ lastVertical ]->SetNextDown( firstFooter );
}

void GameMenu::Render( IRender *pRender )
{
	  // Yes, images and sizes are hard-coded. It was faster that way
	if ( !m_imgInit )
	{
		m_imgInit = true;

		m_atlas = pRender->LoadImage( "Data/Sprites2/ui/atlas_menu_frame.png" );

		m_frame.m_topL.Init( m_atlas, 5, 0, 43, 103 );
		m_frame.m_topM.Init( m_atlas, 48, 0, 66, 44 );
		m_frame.m_topR.Init( m_atlas, 114, 0, 48, 103 );

		m_frame.m_botL.Init( m_atlas, 0, 137, 48, 113 );
		m_frame.m_botM.Init( m_atlas, 48, 236, 66, 14 );
		m_frame.m_botR.Init( m_atlas, 114, 137, 48, 113 );

		m_frame.m_left.Init( m_atlas, 0, 103, 43, 35 );
		m_frame.m_right.Init( m_atlas, 119, 103, 43, 35 );

		m_border.Init( *pRender );
	}

	m_border.Render( *pRender );
	m_frame.Render( pRender, m_x, m_y, m_width, m_height );

	Color off( 73, 198, 135 );
	pRender->DrawText( m_x + 60, m_y + 15, off, 25, "%s", m_caption.c_str() );

	BaseMenu::Render( pRender );
}

void ButtonMenuEntry::Render( IRender *pRender, int x, int y )
{
	if ( !m_imageOff.IsValid() )
	{
		m_imageOn = pRender->LoadImage( m_fileOn.c_str() );
		m_imageOff = pRender->LoadImage( m_fileOff.c_str() );
		m_imagePressed = pRender->LoadImage( m_filePressed.c_str() );
	}

	if ( m_pressed )
		pRender->DrawImage( m_imagePressed, x + m_x, y + m_y );
	else if ( !m_selected )
		pRender->DrawImage( m_imageOff, x + m_x, y + m_y );
	else
		pRender->DrawImage( m_imageOn, x + m_x, y + m_y );

	Color off( 73, 198, 135 );
	Color on( 136, 247, 27 );
	pRender->DrawAlignedText( x + m_x + m_w/2 + m_textOffsetX, y + m_y + m_textOffsetY, m_selected ? on : off, 25, TEXT_ALIGN_CENTER, "%s", m_label.c_str() );
	//pRender->DrawRect( x + GetX1(), y + GetY1(), x + GetX2(), y + GetY2(), Color(255,255,255) );
}

void ButtonMenuEntry::Update( float dt )
{
	if ( m_pressedTimer > 0 )
	{
		m_pressedTimer -= dt;

		if ( m_pressedTimer < 0 )
		{
			m_pressed = false;
		}
	}
}

void ButtonMenuEntry::ProcessInput( IInput *pInput )
{
	pInput->BeginRead();
	while( true )
	{
		InputEvent ev = pInput->GetEvent();
		if ( ev.m_type == InputEvent::EV_INVALID )
			break;

		if ( ev.m_type == InputEvent::EV_KEYBOARD )
		{
			if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ENTER )
			{
				if ( ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_UP )
				{
					m_pressed = false;
					if ( m_pCallback )
						(*m_pCallback)( m_callbackID );
				}
				else if ( ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_DOWN )
				{
					m_pressed = true;
				}

				pInput->PopEvent();
				continue;
			}			
		}
		
		if ( ev.m_type == InputEvent::EV_GAMEPAD )
		{
			const JoystickMapping & mapping = GetJoystickMapping(0);
			if ( mapping.FindButton(ev.m_gamepadEvent.m_button) == JoystickMapping::CONTROL_A )
			{
				if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_UP )
				{
					m_pressed = false;
					if ( m_pCallback )
						(*m_pCallback)( m_callbackID );
				}
				else if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_DOWN ) 
				{
					m_pressed = true;
				}
				pInput->PopEvent();
				continue;
			}
		}

		pInput->NextEvent();
	}
}

void ButtonMenuEntry::Activate()
{
	if ( m_pCallback )
		(*m_pCallback)( m_callbackID );
}


void TextMenuEntry::Render( IRender *pRender, int x, int y )
{
	Color off( 73, 198, 135 );
	Color on( 136, 247, 27 );
	pRender->DrawAlignedText( x + m_x, y + m_y + 10, m_selected ? on : off, 25, TEXT_ALIGN_CENTER, "%s", m_label.c_str() );
	//pRender->DrawRect( x + GetX1(), y + GetY1(), x + GetX2(), y + GetY2(), Color(255,255,255) );
}

void SimpleMenuEntry::ProcessInput( IInput *pInput )
{
	pInput->BeginRead();
	while( true )
	{
		InputEvent ev = pInput->GetEvent();
		if ( ev.m_type == InputEvent::EV_INVALID )
			break;

		if ( ev.m_type == InputEvent::EV_KEYBOARD && ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_UP )
		{
			if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ENTER )
			{
				if ( m_pCallback )
					(*m_pCallback)( m_callbackID );

				pInput->PopEvent();
				continue;
			}			
		}
		
		if ( ev.m_type == InputEvent::EV_GAMEPAD && ev.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_UP )
		{
			const JoystickMapping & mapping = GetJoystickMapping(0);
			if ( mapping.FindButton(ev.m_gamepadEvent.m_button) == JoystickMapping::CONTROL_A )
			{
				if ( m_pCallback )
					(*m_pCallback)( m_callbackID );

				pInput->PopEvent();
				continue;
			}
		}

		pInput->NextEvent();
	}
}

void SimpleMenuEntry::Activate()
{
	if ( m_pCallback )
		(*m_pCallback)( m_callbackID );
}

int SimpleMenuEntry::GetWidth()
{
	return GetRender().GetStringWidth( m_label.c_str(), 25 );
}

void ValueMenuEntry::Render( IRender *pRender, int x, int y )
{
	if ( !m_leftOn.IsValid() )
	{
		m_leftOff = pRender->LoadImage( "Data/Sprites2/ui/menu_btn_normal.png" );
		m_leftOn = pRender->LoadImage( "Data/Sprites2/ui/menu_btn_selected.png" );
		m_rightOff = pRender->LoadImage( "Data/Sprites2/ui/menu_btn2_normal.png" );
		m_rightOn = pRender->LoadImage( "Data/Sprites2/ui/menu_btn2_selected.png" );
		m_arrowLeft = pRender->LoadImage( "Data/Sprites2/ui/arrow_left_selected.png" );
		m_arrowRight = pRender->LoadImage( "Data/Sprites2/ui/arrow_right_selected.png" );
		m_arrowLeftPressed = pRender->LoadImage( "Data/Sprites2/ui/arrow_left_normal.png" );
		m_arrowRightPressed = pRender->LoadImage( "Data/Sprites2/ui/arrow_right_normal.png" );
	}

	pRender->DrawImage( m_leftOff, x + m_x, y + m_y );

	if ( !m_selected )
		pRender->DrawImage( m_rightOff, x + m_x + m_w - 367 - 68, y + m_y );
	else
	{
		m_screenX = x + m_x + m_w - 367 - 68;

		pRender->DrawImage( m_rightOn, x + m_x + m_w - 367 - 68, y + m_y );
		if ( m_needArrows )
		{
			if ( m_pressedLeft )
				pRender->DrawImage( m_arrowLeftPressed, x + m_x + m_w - 367 - 68 + 20, y + m_y + 10 );
			else
				pRender->DrawImage( m_arrowLeft, x + m_x + m_w - 367 - 68 + 20, y + m_y + 10 );

			if ( m_pressedRight )
				pRender->DrawImage( m_arrowRightPressed, x + m_x + m_w - 68 - 125, y + m_y + 10 );				
			else
				pRender->DrawImage( m_arrowRight, x + m_x + m_w - 68 - 125, y + m_y + 10 );
		}
	}

	Color off( 73, 198, 135 );
	Color on( 136, 247, 27 );
	
	pRender->DrawAlignedText( x + m_x + 308/2 + 40, y + m_y + 10, off, 25, TEXT_ALIGN_CENTER, "%s", m_label.c_str() );
	const char *value = CurrentValue();
	pRender->DrawAlignedText( x + m_x + m_w - 367/2 - 101, y + m_y + 13, m_selected ? on : off, 25, TEXT_ALIGN_CENTER, "%s", value );
}

int ValueMenuEntry::GetWidth()
{
	static char tmp[256];
	const char *value = CurrentValue();
	_snprintf( tmp, 255, "%s : %s",  m_label.c_str(), value ? value : "(NULL)" );
	return GetRender().GetStringWidth( tmp, 25 );
}

void ValueListMenuEntryBase::ProcessInput( IInput *pInput )
{
	/*
		For the most part, input processing of ValueListMenuEntry is simple: switch to the next or previous value when Right or Left is pressed.
		However, please note mouse/touch behaviour: we then use entry's center coordinate to devide it into left and right part, and use that
		to know, whether we should show next or previous item.
	*/
	pInput->BeginRead();
	while( true )
	{
		InputEvent ev = pInput->GetEvent();
		if ( ev.m_type == InputEvent::EV_INVALID )
			break;

		if ( ev.m_type == InputEvent::EV_KEYBOARD && ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_DOWN )
		{
			if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_LEFT )
				m_pressedLeft = true;
			else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_RIGHT )
				m_pressedRight = true;
		}

		if ( ev.m_type == InputEvent::EV_KEYBOARD && ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_UP )
		{
			m_pressedLeft = false;
			m_pressedRight = false;
		}

		if ( ev.m_type == InputEvent::EV_KEYBOARD && ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_REPEAT )
		{
			if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_RIGHT )
			{
				NextValue();
				pInput->PopEvent();
				continue;
			}

			if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_LEFT )
			{
				PrevValue();
				pInput->PopEvent();
				continue;
			}
		}

		if ( ev.m_type == InputEvent::EV_GAMEPAD )
		{
			const JoystickMapping & mapping = GetJoystickMapping(0);				
			JoystickMapping::EControl control = JoystickMapping::CONTROL_INVALID;
			if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_AXIS && fabsf(ev.m_gamepadEvent.m_pos) > 0.00001f )
			{
				const int sign = ev.m_gamepadEvent.m_pos < 0 ? -1 : 1;
				control = mapping.FindStick( ev.m_gamepadEvent.m_stick, ev.m_gamepadEvent.m_axis, sign );
			}
			else if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_DOWN )
				control = mapping.FindButton( ev.m_gamepadEvent.m_button );

			if ( control == JoystickMapping::CONTROL_DPAD_RIGHT || control == JoystickMapping::CONTROL_LEFTSTICK_RIGHT )
			{
				NextValue();
				pInput->PopEvent();
				continue;
			}

			if ( control == JoystickMapping::CONTROL_DPAD_LEFT || control == JoystickMapping::CONTROL_LEFTSTICK_LEFT )
			{
				PrevValue();
				pInput->PopEvent();
				continue;
			}
		}


		if ( ev.m_type == InputEvent::EV_MOUSE )
		{
			if ( ev.m_mouseEvent.m_type == MouseEvent::MOUSE_B1_UP )
			{
				const int mx = ev.m_mouseEvent.m_x;
				const int my = ev.m_mouseEvent.m_y;

				const int cx = m_screenX + GetWidth() / 2;
				if ( mx < cx )
					PrevValue();
				else
					NextValue();				

				pInput->PopEvent();
				continue;
			}
		}
		
		pInput->NextEvent();
	}
}

void TextInputMenuEntry::Render( IRender *pRender, int x, int y )
{
	pRender->DrawText( x, y, m_selected ? pRender->MakeColor( 255, 255, 0 ) : m_color, 25, "%s : %s ", m_label.c_str(), m_value.c_str() );
}

void TextInputMenuEntry::ProcessInput( IInput *pInput )
{
	bool endEdit = false;

	pInput->BeginRead();
	while( true )
	{
		InputEvent ev = pInput->GetEvent();
		if ( ev.m_type == InputEvent::EV_INVALID )
			break;

		if ( ev.m_type == InputEvent::EV_KEYBOARD )
		{
			if ( ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_REPEAT )
			{
				if ( !m_editing )
				{
					if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ENTER )
					{
						m_editing = true;
						pInput->PopEvent();
						continue;
					}
				}
				else
				{
					if ( isprint( ev.m_keyboardEvent.m_unichar ) && ( m_maxLength < 0 || (int)m_value.length() < m_maxLength ) )
					{
						char c = ev.m_keyboardEvent.m_unichar;
						m_value.append( &c, 1 );
					}
					else
					{
						if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_BACKSPACE )
						{
							if ( !m_value.empty() )
								m_value.erase( m_value.begin() + m_value.length() - 1 );
						}
						else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ESCAPE && m_canCancel )
						{
							m_value = m_oldValue;
							endEdit = true;
							if ( m_pCallback )
								m_pCallback->CancelEdit();
						}
					}
				}
			}
			else if ( ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_UP )
			{
				if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ENTER )
				{
					endEdit = true;
					if ( m_pCallback )
						m_pCallback->AcceptEdit( m_value );
				}
			}
		}

		if ( m_editing )
			pInput->PopEvent();
		else
			pInput->NextEvent();
	}

	if ( endEdit )
	{
		m_editing = false;
		pInput->Clear();
	}
}
