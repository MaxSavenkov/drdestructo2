#include "stdafx.h"
#include "GameStateTutorial.h"
#include "Common.h"
#include "ObjectsStorage.h"
#include "Vars.h"
#include "IRender.h"
#include "JoystickMapping.h"

float FADE_TIME = 0.5f;
float PAUSE_TIME = 1.0f;

UseIntVar GM_PLAYER("GM_PLAYER");
UseIntVar GM_ENEMY("GM_ENEMY");
UseIntVar GM_TARGET("GM_TARGET");

void GameStateTutorial::RenderBefore( IRender & render, const IGameContext & context )
{

}

void GameStateTutorial::RenderAfter( IRender & render, const IGameContext & context )
{
	int c = 64;
	if ( m_stage == STAGE_TITLE )
	{
		c = 64 * (1.0f - m_fadeTimer / FADE_TIME);
		if ( c > 64 ) c = 64; if ( c < 0 ) c = 0;
	}
	else if ( m_stage == STAGE_FADE_IN )
	{
		c = 64 * (m_fadeTimer / FADE_TIME);
		if ( c > 64 ) c = 64; if ( c < 0 ) c = 0;
	}
	render.DrawRectFill( 0, 0, SCREEN_W, SCREEN_H, Color(0,0,0,c) );

	render.DrawAlignedText( SCREEN_W/2, 40, Color(255,255,255), 45, TEXT_ALIGN_CENTER, "Tutorial" );
	render.DrawAlignedText( SCREEN_W/2, 85, Color(255,255,255), 20, TEXT_ALIGN_CENTER, "This will only be shown once. You can re-enable tutorial in Options later." );
	render.DrawAlignedText( SCREEN_W/2, 105, Color(255,255,255), 20, TEXT_ALIGN_CENTER, "Press fire or Enter to see the next message. Press ESC to skip" );

	if ( m_stage < STAGE_LAST )
	{
		const SMessage & msg = m_messages[ m_stage ];

		  // 1. Display pulsing rectangle
		const float arg = sinf( (m_blinkTimer / 5.0f * 2 * ALLEGRO_PI) * 5 );
	
		for ( int i = 0; i < (int)msg.m_rects.size(); ++i )
		{
			const SMessage::SRect & rect = msg.m_rects[ i ];
			render.DrawRect( rect.x1 - arg * 5, rect.y1 - arg * 5, rect.x2 + arg * 5, rect.y2 + arg * 5, rect.color );
		}		

		  // 2. Display message text
		render.DrawTextInArea( msg.m_textX, msg.m_textY, msg.m_textW, msg.m_textH, Color(255,255,255), 25, msg.m_text );

		Control (*controls) = 0;
		switch( m_stage )
		{
			case STAGE_CONTROLS_KDB: controls = m_kbdControls; break;
			case STAGE_CONTROLS_PAD: controls = m_padControls; break;
			case STAGE_CONTROLS_TOUCH: break;
		}
		  // 3. For control scheme stages, display control shceme
		if ( controls )
		{
			for ( int i = 0; i < CONTROLS_COUNT; ++i )
			{
				const Control & ctrl = controls[i];
				const int ctrlX = ctrl.m_pos.x;
				const int ctrlY = ctrl.m_pos.y;
				render.DrawImage( ctrl.m_image, ctrlX, ctrlY );
				render.DrawText( ctrlX + ctrl.m_width + 10, ctrlY + ctrl.m_height / 2 - 12, Color(255,255,255), 24, "%s", ctrl.m_controlLabel.c_str() );

				if ( !ctrl.m_controlName.empty() )
				{
					if ( m_stage == STAGE_CONTROLS_KDB )
					{
						  // Handle usual and special keys (usual keys get name from m_controlName, arrow keys don't have names, but have an arrow image)
						if ( ctrl.m_controlName[0] > '\4' )
							render.DrawText( ctrlX + 10, ctrlY + 10, Color(0,0,0), 20, "%s", ctrl.m_controlName.c_str() );
						else if ( ctrl.m_controlName[0] == '\1' )
							render.DrawImage( m_keyArrowLeft, ctrlX + 10, ctrlY + 10 );
						else if ( ctrl.m_controlName[0] == '\2' )
							render.DrawImage( m_keyArrowRight, ctrlX + 10, ctrlY + 10 );
						else if ( ctrl.m_controlName[0] == '\3' )
							render.DrawImage( m_keyArrowUp, ctrlX + 10, ctrlY + 10 );
						else if ( ctrl.m_controlName[0] == '\4' )
							render.DrawImage( m_keyArrowDown, ctrlX + 10, ctrlY + 10 );
					}
					else if ( m_stage == STAGE_CONTROLS_PAD )
					{
						render.DrawAlignedText( ctrlX + ctrl.m_width / 2, ctrlY + ctrl.m_height / 2 - 10, Color(255,255,255), 20, TEXT_ALIGN_CENTER, "%s", ctrl.m_controlName.c_str() );
					}
				}
			}

			const IImage *pArrowImg = render.GetImageByHandle( m_rotateArrow );
			const int arrowW = pArrowImg->GetWidth();
			const int arrowH = pArrowImg->GetHeight();
			render.DrawImage( m_rotateArrow, (msg.m_rects[0].x1 + msg.m_rects[0].x2)/2 - arrowW/2, msg.m_rects[0].y1 - 5 - arrowH );
		}
	}
	else if ( m_stage == STAGE_LAST )
	{
		render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2, Color(255,255,255),20, TEXT_ALIGN_CENTER, "Press Fire to continue" );
	}
}

