#include "stdafx.h"
#include "AchMenu.h"
#include "GameContext.h"
#include "PlayerProfile.h"
#include "Achievements.h"
#include "JoystickMapping.h"
#include "IInput.h"


AchMenu::AchMenu( IAchMenuCallback *pCallback )
	: BaseMenu( "Achievements", SCREEN_W / 2, SCREEN_H - 100 )
	, m_pCallback( pCallback )
	//, m_exportBig			( this, 0, "Export big image (1024x768)", Color(255,255,255), EID_EXPORT_BIG, this )
	//, m_exportSmall			( this, 1, "Export small image (300x300)", Color(255,255,255), EID_EXPORT_SMALL, this )
	//, m_exportHorizontal	( this, 2, "Export horizontal image (2000x50)", Color(255,255,255), EID_EXPORT_HOR, this )
	, m_back				( this, 3, "Back", Color(255,255,255), EID_BACK, this )
	, m_init( false )
	, m_inAchievements( false )
{
	SetSelected( &m_back );
	m_back.SetX( 0 );
	m_back.SetY( 0 );
}

void AchMenu::Init( IGameContext & context )
{
	  // Populate list of achievements
	if ( !m_init )
	{
		m_init = true;

		m_achievements.clear();

		PlayerProfile & prof = context.GetPlayerProfile();

		IRender & render = GetRender();
		const Achievements & achs = GetAchievements();

		for ( int i = 0; i < achs.GetAchievementCount(); ++i )
		{
			const SAchievement* pAch = achs.GetAchievement( i );
			if ( !pAch )
				continue;

			SAchEntry entry;
			entry.m_icon = render.LoadImage( (std::string( "Data/Sprites2/" ) + pAch->m_icon).c_str() );
			entry.m_name = pAch->m_name;
			entry.m_desc = pAch->m_desc;
			entry.m_has = prof.m_achievements.find( pAch->m_guid ) != prof.m_achievements.end();
			m_achievements.push_back( entry );
		}
	}

	SetSelected(0);
	m_inAchievements = false;
	m_selectedAchievement = -1;
}

void AchMenu::Render( IRender *pRender )
{
	BaseMenu::Render( pRender );

	pRender->DrawAlignedText( SCREEN_W / 2, 20, Color(255,255,255), 40, TEXT_ALIGN_CENTER, "Achievements: " );		

	for ( int i = 0; i < (int)m_achievements.size(); ++i )
	{
		const SAchEntry & ach = m_achievements[ i ];

		  // Row and column for this achievement
		const int gx = i % ACH_PER_LINE;
		const int gy = i / ACH_PER_LINE;

		  // Screen coordinates of top-left corner of achievement area
		const int x = 10 + ACH_WIDTH * gx;
		const int y = 80 + ACH_HEIGHT * gy;
	
		  // Draw icon and border
		pRender->DrawRectFill( x + ACH_WIDTH / 2 - 25, y, x + ACH_WIDTH / 2 + 25 + 1, y + 50 + 1, Color( 0, 0, 0 ) );
		if ( ach.m_has )
			pRender->DrawImage( ach.m_icon, x + ACH_WIDTH / 2 - 25, y );
		pRender->DrawRect( x + ACH_WIDTH / 2 - 25, y, x + ACH_WIDTH / 2 + 25 + 1, y + 50 + 1, Color( 200, 200, 200 ) );

		if ( i == m_selectedAchievement )
			pRender->DrawRect( x + ACH_WIDTH / 2 - 26, y - 1, x + ACH_WIDTH / 2 + 25 + 2, y + 50 + 2, Color( 255, 255, 0 ) );

		  // Unselected gained achievement is white
		Color c = Color(255,255,255);

		  // Draw selected achievement's name in yellow
		if ( i == m_selectedAchievement )
		{
			c = Color(255,255,0);
		}
		else
		{
			 // Otherwise, if player doesn't have this achievement, draw it in gray
			if ( !ach.m_has )
				c = Color( 128,128,128 );
		}

		pRender->DrawAlignedText( x + ACH_WIDTH / 2 , y + 55, c, 20, TEXT_ALIGN_CENTER, "%s", ach.m_name.c_str() );
	}

	  // Dimensions of text area with information ( above menu, below achievements )
	const int descW = 400;
	const int descX = SCREEN_W/2 - descW/2;
	const int descY = SCREEN_H - 180;
	const int descH = 70;

	  // Draw border
	pRender->DrawRect( descX, descY, descX + descW, descY + descH, Color(255,255,255) );

	  // We can display either achievement's description or menu entry description here
	const char *info = 0;

	if ( m_inAchievements )
	{
		if ( m_selectedAchievement >= 0 && m_selectedAchievement < (int)m_achievements.size() )
		{
			const SAchEntry & ach = m_achievements[ m_selectedAchievement ];
			info = ach.m_desc.c_str();
		}
	}
	else
	{
		//  // UGLY! To hell with it.
		//switch( m_selected )
		//{		
		//	case 0: 
		//		info = "Export a big screenshot of your achievements with names (1024x768)";
		//		break;
		//	case 1:
		//		info = "Export a small, condensed screenshot of your achievements without names (300x300)";
		//		break;
		//	/*case 2:
		//		info = "Export a very small screenshot of your achievements, suitable for a forum signature (500x20)";
		//		break;*/
			//case 3:
				info = "Go back to main menu";
		//		break;
		//}
	}

	  // Draw description, fitting it into are if possible
	if ( info )
		pRender->DrawTextInArea( descX + 10, descY + 10, descW - 20, descH - 20, Color(230,230,230), 20, "%s", info );
}