void GameStateTutorial::Update( float dt, IGameContext & context )
{
	if ( m_fadeTimer > 0.0f )
	{
		m_fadeTimer -= dt;
		if ( m_fadeTimer < 0 )
		{
			m_fadeTimer = 0.0f;
			if ( m_stage == STAGE_TITLE )
				m_stage = STAGE_PLAYER;
			else if ( m_stage == STAGE_FADE_IN )
				context.RemoveState( GAMESTATE_TUTORIAL );
		}
	}

	SMessage & msg = m_messages[ m_stage ];

	m_blinkTimer += dt;
}

  // This struct describes a keyboard key, its shape and name
struct KeyInfo
{
	enum EType { KEY_NORMAL, KEY_WIDE, KEY_SHIFT, KEY_ENTER };

	KeyInfo() : m_type( KEY_NORMAL ), m_name( "?" ) {}
	KeyInfo( int index ) : m_type( KEY_NORMAL )
	{
		char tmp[16];
		sprintf(tmp, "Key %i", index);
		m_name = tmp;
	}
	KeyInfo( EType type, const char *name ) : m_type( type ), m_name( name ) {}

	EType m_type;
	std::string m_name;
};

  // List of keys
KeyInfo keyInfo[ ALLEGRO_KEY_MAX ];

void GameStateTutorial::PrepareControlsInfo()
{
	for ( int i = 0; i < ALLEGRO_KEY_MAX; ++i )
		keyInfo[i] = KeyInfo(i);

	for ( int i = ALLEGRO_KEY_A; i < ALLEGRO_KEY_Z; ++i )
	{
		keyInfo[i].m_type = KeyInfo::KEY_NORMAL;
		keyInfo[i].m_name.assign( 1, 'A' + (i-ALLEGRO_KEY_A) );
	}

	for ( int i = ALLEGRO_KEY_0; i < ALLEGRO_KEY_9; ++i )
	{
		keyInfo[i].m_type = KeyInfo::KEY_NORMAL;
		keyInfo[i].m_name.assign( 1, '0' + (i-ALLEGRO_KEY_0) );
	}

	for ( int i = ALLEGRO_KEY_PAD_0; i < ALLEGRO_KEY_PAD_9; ++i )
	{
		keyInfo[i].m_type = KeyInfo::KEY_WIDE;
		keyInfo[i].m_name = "Pad ";
		keyInfo[i].m_name.append( 1, '0' + (i-ALLEGRO_KEY_PAD_0) );
	}

	for ( int i = ALLEGRO_KEY_F1; i < ALLEGRO_KEY_F12; ++i )
	{
		keyInfo[i].m_type = KeyInfo::KEY_NORMAL;
		keyInfo[i].m_name = "F";
		keyInfo[i].m_name.append( 1, '1' + (i-ALLEGRO_KEY_F1) );
	}

	keyInfo[ALLEGRO_KEY_ESCAPE] = KeyInfo( KeyInfo::KEY_NORMAL, "Esc" );
	keyInfo[ALLEGRO_KEY_TILDE] = KeyInfo( KeyInfo::KEY_NORMAL, "~" );
	keyInfo[ALLEGRO_KEY_MINUS] = KeyInfo( KeyInfo::KEY_NORMAL, "-" );
	keyInfo[ALLEGRO_KEY_EQUALS] = KeyInfo( KeyInfo::KEY_NORMAL, "=" );
	keyInfo[ALLEGRO_KEY_BACKSPACE] = KeyInfo( KeyInfo::KEY_NORMAL, "<-" );
	keyInfo[ALLEGRO_KEY_TAB] = KeyInfo( KeyInfo::KEY_WIDE, "Tab" );
	keyInfo[ALLEGRO_KEY_OPENBRACE] = KeyInfo( KeyInfo::KEY_NORMAL, "[" );
	keyInfo[ALLEGRO_KEY_CLOSEBRACE] = KeyInfo( KeyInfo::KEY_NORMAL, "]" );
	keyInfo[ALLEGRO_KEY_ENTER] = KeyInfo( KeyInfo::KEY_ENTER, "" );
	keyInfo[ALLEGRO_KEY_SEMICOLON] = KeyInfo( KeyInfo::KEY_NORMAL, ";" );
	keyInfo[ALLEGRO_KEY_QUOTE] = KeyInfo( KeyInfo::KEY_NORMAL, "'" );
	keyInfo[ALLEGRO_KEY_BACKSLASH] = KeyInfo( KeyInfo::KEY_NORMAL, "\\" );
	keyInfo[ALLEGRO_KEY_BACKSLASH2] = KeyInfo( KeyInfo::KEY_NORMAL, "\\" );
	keyInfo[ALLEGRO_KEY_COMMA] = KeyInfo( KeyInfo::KEY_NORMAL, "," );
	keyInfo[ALLEGRO_KEY_FULLSTOP] = KeyInfo( KeyInfo::KEY_NORMAL, "." );
	keyInfo[ALLEGRO_KEY_SLASH] = KeyInfo( KeyInfo::KEY_NORMAL, "/" );
	keyInfo[ALLEGRO_KEY_SPACE] = KeyInfo( KeyInfo::KEY_WIDE, "Space" );
	keyInfo[ALLEGRO_KEY_INSERT] = KeyInfo( KeyInfo::KEY_NORMAL, "Ins" );
	keyInfo[ALLEGRO_KEY_DELETE] = KeyInfo( KeyInfo::KEY_NORMAL, "Del" );
	keyInfo[ALLEGRO_KEY_HOME] = KeyInfo( KeyInfo::KEY_NORMAL, "Home" );
	keyInfo[ALLEGRO_KEY_END] = KeyInfo( KeyInfo::KEY_NORMAL, "End" );
	keyInfo[ALLEGRO_KEY_PGUP] = KeyInfo( KeyInfo::KEY_NORMAL, "PgUp" );
	keyInfo[ALLEGRO_KEY_PGDN] = KeyInfo( KeyInfo::KEY_NORMAL, "PgDn" );
	keyInfo[ALLEGRO_KEY_LEFT] = KeyInfo( KeyInfo::KEY_NORMAL, "\1" );
	keyInfo[ALLEGRO_KEY_RIGHT] = KeyInfo( KeyInfo::KEY_NORMAL, "\2" );
	keyInfo[ALLEGRO_KEY_UP] = KeyInfo( KeyInfo::KEY_NORMAL, "\3" );
	keyInfo[ALLEGRO_KEY_DOWN] = KeyInfo( KeyInfo::KEY_NORMAL, "\4" );

	keyInfo[ALLEGRO_KEY_PAD_SLASH] = KeyInfo( KeyInfo::KEY_WIDE, "Pad /" );
	keyInfo[ALLEGRO_KEY_PAD_ASTERISK] = KeyInfo( KeyInfo::KEY_WIDE, "Pad *" );
	keyInfo[ALLEGRO_KEY_PAD_MINUS] = KeyInfo( KeyInfo::KEY_WIDE, "Pad -" );
	keyInfo[ALLEGRO_KEY_PAD_PLUS] = KeyInfo( KeyInfo::KEY_WIDE, "Pad +" );
	keyInfo[ALLEGRO_KEY_PAD_DELETE] = KeyInfo( KeyInfo::KEY_WIDE, "Pad Del" );
	keyInfo[ALLEGRO_KEY_PAD_ENTER] = KeyInfo( KeyInfo::KEY_WIDE, "Pad Enter" );
	keyInfo[ALLEGRO_KEY_PRINTSCREEN] = KeyInfo( KeyInfo::KEY_WIDE, "Pad PrSc" );
	keyInfo[ALLEGRO_KEY_PAUSE] = KeyInfo( KeyInfo::KEY_WIDE, "Pad Pause" );

	keyInfo[ALLEGRO_KEY_ABNT_C1] = KeyInfo( KeyInfo::KEY_NORMAL, "C1" );
	keyInfo[ALLEGRO_KEY_YEN] = KeyInfo( KeyInfo::KEY_NORMAL, "Yen" );
	keyInfo[ALLEGRO_KEY_KANA] = KeyInfo( KeyInfo::KEY_WIDE, "Kana" );
	keyInfo[ALLEGRO_KEY_CONVERT] = KeyInfo( KeyInfo::KEY_WIDE, "Cnv" );
	keyInfo[ALLEGRO_KEY_NOCONVERT] = KeyInfo( KeyInfo::KEY_WIDE, "NoCnv" );

	keyInfo[ALLEGRO_KEY_AT] = KeyInfo( KeyInfo::KEY_NORMAL, "@" );
	keyInfo[ALLEGRO_KEY_CIRCUMFLEX] = KeyInfo( KeyInfo::KEY_NORMAL, "^" );
	keyInfo[ALLEGRO_KEY_COLON2] = KeyInfo( KeyInfo::KEY_NORMAL, ":" );
	keyInfo[ALLEGRO_KEY_KANJI] = KeyInfo( KeyInfo::KEY_WIDE, "Kanji" );

	keyInfo[ALLEGRO_KEY_PAD_EQUALS] = KeyInfo( KeyInfo::KEY_WIDE, "Pad =" );

	keyInfo[ALLEGRO_KEY_BACKQUOTE] = KeyInfo( KeyInfo::KEY_NORMAL, "`" );

	keyInfo[ALLEGRO_KEY_SEMICOLON2] = KeyInfo( KeyInfo::KEY_NORMAL, ";" );

	keyInfo[ALLEGRO_KEY_COMMAND] = KeyInfo( KeyInfo::KEY_WIDE, "Cmd" );

	keyInfo[ALLEGRO_KEY_BACK] = KeyInfo( KeyInfo::KEY_WIDE, "Back" );

	keyInfo[ALLEGRO_KEY_VOLUME_UP] = KeyInfo( KeyInfo::KEY_WIDE, "VolUp" );
	keyInfo[ALLEGRO_KEY_VOLUME_DOWN] = KeyInfo( KeyInfo::KEY_WIDE, "VolDn" );

	keyInfo[ALLEGRO_KEY_LSHIFT] = KeyInfo( KeyInfo::KEY_SHIFT, "L.Shift" );
	keyInfo[ALLEGRO_KEY_RSHIFT] = KeyInfo( KeyInfo::KEY_SHIFT, "R.Shift" );

	keyInfo[ALLEGRO_KEY_LCTRL] = KeyInfo( KeyInfo::KEY_WIDE, "L.Ctrl" );
	keyInfo[ALLEGRO_KEY_RCTRL] = KeyInfo( KeyInfo::KEY_WIDE, "R.Ctrl" );
	keyInfo[ALLEGRO_KEY_ALT] = KeyInfo( KeyInfo::KEY_WIDE, "Alt" );
	keyInfo[ALLEGRO_KEY_ALTGR] = KeyInfo( KeyInfo::KEY_WIDE, "AltGr" );

	keyInfo[ALLEGRO_KEY_LWIN] = KeyInfo( KeyInfo::KEY_WIDE, "L.Win" );
	keyInfo[ALLEGRO_KEY_RWIN] = KeyInfo( KeyInfo::KEY_WIDE, "R.Win" );

	keyInfo[ALLEGRO_KEY_MENU] = KeyInfo( KeyInfo::KEY_WIDE, "Menu" );

	keyInfo[ALLEGRO_KEY_SCROLLLOCK] = KeyInfo( KeyInfo::KEY_WIDE, "ScrlLk" );

	keyInfo[ALLEGRO_KEY_NUMLOCK] = KeyInfo( KeyInfo::KEY_WIDE, "Num.Lk" );

	keyInfo[ALLEGRO_KEY_CAPSLOCK] = KeyInfo( KeyInfo::KEY_WIDE, "CapsLk" );
}