void AchMenu::ProcessInput( IInput *pInput )
{
	  // Begin reading events
	pInput->BeginRead();
	  
	while( true )
	{
		const InputEvent & ev = pInput->GetEvent();
		if ( ev.m_type == InputEvent::EV_INVALID )
			break;

		if ( !m_inAchievements )
		{
			bool upEdge = false;
			bool dnEdge = false;

			if ( ev.m_type == InputEvent::EV_KEYBOARD && ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_REPEAT )
			{					
				if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_UP && m_selected == 0 )
					upEdge = true;
				else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_DOWN && m_selected == 3 )
					dnEdge = true;
			}
			else if ( ev.m_type == InputEvent::EV_GAMEPAD )
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

				if ( ( control == JoystickMapping::CONTROL_DPAD_UP || control == JoystickMapping::CONTROL_LEFTSTICK_UP ) && m_selected == 0 )
					upEdge = true;
				else if ( ( control == JoystickMapping::CONTROL_DPAD_DOWN || control == JoystickMapping::CONTROL_LEFTSTICK_DOWN ) && m_selected == 3 )
					dnEdge = true;
			}

			if ( upEdge )
			{
					// If player pressed UP when cursor was on the top-most menu entry,
					// we should move cursor into achievements area.
				m_selected = -1;
				for ( size_t i = 0; i < m_entries.size(); ++i )
				{
					m_entries[ i ]->SetSelected( false );
				}
				m_selectedAchievement = (int)m_achievements.size() - 1;
				m_inAchievements = true;
				pInput->PopEvent();
			}
			if ( dnEdge )
			{
					// Unlike "normal" menus, we don't want cursor to move to the top entry
					// if player pressed DOWN at the bottom-most entry, so we swallow this event
				pInput->PopEvent();
			}
		}
		else
		{
				// If cursor was in achievements section of screen
			int ax = m_selectedAchievement % ACH_PER_LINE;
			int ay = m_selectedAchievement / ACH_PER_LINE;

			while( true )
			{
				const InputEvent & ev = pInput->GetEvent();
				if ( ev.m_type == InputEvent::EV_INVALID )
					break;

				bool handled = false;

				if ( ev.m_type == InputEvent::EV_KEYBOARD )
				{
						// Change selected achievement
					if ( ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_REPEAT )
					{
						handled = true;
						if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_RIGHT )
							++ax;
						else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_LEFT )
							--ax;
						else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_UP )
							--ay;
						else if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_DOWN )
							++ay;
						else
							handled = false;
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

						// Change selected achievement
					if ( control == JoystickMapping::CONTROL_DPAD_RIGHT || control == JoystickMapping::CONTROL_LEFTSTICK_RIGHT )
						++ax;
					if ( control == JoystickMapping::CONTROL_DPAD_LEFT || control == JoystickMapping::CONTROL_LEFTSTICK_LEFT )
						--ax;
					if ( control == JoystickMapping::CONTROL_DPAD_UP || control == JoystickMapping::CONTROL_LEFTSTICK_UP )
						--ay;
					if ( control == JoystickMapping::CONTROL_DPAD_DOWN || control == JoystickMapping::CONTROL_LEFTSTICK_DOWN )
						++ay;

					handled = true;
				}

				if ( ax < 0 ) { --ay; ax = ACH_PER_LINE - 1; }
				if ( ax >= ACH_PER_LINE ) { ++ay; ax = 0; }
				if ( ay < 0 ) { ay = 0; }
				
				m_selectedAchievement = ay * ACH_PER_LINE + ax;

					// If player pressed DOWN while cursor was on the bottom-most line of achievement,
					// return cursor to menu section of screen
				if ( m_selectedAchievement >= (int)m_achievements.size() )
				{
					m_inAchievements = false;
					SetSelected( 0 );
				}

				if ( handled )
					pInput->PopEvent();
				else
					pInput->NextEvent();
			}
		}

		if ( ev.m_type == InputEvent::EV_MOUSE )
		{
			if ( ev.m_mouseEvent.m_type == MouseEvent::MOUSE_B1_DOWN || ev.m_mouseEvent.m_type == MouseEvent::MOUSE_B1_DOUBLECLICK )
			{
				TrySelectAch( ev.m_mouseEvent.m_x, ev.m_mouseEvent.m_y );
			}
		}

		if ( ev.m_type == InputEvent::EV_TOUCH )
		{
			if ( ev.m_touchEvent.m_type == TouchEvent::TOUCH_BEGIN )
			{
				TrySelectAch( ev.m_touchEvent.m_x, ev.m_touchEvent.m_y );
			}
		}

		pInput->NextEvent();
	}

		// Otherwise, this menu acts like a normal menu
	BaseMenu::ProcessInput( pInput );
}