void GameStateTutorial::ConfigureKdbControl( EControl control, const char *keyName )
{
	ISystem & system = GetSystem();
	int key = system.GetConfigValue_Int( "DD_Controls", keyName );

	const char *label = "";
	switch( control )
	{
		case CONTROL_CLOCK: label = ""; /*"Rotate clockwise";*/ break;
		case CONTROL_COUNTER: label = ""; /*"Rotate counter-clockwise";*/ break;
		case CONTROL_FIRE: label = "Fire gun"; break;
		case CONTROL_BOMB: label = "Drop bomb"; break;
		case CONTROL_ACCEL: label = "Accelerate"; break;
	}

	if ( key < 0 ) key = 0;
	if ( key >= ALLEGRO_KEY_MAX ) key = ALLEGRO_KEY_MAX - 1;

	const char *imgName = "";
	switch( keyInfo[key].m_type )
	{
		case KeyInfo::KEY_NORMAL: imgName = "Data/Sprites2/ui/tutorial_key_normal.png"; break;
		case KeyInfo::KEY_WIDE: imgName = "Data/Sprites2/ui/tutorial_key_large.png"; break;
		case KeyInfo::KEY_SHIFT: imgName = "Data/Sprites2/ui/tutorial_key_shift.png"; break;
		case KeyInfo::KEY_ENTER: imgName = "Data/Sprites2/ui/tutorial_key_enter.png"; break;
	}

	Control & ctrl = m_kbdControls[ control ];

	ctrl.m_controlName = keyInfo[key].m_name;
	ctrl.m_controlLabel = label;
	ctrl.m_image = GetRender().LoadImage( imgName );
	IImage *pImg = GetRender().GetImageByHandle( ctrl.m_image );
	ctrl.m_width = pImg->GetWidth();
	ctrl.m_height = pImg->GetHeight();
}

void GameStateTutorial::ConfigurePadControl( const JoystickMapping & mapping, EControl control, const char *name )
{
	const char *label = "";
	switch( control )
	{
		case CONTROL_CLOCK: label = ""; /*"Rotate clockwise";*/ break;
		case CONTROL_COUNTER: label = ""; /*"Rotate counter-clockwise";*/ break;
		case CONTROL_FIRE: label = "Fire gun"; break;
		case CONTROL_BOMB: label = "Drop bomb"; break;
		case CONTROL_ACCEL: label = "Accelerate"; break;
	}

	ISystem & system = GetSystem();

	JoystickMapping::EControl joyControl = (JoystickMapping::EControl)system.GetConfigValue_Int( "DD_Controls", name, JoystickMapping::CONTROL_INVALID );

	Control & ctrl = m_padControls[ control ];

	std::string iconPath = "Data/Sprites2/ui/";
	iconPath += mapping.GetControlNameIcon( joyControl );
	iconPath += ".png";

	ctrl.m_controlName = mapping.GetControlNameShort( joyControl );
	ctrl.m_controlLabel = label;
	ctrl.m_image = GetRender().LoadImage( iconPath.c_str() );
	IImage *pImg = GetRender().GetImageByHandle( ctrl.m_image );
	ctrl.m_width = pImg->GetWidth();
	ctrl.m_height = pImg->GetHeight();
}