void AchMenu::SaveBinAchievements()
{
//#ifdef WIN32
//	ALLEGRO_FILECHOOSER *dlg = al_create_native_file_dialog( ".", "Enter file name to save", "", ALLEGRO_FILECHOOSER_SAVE );
//	if ( al_show_native_file_dialog( al_get_current_display(), dlg ) )
//	{
//		IRender & render = GetRender();
//		ImageHandle buffer = render.CreateImage( 1024, 768, true, "Ach_Big" );
//
//		render.ClearImage( buffer, Color(0,0,0) );
//		for ( int i = 0; i < (int)m_achievements.size(); ++i )
//		{
//			const SAchEntry & ach = m_achievements[ i ];
//
//			const int gx = i % ACH_PER_LINE;
//			const int gy = i / ACH_PER_LINE;
//
//			const int x = 10 + ACH_WIDTH * gx;
//			const int y = 80 + ACH_HEIGHT * gy;
//		
//			render.DrawRect( buffer, x + ACH_WIDTH / 2 - 25, y, x + ACH_WIDTH / 2 + 25 + 1, y + 50 + 1, Color( 200, 200, 200 ) );
//			if ( ach.m_has )
//				render.DrawImage( buffer, ach.m_icon, x + ACH_WIDTH / 2 - 25, y );
//
//			Color c = Color(255,255,255);
//
//			if ( !ach.m_has )
//				c = Color( 128,128,128 );
//
//			render.DrawAlignedText( buffer, x + ACH_WIDTH / 2 , y + 55, c, 14, TEXT_ALIGN_CENTER, "%s", ach.m_name.c_str() );
//		}
//
//		const char *path = al_get_native_file_dialog_path( dlg, 0 );
//		render.SaveImage( path, buffer );
//		render.DestroyImage( buffer );
//	}
//    al_destroy_native_file_dialog( dlg );	
//#endif
}

void AchMenu::SaveSmallAchievements()
{
//#ifdef WIN32
//	ALLEGRO_FILECHOOSER *dlg = al_create_native_file_dialog( ".", "Enter file name to save", "", ALLEGRO_FILECHOOSER_SAVE );
//	if ( al_show_native_file_dialog( al_get_current_display(), dlg ) )
//	{
//		IRender & render = GetRender();
//		ImageHandle buffer = render.CreateImage( 50*6, 50*6, true, "Ach_Small" );
//
//		render.ClearImage( buffer, Color(0,0,0) );
//		for ( int i = 0; i < (int)m_achievements.size(); ++i )
//		{
//			const SAchEntry & ach = m_achievements[ i ];
//
//			const int gx = i % 6;
//			const int gy = i / 6;
//
//			const int x = 50 * gx;
//			const int y = 50 * gy;
//					
//			if ( ach.m_has )
//				render.DrawImage( buffer, ach.m_icon, x, y );
//			render.DrawRect( buffer, x, y, x + 50, y + 50, Color( 200, 200, 200 ) );
//		}
//
//		const char *path = al_get_native_file_dialog_path( dlg, 0 );
//		render.SaveImage( path, buffer );
//		render.DestroyImage( buffer );
//	}
//    al_destroy_native_file_dialog( dlg );
//#endif
}

bool AchMenu::TrySelectAch( int x, int y )
{
	const int gx = ( x - 10 ) / ACH_WIDTH;
	const int gy = ( y - 80 ) / ACH_HEIGHT;

	const int idx = gy * ACH_PER_LINE + gx;

	if ( idx < 0 || idx >= (int)m_achievements.size() )
		return false;

	m_back.SetSelected( false );
	m_selected = -1;
	m_selectedAchievement = idx;
	m_inAchievements = true;

	return true;
}

void AchMenu::operator()( int id )
{
	switch( id )
	{
		case EID_EXPORT_BIG:
		{
			SaveBinAchievements();
			break;
		}
		case EID_EXPORT_SMALL:
		{
			SaveSmallAchievements();
			break;
		}
		case EID_BACK:
		{
			if ( m_pCallback )
				m_pCallback->BackFromAchievements();
			m_init = false;
			m_inAchievements = false;
			break;
		}
	}
}