void GameStateTutorial::OnPush( IGameContext & context )
{
	m_fadeTimer = FADE_TIME;
	m_stage = STAGE_TITLE;
	m_blinkTimer = 0;
	

	  // Iterate through all objects and find some we can use in the course of the tutorial
	const ObjectsStorage::GameObjectContainer & objs = context.GetOjectsStorage().GetGameObjects();
	for ( ObjectsStorage::GameObjectContainer::ConstIterator iter = objs.GetConstIterator();
		  !iter.AtEnd();
		  ++iter )
	{
		const MechanicComponent *pMech = context.GetOjectsStorage().GetMechanicComponent( (*iter)->GetMechanicComponent() );
		const GraphicComponent *pRend = context.GetOjectsStorage().GetGraphicComponent( (*iter)->GetGraphicComponent() );
		if ( !pMech || !pRend )
			continue;

		if ( pMech->m_type == GM_PLAYER )
		{
			int plX1 = pRend->m_x - pRend->GetFrameW() / 2;
			int plY1 = pRend->m_y - pRend->GetFrameH() / 2;
			int plX2 = pRend->m_x + pRend->GetFrameW() / 2;
			int plY2 = pRend->m_y + pRend->GetFrameH() / 2;

			SMessage baseMsg;
			baseMsg.m_rects.push_back( SMessage::SRect( plX1  - 10, plY1 - 10, plX2 + 10, plY2 + 10, Color( 255, 255, 255) ) );
			baseMsg.m_textX = plX1 + 10; baseMsg.m_textY = plY1 - 120; baseMsg.m_textW = 500; baseMsg.m_textH = 400;

			SMessage & msg = m_messages[ STAGE_PLAYER ];
			msg.Copy( baseMsg );
			msg.m_text->AssignCstr( "This is your plane. You can fly it to the edge of the screen, but beware of stalling if you go too high!" );

			SMessage & msgKbd = m_messages[ STAGE_CONTROLS_KDB ];
			msgKbd.Copy( baseMsg );
			msgKbd.m_text->AssignCstr( "Keyboard controls:" );

			SMessage & msgPad = m_messages[ STAGE_CONTROLS_PAD ];
			msgPad.Copy( baseMsg );
			msgPad.m_text->AssignCstr( "Gamepad/Joystick controls:" );

			SMessage & msgTouch = m_messages[ STAGE_CONTROLS_TOUCH ];
			msgTouch.Copy( baseMsg );
			msgTouch.m_text->AssignCstr( "Touch controls:" );
		}
		else if ( pMech->m_type == GM_TARGET )
		{
			int tx1 = pRend->m_x - pRend->GetFrameW() / 2;
			int ty1 = pRend->m_y - pRend->GetFrameH() / 2;
			int tx2 = pRend->m_x + pRend->GetFrameW() / 2;
			int ty2 = pRend->m_y + pRend->GetFrameH() / 2;

			SMessage & msg = m_messages[ STAGE_TARGET ];
			msg.m_rects.push_back( SMessage::SRect( tx1 - 10, ty1 - 10, tx2 + 10, ty2 + 10, Color( 255, 0, 0 ) ) );
			msg.m_textX = tx1 + 20; msg.m_textY = ty1 - 81; msg.m_textW = 400; msg.m_textH = 400;
			msg.m_text->AssignCstr( "This is level's target. You have to sink it by dropping enemy planes onto it." );
		}
		else if ( pMech->m_type == GM_ENEMY )
		{
			int ex1 = pRend->m_x - pRend->GetFrameW() / 2;
			int ey1 = pRend->m_y - pRend->GetFrameH() / 2;
			int ex2 = pRend->m_x + pRend->GetFrameW() / 2;
			int ey2 = pRend->m_y + pRend->GetFrameH() / 2;

			if ( ex1 > 0 && ex2 < SCREEN_W && ey1 > 0 && ey2 < SCREEN_H )
			{
				SMessage & msg = m_messages[ STAGE_ENEMIES ];
				msg.m_rects.push_back( SMessage::SRect( ex1 - 10, ey1 - 10, ex2 + 10, ey2 + 10, Color( 255, 0, 0 ) ) );
			}
		}
	}

	{
		SMessage & msg = m_messages[ STAGE_ENEMIES ];
		msg.m_textX = SCREEN_W/2-300; msg.m_textY = SCREEN_H / 2 - 200; msg.m_textW = 600; msg.m_textH = 600;
		msg.m_text->AssignCstr( "Those are enemies. Some of them are harmless, but collision with others will send your plane crashing. Discovering which is which is a part of the game." );
	}


	{
		SMessage & msg = m_messages[ STAGE_HUD ];
		msg.m_textX = SCREEN_W/2 - 400; msg.m_textY = SCREEN_H - 128; msg.m_textW = 800; msg.m_textH = 30;
		msg.m_rects.push_back( SMessage::SRect( 310, 710, 420, 771, Color( 0, 255, 0 ) ) );
		msg.m_rects.push_back( SMessage::SRect( 515, 710, 560, 771, Color( 0, 255, 0 ) ) );
		msg.m_text->AssignCstr( "This is number of lives and days you have left. Don't let either of those reach 0 or you lose." );
	}

	PrepareControlsInfo();

	ConfigureKdbControl( CONTROL_CLOCK, "Kbd_Clock" );
	ConfigureKdbControl( CONTROL_COUNTER, "Kbd_Counter" );
	ConfigureKdbControl( CONTROL_ACCEL, "Kbd_Accel" );
	ConfigureKdbControl( CONTROL_FIRE, "Kbd_Fire" );
	ConfigureKdbControl( CONTROL_BOMB, "Kbd_Bomb" );

	const JoystickMapping & mapping = GetJoystickMapping(0);

	ConfigurePadControl( mapping, CONTROL_CLOCK, "Pad_Clock" );
	ConfigurePadControl( mapping, CONTROL_COUNTER, "Pad_Counter" );
	ConfigurePadControl( mapping, CONTROL_ACCEL, "Pad_Accel" );
	ConfigurePadControl( mapping, CONTROL_FIRE, "Pad_Fire" );
	ConfigurePadControl( mapping, CONTROL_BOMB, "Pad_Bomb" );

	SetupControlPositions( m_kbdControls );
	SetupControlPositions( m_padControls );

	IRender & render = GetRender();
	m_keyArrowLeft  = render.LoadImage( "Data/Sprites2/ui/tutorial_arrow_left.png" );
	m_keyArrowRight = render.LoadImage( "Data/Sprites2/ui/tutorial_arrow_right.png" );
	m_keyArrowUp    = render.LoadImage( "Data/Sprites2/ui/tutorial_arrow_up.png" );
	m_keyArrowDown  = render.LoadImage( "Data/Sprites2/ui/tutorial_arrow_down.png" );
	m_rotateArrow   = render.LoadImage( "Data/Sprites2/ui/tutorial_arrow.png" );

	const int keyFire = GetSystem().GetConfigValue_Int( "DD_Controls", "Kbd_Fire" );

	  // Skip one phase:
	AddTouchHandler( this, &GameStateTutorial::SkipPhase,  TouchEvent::TOUCH_BEGIN );
	AddKeyboardHandler( this, &GameStateTutorial::SkipPhase,  KeyboardEvent::KBD_KEY_UP, keyFire ? keyFire    : ALLEGRO_KEY_Q );
	AddKeyboardHandler( this, &GameStateTutorial::SkipPhaseAlt,  KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_ENTER );
	AddGamepadHandlerFromConfig( "Pad_Fire", this, &GameStateTutorial::SkipPhase );

  	  // Ship the whole tutorial
	AddKeyboardHandler( this, &GameStateTutorial::Skip, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_ESCAPE );
	AddGamepadHandlerFromConfig( "Pad_Bomb", this, &GameStateTutorial::Skip );
}

void GameStateTutorial::SetupControlPositions( Control (&controls)[CONTROLS_COUNT] )
{
	SMessage & msg = m_messages[ STAGE_PLAYER ];

	controls[ CONTROL_CLOCK ].m_pos.x = msg.m_rects[0].x2 + 25;
	controls[ CONTROL_CLOCK ].m_pos.y = msg.m_rects[0].y1 - controls[ CONTROL_CLOCK ].m_height - 5;

	controls[ CONTROL_COUNTER ].m_pos.x = msg.m_rects[0].x1 - controls[ CONTROL_COUNTER ].m_width - 25;
	controls[ CONTROL_COUNTER ].m_pos.y = msg.m_rects[0].y1 - controls[ CONTROL_COUNTER ].m_height - 5;

	controls[ CONTROL_ACCEL ].m_pos.x = msg.m_rects[0].x1 + 50 - controls[ CONTROL_ACCEL ].m_width;
	controls[ CONTROL_ACCEL ].m_pos.y = msg.m_rects[0].y2 + 20;

	controls[ CONTROL_FIRE ].m_pos.x = msg.m_rects[0].x1 + 50 - controls[ CONTROL_FIRE ].m_width;
	controls[ CONTROL_FIRE ].m_pos.y = msg.m_rects[0].y2 + 20 + controls[ CONTROL_ACCEL ].m_height + 10;

	controls[ CONTROL_BOMB ].m_pos.x = msg.m_rects[0].x1 + 50 - controls[ CONTROL_BOMB ].m_width;
	controls[ CONTROL_BOMB ].m_pos.y = msg.m_rects[0].y2 + 20 + controls[ CONTROL_ACCEL ].m_height + 10 + controls[ CONTROL_FIRE ].m_height + 10;	
}

void GameStateTutorial::OnRemove( IGameContext & context )
{
	GetSystem().SetConfigValue_Int( "DD_Gameplay", "DisableTutorial", 1 );
	GetSystem().SaveConfig();
}

bool GameStateTutorial::Skip( const InputEvent & e)
{
	if ( e.m_type == InputEvent::EV_GAMEPAD && e.m_gamepadEvent.m_type != GamepadEvent::GP_BUTTON_UP )
		return true;

	m_stage = STAGE_FADE_IN;
	m_fadeTimer = FADE_TIME;
	return true;
}

bool GameStateTutorial::SkipPhase( const InputEvent & e)
{
	if ( e.m_type == InputEvent::EV_GAMEPAD && e.m_gamepadEvent.m_type != GamepadEvent::GP_BUTTON_UP )
		return true;

	if ( m_stage >= STAGE_LAST )
	{
		m_stage = STAGE_FADE_IN;
		m_fadeTimer = FADE_TIME;
	}
	else
	{
		while( true )
		{
			m_stage = (EStage)((int)m_stage + 1);
			if ( m_stage == STAGE_CONTROLS_KDB && !GetInput().HasHardwareKeyboardInstalled() )
				continue;
			if ( m_stage == STAGE_CONTROLS_PAD && !GetInput().HasGamepadInstalled() )
				continue;
			if ( m_stage == STAGE_CONTROLS_TOUCH && !GetInput().HasTouchInstalled() )
				continue;

			break;
		}
		m_blinkTimer = 10.0f;
	}
	return true;
}

bool GameStateTutorial::SkipPhaseAlt( const InputEvent & e)
{
	return SkipPhase( e );
}
