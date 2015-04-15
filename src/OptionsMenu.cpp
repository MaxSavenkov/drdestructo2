#include "stdafx.h"
#include "OptionsMenu.h"
#include "Game.h"
#include "Common.h"
#include "IInput.h"
#include "ISystem.h"

#ifndef WIN32
    #define _snprintf snprintf
#endif


int g_options_version = -1;

ControlTypeMenuEntry::ControlTypeMenuEntry( BaseMenu *menu, int order, const std::string & label, IControlTypeCallback *pCallback )
	: ValueListMenuEntry<EControlType>( menu, order, label )
	, m_pCallback( pCallback )
{
	IInput & input = GetInput();
	if ( input.HasHardwareKeyboardInstalled() )
		AddValue( "Keyboard", CONTROLTYPE_KEYBOARD );
	if ( input.HasGamepadInstalled() )
		AddValue( "Gamepad", CONTROLTYPE_GAMEPAD );
	if ( input.HasTouchInstalled() )
		AddValue( "Touch", CONTROLTYPE_TOUCH );
}


void KeyMenuEntry::ProcessInput( IInput *pInput )
{
	pInput->BeginRead();
	while( true )
	{
		InputEvent ev = pInput->GetEvent();
		if ( ev.m_type == InputEvent::EV_INVALID )
			break;

		if ( ev.m_type == InputEvent::EV_KEYBOARD && 
			( ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_DOWN ) )
		{
			if ( !m_editing )
			{
				if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ENTER )
				{
					Activate();
					pInput->PopEvent();
					continue;
				}
			}
			else
			{
				if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ESCAPE )
				{
					m_editing = false;
				}
				else
				{
					if ( m_pCallback )
						m_pCallback->KeyChanged( m_keyCode, ev.m_keyboardEvent.m_keycode );
					m_keyCode = ev.m_keyboardEvent.m_keycode;
					m_keyName = al_keycode_to_name( m_keyCode );
					m_editing = false;
				}

				pInput->Clear();
				return;
			}
		}

		if ( ev.m_type == InputEvent::EV_GAMEPAD )
		{
			const JoystickMapping & mapping = GetJoystickMapping(0);
			JoystickMapping::EControl control = mapping.FindButton(ev.m_gamepadEvent.m_button);
			if ( control == JoystickMapping::CONTROL_A )
			{
				if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_UP ) 
				{
					Activate();
					pInput->PopEvent();
					continue;
				}
			}
			else if ( m_editing && control == JoystickMapping::CONTROL_B )
			{
				m_editing = false;
				pInput->PopEvent();
				continue;
			}
		}
		
		if ( m_editing )
			pInput->PopEvent();
		else
			pInput->NextEvent();
	}
}

void GamepadMenuEntry::ProcessInput( IInput *pInput )
{
	pInput->BeginRead();
	while( true )
	{
		InputEvent ev = pInput->GetEvent();
		if ( ev.m_type == InputEvent::EV_INVALID )
			break;

		if ( ev.m_type == InputEvent::EV_KEYBOARD && 
			( ev.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_DOWN ) )
		{
			if ( !m_editing )
			{
				if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ENTER )
				{
					Activate();
					pInput->PopEvent();
					continue;
				}
			}
			else
			{
				if ( ev.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ESCAPE )
				{
					m_editing = false;
				}

				pInput->Clear();
				return;
			}
		}

		if ( ev.m_type == InputEvent::EV_GAMEPAD )
		{
			if ( m_editing )
			{
				if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_UP )
				{
					const JoystickMapping & mapping = GetJoystickMapping( 0 );
					const JoystickMapping::EControl newCtrl = mapping.FindButton( ev.m_gamepadEvent.m_button );
					if ( m_pCallback )
						m_pCallback->ControlChanged( m_control, newCtrl );
					SetControl( newCtrl );
					m_editing = false;
					pInput->Clear();
					return;
				}

				if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_AXIS && fabsf(ev.m_gamepadEvent.m_pos) > 0.00001f )
				{
					const JoystickMapping & mapping = GetJoystickMapping( 0 );
					const int sign = ev.m_gamepadEvent.m_pos > 0 ? 1 : -1;
					const JoystickMapping::EControl newCtrl = mapping.FindStick( ev.m_gamepadEvent.m_stick, ev.m_gamepadEvent.m_axis, sign );
					if ( m_pCallback )
						m_pCallback->ControlChanged( m_control, newCtrl );
					SetControl( newCtrl );
					m_editing = false;
					pInput->Clear();
					return;
				}
			}
			else
			{
				const JoystickMapping & mapping = GetJoystickMapping(0);
				if ( mapping.FindButton(ev.m_gamepadEvent.m_button) == JoystickMapping::CONTROL_A )
				{
					if ( ev.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_UP ) 
					{
						Activate();
						pInput->PopEvent();
						continue;
					}
				}
			}
		}

		if ( m_editing )
			pInput->PopEvent();
		else
			pInput->NextEvent();
	}
}

void MenuOptions::operator()( int id )
{
	switch( id )
	{
		case EID_BACK:
		{
			m_pCallback->BackFromOptions();
			break;
		}
		case EID_VID_OPTIONS:
		{
			m_pCallback->ShowSubOptions( OPTIONS_VIDEO );
			break;
		}
		case EID_SND_OPTIONS:
		{
			m_pCallback->ShowSubOptions( OPTIONS_SOUND );
			break;
		}
		case EID_CTRL_OPTIONS:
		{
#if defined(ALLEGRO_ANDROID) || defined(ALLEGRO_IPHONE)
			m_pCallback->ShowSubOptions( OPTIONS_CONTROLS_TOUCH );
#else
			m_pCallback->ShowSubOptions( OPTIONS_CONTROLS_KEYBOARD );
#endif
			break;
		}
		case EID_GAME_OPTIONS:
		{
			m_pCallback->ShowSubOptions( OPTIONS_GAME );
			break;
		}
	}
}

void PauseMenuOptions::operator()( int id )
{
	switch( id )
	{
		case EID_BACK:
		{
			m_pCallback->BackFromOptions();
			break;
		}
		/*case EID_VID_OPTIONS:
		{
			m_pCallback->ShowSubOptions( OPTIONS_VIDEO );
			break;
		}*/
		case EID_SND_OPTIONS:
		{
			m_pCallback->ShowSubOptions( OPTIONS_SOUND );
			break;
		}
		case EID_CTRL_OPTIONS:
		{
#if defined(ALLEGRO_ANDROID) || defined(ALLEGRO_IPHONE)
			m_pCallback->ShowSubOptions( OPTIONS_CONTROLS_TOUCH );
#else
			m_pCallback->ShowSubOptions( OPTIONS_CONTROLS_KEYBOARD );
#endif
			break;
		}
		/*case EID_GAME_OPTIONS:
		{
			m_pCallback->ShowSubOptions( OPTIONS_GAME );
			break;
		}*/
	}
}

BaseOptionsMenu::BaseOptionsMenu( const std::string & caption, int h, IOptionsMenuCallback *pCallback )
	: GameMenu( caption, 30, 30, 800, h )
	, m_pCallback( pCallback )
	, m_cancelEntry    ( this, 255, "Cancel", CID_CANCEL, this )
	, m_applyEntry     ( this, 254, "Apply", CID_APPLY, this )
{

}

VideoOptionsMenu::VideoOptionsMenu( int x, int y, IOptionsMenuCallback *pCallback )
	: BaseOptionsMenu( "Video options", 500, pCallback )
	, m_renderType     ( this, 0, "Render type" )
	, m_fullscreenEntry( this, 1, "Screen mode" )
	, m_resolution	   ( this, 2, "Resolution" )
	, m_vsyncEntry	   ( this, 3, "VSync", true )
	, m_scaling  	   ( this, 4, "Scaling" )	
{
	SetSelected( &m_renderType );

	ISystem & system = GetSystem();

#ifdef _WINDOWS_
	m_renderType.AddValue( "Direct3D", RENDERTYPE_DIRECT3D );
#endif

	m_renderType.AddValue( "OpenGL", RENDERTYPE_OPENGL );

	m_fullscreenEntry.AddValue( "Windowed", false );
	m_fullscreenEntry.AddValue( "Fullscreen", true );

	m_scaling.AddValue( "Keep aspect ratio", SCALING_KEEP_ASPECT );
	m_scaling.AddValue( "Do not keep aspect ratio", SCALING_FREE );

	GetRender().EnumerateDisplayModes( *this );

	std::sort( m_modes.begin(), m_modes.end() );
	for ( size_t i = 0; i < m_modes.size(); ++i )
	{
		const SModeDesc & mode = m_modes[ i ];
		char tmp[256];

		_snprintf( tmp, 255, "%ix%i %ibit", mode.w, mode.h, mode.b );
		const int newIndex = m_resolution.AddValue( tmp, i );
	}

	OnShow();
}

void VideoOptionsMenu::OnShow()
{
	ISystem & system = GetSystem();

	int renderMode = system.GetConfigValue_Int( "DD_Graphics", "RenderMode" );
	if ( renderMode < 0 || renderMode >= m_renderType.GetValuesCount() )
		renderMode = 0;
	m_renderType.SetSelectedIndex( renderMode );

	m_fullscreenEntry.SetSelectedIndex( system.GetConfigValue_Int( "DD_Graphics", "Fullscreen" ) );

	int scaling = system.GetConfigValue_Int( "DD_Graphics", "Scaling" );
	if ( scaling <= 0 )
		scaling = SCALING_KEEP_ASPECT;
	m_scaling.SetSelectedIndex( scaling - 1 );

	const int cfgW	= system.GetConfigValue_Int( "DD_Graphics", "Width" );
	const int cfgH	= system.GetConfigValue_Int( "DD_Graphics", "Height" );
	
	for ( size_t i = 0; i < m_modes.size(); ++i )
	{
		const SModeDesc & mode = m_modes[ i ];

		if ( cfgW == mode.w && cfgH == mode.h )
			m_resolution.SetSelectedIndex( (int)i );
	}

	if ( m_resolution.GetSelectedIndex() == -1 )
	{
		m_resolution.SetSelectedIndex( 0 );
		for ( int i = 0; i < (int)m_modes.size(); ++i )
		{
			if ( m_modes[i].w == 1280 && m_modes[i].h == 960 && m_modes[i].b >= 24 )
				m_resolution.SetSelectedIndex( i );
		}
	}

	int vsync = system.HasConfigValue( "DD_Graphics", "VSync" ) ? system.GetConfigValue_Int( "DD_Graphics", "VSync" ) : 1;
	m_vsyncEntry.SetSelectedIndex( vsync != 0 ? 1 : 0 );
}

void VideoOptionsMenu::Apply()
{
	ISystem & system = GetSystem();

	system.SetConfigValue_Int( "DD_Graphics", "RenderMode", m_renderType.GetSelectedIndex() );
	system.SetConfigValue_Int( "DD_Graphics", "Fullscreen", m_fullscreenEntry.GetSelectedIndex() );
	system.SetConfigValue_Int( "DD_Graphics", "Scaling", m_scaling.GetSelectedIndex() + 1 );
	system.SetConfigValue_Int( "DD_Graphics", "VSync", m_vsyncEntry.GetSelectedIndex() );

	const int modeIndex = m_resolution.GetSelectedIndex();
	if ( modeIndex >= 0 )
	{
		system.SetConfigValue_Int( "DD_Graphics", "Width",    m_modes[ modeIndex ].w );	
		system.SetConfigValue_Int( "DD_Graphics", "Height",   m_modes[ modeIndex ].h );	
		system.SetConfigValue_Int( "DD_Graphics", "Depth",    m_modes[ modeIndex ].b );	
		system.SetConfigValue_Int( "DD_Graphics", "Refresh",  m_modes[ modeIndex ].r );

		GetRender().SetGlobalScaling( (EScalingMode)(m_scaling.GetSelectedIndex() + 1), SCREEN_W, SCREEN_H );

		const int renderType = m_renderType.GetSelectedIndex();
		const bool hasDirect3D = m_renderType.GetValuesCount() > 1;
		ERenderType realType = RENDERTYPE_OPENGL;
		if ( hasDirect3D )
		{
			realType = renderType == 0 ? RENDERTYPE_DIRECT3D : RENDERTYPE_OPENGL;
		}
		const bool vsync = m_vsyncEntry.GetSelectedIndex();
		GetRender().Reinit( realType, m_fullscreenEntry.GetSelectedIndex(), m_modes[ modeIndex ].w, m_modes[ modeIndex ].h, vsync );
	}	
}

SoundOptionsMenu::SoundOptionsMenu( int x, int y, IOptionsMenuCallback *pCallback )
	: BaseOptionsMenu( "Sound options", 370, pCallback )
	, m_masterVolume   ( this, 0,  "Master volume" )
	, m_soundVolume    ( this, 1,  "Sound volume" )
	, m_musicVolume    ( this, 2,  "Music volume" )
{
	SetSelected( &m_masterVolume );

	for ( int i = 0; i <= 10; ++i )
	{
		char tmp[255];
		sprintf( tmp, "%i%%", i * 10 );
		m_masterVolume.AddValue( tmp, i * 10 );
		m_soundVolume.AddValue( tmp, i * 10 );
		m_musicVolume.AddValue( tmp, i * 10 );
	}

	OnShow();
	Apply();
}

void SoundOptionsMenu::OnShow()
{
	ISystem & system = GetSystem();

	int masterVolume = system.HasConfigValue( "DD_Sound", "Master" ) ? system.GetConfigValue_Int( "DD_Sound", "Master" ) : 100;
	int soundVolume = system.HasConfigValue( "DD_Sound", "Sound" ) ? system.GetConfigValue_Int( "DD_Sound", "Sound" ) : 100;
	int musicVolume = system.HasConfigValue( "DD_Sound", "Music" ) ? system.GetConfigValue_Int( "DD_Sound", "Music" ) : 100;

	if ( masterVolume < 0 )		masterVolume = 0;
	if ( soundVolume < 0 )		soundVolume = 0;
	if ( musicVolume < 0 )		musicVolume = 0;
	if ( masterVolume > 100 )		masterVolume = 100;
	if ( soundVolume > 100 )		soundVolume = 100;
	if ( musicVolume > 100 )		musicVolume = 100;

	m_masterVolume.SetSelectedIndex( masterVolume / 10 );
	m_musicVolume.SetSelectedIndex( musicVolume / 10 );
	m_soundVolume.SetSelectedIndex( soundVolume / 10 );
}

void SoundOptionsMenu::Apply()
{
	ISystem & system = GetSystem();
	
	system.SetConfigValue_Int( "DD_Sound", "Master", m_masterVolume.GetSelectedIndex() * 10 );
	system.SetConfigValue_Int( "DD_Sound", "Sound", m_soundVolume.GetSelectedIndex() * 10 );
	system.SetConfigValue_Int( "DD_Sound", "Music", m_musicVolume.GetSelectedIndex() * 10 );
	GetSound().SetMasterVolume( m_masterVolume.GetSelectedIndex() * 10 / 100.0f );
	GetSound().SetVolumeByType( ISample::TYPE_SOUND, m_soundVolume.GetSelectedIndex() * 10 / 100.0f );
	GetSound().SetVolumeByType( ISample::TYPE_MUSIC, m_musicVolume.GetSelectedIndex() * 10 / 100.0f );
}

ControlOptionsMenu_Keyboard::ControlOptionsMenu_Keyboard( int x, int y, IOptionsMenuCallback *pCallback, IControlTypeCallback *pControlTypeCallback )
	: BaseOptionsMenu( "Control options", 564, pCallback )
	, m_controlsType   ( this, 0, "Controller", pControlTypeCallback )
	, m_keyClockEntry  ( this, 1, "Turn right", ALLEGRO_KEY_F, this )
	, m_keyCounterEntry( this, 2, "Turn left", ALLEGRO_KEY_S, this )
	, m_keyAccelEntry  ( this, 3, "Accelerate", ALLEGRO_KEY_Q, this )
	, m_keyFireEntry   ( this, 4, "Fire gun", ALLEGRO_KEY_D, this )
	, m_keyBombEntry   ( this, 5, "Drop bomb", ALLEGRO_KEY_A, this )
{
	SetSelected( &m_controlsType );
	OnShow();
	Apply();
}

void ControlOptionsMenu_Keyboard::OnShow()
{
	ISystem & system = GetSystem();

	int keyClock	= system.GetConfigValue_Int( "DD_Controls", "Kbd_Clock" );
	int keyCounter	= system.GetConfigValue_Int( "DD_Controls", "Kbd_Counter" );
	int keyAccel	= system.GetConfigValue_Int( "DD_Controls", "Kbd_Accel" );
	int keyFire		= system.GetConfigValue_Int( "DD_Controls", "Kbd_Fire" );
	int keyBomb		= system.GetConfigValue_Int( "DD_Controls", "Kbd_Bomb" );

	m_keyClockEntry.SetKeyCode(		keyClock == 0	? ALLEGRO_KEY_RIGHT : keyClock );
	m_keyCounterEntry.SetKeyCode(	keyCounter == 0 ? ALLEGRO_KEY_LEFT  : keyCounter );
	m_keyAccelEntry.SetKeyCode(		keyAccel == 0	? ALLEGRO_KEY_UP    : keyAccel );
	m_keyFireEntry.SetKeyCode(		keyFire == 0	? ALLEGRO_KEY_Q     : keyFire );
	m_keyBombEntry.SetKeyCode(		keyBomb == 0	? ALLEGRO_KEY_A     : keyBomb );

	m_controlsType.SetSelectedIndexByData( CONTROLTYPE_KEYBOARD );
	SetSelected(0);
}

void ControlOptionsMenu_Keyboard::Apply()
{
	ISystem & system = GetSystem();

	system.SetConfigValue_Int( "DD_Controls", "Kbd_Clock",   m_keyClockEntry.GetKeyCode() );
	system.SetConfigValue_Int( "DD_Controls", "Kbd_Counter", m_keyCounterEntry.GetKeyCode() );
	system.SetConfigValue_Int( "DD_Controls", "Kbd_Accel",   m_keyAccelEntry.GetKeyCode() );
	system.SetConfigValue_Int( "DD_Controls", "Kbd_Fire",    m_keyFireEntry.GetKeyCode() );
	system.SetConfigValue_Int( "DD_Controls", "Kbd_Bomb",    m_keyBombEntry.GetKeyCode() );
}


ControlOptionsMenu_Gamepad::ControlOptionsMenu_Gamepad( int x, int y, IOptionsMenuCallback *pCallback, IControlTypeCallback *pControlTypeCallback )
	: BaseOptionsMenu( "Control options", 564, pCallback )
	, m_controlsType   ( this, 0, "Controller", pControlTypeCallback )
	, m_padClockEntry  ( this, 1, "Turn right", JoystickMapping::CONTROL_DPAD_RIGHT,   this )
	, m_padCounterEntry( this, 2, "Turn left",  JoystickMapping::CONTROL_DPAD_LEFT,    this )
	, m_padAccelEntry  ( this, 3, "Accelerate", JoystickMapping::CONTROL_RIGHTTRIGGER, this )
	, m_padFireEntry   ( this, 4, "Fire gun",   JoystickMapping::CONTROL_A,			   this )
	, m_padBombEntry   ( this, 5, "Drop bomb",  JoystickMapping::CONTROL_B,			   this )
{
	SetSelected( &m_controlsType );
	OnShow();
	Apply();
}

void ControlOptionsMenu_Gamepad::SetEntryFromConfig(const char *name, GamepadMenuEntry & entry, JoystickMapping::EControl defControl)
{
	ISystem & system = GetSystem();

	JoystickMapping::EControl control = (JoystickMapping::EControl)system.GetConfigValue_Int( "DD_Controls", name, defControl );
	entry.SetControl( control );
}

void ControlOptionsMenu_Gamepad::SetConfigFromEntry( const char *name, const GamepadMenuEntry & entry )
{
	ISystem & system = GetSystem();

	system.SetConfigValue_Int( "DD_Controls", name, entry.GetControl() );
}

void ControlOptionsMenu_Gamepad::OnShow()
{
	ISystem & system = GetSystem();

	SetEntryFromConfig( "Pad_Clock",   m_padClockEntry,		JoystickMapping::CONTROL_DPAD_RIGHT );
	SetEntryFromConfig( "Pad_Counter", m_padCounterEntry,   JoystickMapping::CONTROL_DPAD_LEFT );
	SetEntryFromConfig( "Pad_Accel",   m_padAccelEntry,     JoystickMapping::CONTROL_RIGHTTRIGGER );
	SetEntryFromConfig( "Pad_Fire",    m_padFireEntry,		JoystickMapping::CONTROL_A );
	SetEntryFromConfig( "Pad_Bomb",    m_padBombEntry,		JoystickMapping::CONTROL_B );

	m_controlsType.SetSelectedIndexByData( CONTROLTYPE_GAMEPAD );
	SetSelected(0);
}

void ControlOptionsMenu_Gamepad::Apply()
{
	ISystem & system = GetSystem();

	SetConfigFromEntry( "Pad_Clock", m_padClockEntry );
	SetConfigFromEntry( "Pad_Counter", m_padCounterEntry );
	SetConfigFromEntry( "Pad_Accel", m_padAccelEntry );
	SetConfigFromEntry( "Pad_Fire", m_padFireEntry );
	SetConfigFromEntry( "Pad_Bomb", m_padBombEntry );
}

ControlOptionsMenu_Touch::ControlOptionsMenu_Touch( int x, int y, IOptionsMenuCallback *pCallback, IControlTypeCallback *pControlTypeCallback )
	: BaseOptionsMenu( "Control options", 564, pCallback )
	, m_controlsType   ( this, 0, "Controller", pControlTypeCallback )
{
	OnShow();
	Apply();
	SetSelected( &m_controlsType );
}

void ControlOptionsMenu_Touch::OnShow()
{
	ISystem & system = GetSystem();

	//int keyClock	= system.GetConfigValue_Int( "DD_Controls", "Clock" );
	//int keyCounter	= system.GetConfigValue_Int( "DD_Controls", "Counter" );
	//int keyAccel	= system.GetConfigValue_Int( "DD_Controls", "Accel" );
	//int keyFire		= system.GetConfigValue_Int( "DD_Controls", "Fire" );
	//int keyBomb		= system.GetConfigValue_Int( "DD_Controls", "Bomb" );

	//m_keyClockEntry.SetKeyCode(		keyClock == 0	? ALLEGRO_KEY_RIGHT : keyClock );
	//m_keyCounterEntry.SetKeyCode(	keyCounter == 0 ? ALLEGRO_KEY_LEFT  : keyCounter );
	//m_keyAccelEntry.SetKeyCode(		keyAccel == 0	? ALLEGRO_KEY_UP    : keyAccel );
	//m_keyFireEntry.SetKeyCode(		keyFire == 0	? ALLEGRO_KEY_Q     : keyFire );
	//m_keyBombEntry.SetKeyCode(		keyBomb == 0	? ALLEGRO_KEY_A     : keyBomb );

	m_controlsType.SetSelectedIndexByData( CONTROLTYPE_TOUCH );
	SetSelected(0);
}

void ControlOptionsMenu_Touch::Apply()
{
	ISystem & system = GetSystem();

	//system.SetConfigValue_Int( "DD_Controls", "Clock",   m_keyClockEntry.GetKeyCode() );
	//system.SetConfigValue_Int( "DD_Controls", "Counter", m_keyCounterEntry.GetKeyCode() );
	//system.SetConfigValue_Int( "DD_Controls", "Accel",   m_keyAccelEntry.GetKeyCode() );
	//system.SetConfigValue_Int( "DD_Controls", "Fire",    m_keyFireEntry.GetKeyCode() );
	//system.SetConfigValue_Int( "DD_Controls", "Bomb",    m_keyBombEntry.GetKeyCode() );
}

GameplayOptionsMenu::GameplayOptionsMenu( int x, int y, IOptionsMenuCallback *pCallback )
	: BaseOptionsMenu( "Gameplay options", 500, pCallback )
	, m_tutorialEntry  ( this, 0, "Enable tutorial", true   ) 
	, m_livesEntry     ( this, 1, "Starting lives" ) 
	, m_dayLengthEntry ( this, 2, "Day length (minutes)" ) 
	, m_aimAidEntry    ( this, 3, "Enable aiming aid", true  ) 
{
	SetSelected( &m_tutorialEntry );

	ISystem & system = GetSystem();

	m_livesEntry.AddValue( "5  (Very hard)", 5  );
	m_livesEntry.AddValue( "7  (Hard)",      7  );
	m_livesEntry.AddValue( "10 (Easy)",      10 );
	m_livesEntry.AddValue( "21 (Very easy)", 21 );
	
	m_dayLengthEntry.AddValue( "7  (49 min.)", 7 );
	m_dayLengthEntry.AddValue( "8  (56 min.)", 8 );
	m_dayLengthEntry.AddValue( "9  (63 min.)", 9 );
	m_dayLengthEntry.AddValue( "10 (70 min.)", 10 );

	OnShow();
	Apply();
}

void GameplayOptionsMenu::OnShow()
{
	ISystem & system = GetSystem();

	const int aimAid = system.HasConfigValue( "DD_Gameplay", "AimAid" ) ? system.GetConfigValue_Int( "DD_Gameplay", "AimAid" ) : 1;
	const int tutorial = system.HasConfigValue( "DD_Gameplay", "DisableTutorial" ) ? !system.GetConfigValue_Int( "DD_Gameplay", "DisableTutorial" ) : 1;
	m_aimAidEntry.SetSelectedIndex( aimAid );
	m_tutorialEntry.SetSelectedIndex( tutorial );

	if ( !system.HasConfigValue( "DD_Gameplay", "Lives" ) )
	{
		system.SetConfigValue_Int( "DD_Gameplay", "Lives", 10 );
		m_livesEntry.SetSelectedIndex( 2 );
	}
	else
	{
		const int lives = system.GetConfigValue_Int( "DD_Gameplay", "Lives" );
		m_livesEntry.SetSelectedIndexByData( lives );
	}
	
	if ( !system.HasConfigValue( "DD_Gameplay", "Length" ) )
	{
		system.SetConfigValue_Int( "DD_Gameplay", "Length", 10 );
		m_dayLengthEntry.SetSelectedIndex( 3 );
	}
	else
	{
		const int length = system.GetConfigValue_Int( "DD_Gameplay", "Length" );
		m_dayLengthEntry.SetSelectedIndexByData( length );
	}
}

void GameplayOptionsMenu::Apply()
{
	ISystem & system = GetSystem();

	system.SetConfigValue_Int( "DD_Gameplay", "DisableTutorial",  !m_tutorialEntry.GetSelectedIndex() );
	system.SetConfigValue_Int( "DD_Gameplay", "AimAid",  m_aimAidEntry.GetSelectedIndex() );
	system.SetConfigValue_Int( "DD_Gameplay", "Lives", *m_livesEntry.GetSelectedData() );
	system.SetConfigValue_Int( "DD_Gameplay", "Length", *m_dayLengthEntry.GetSelectedData() );
}

void BaseOptionsMenu::operator()( int id )
{
	if ( id == CID_APPLY )
	{
		ISystem & system = GetSystem();
		Apply();
		system.SaveConfig();
		++g_options_version;		
	}

	m_pCallback->BackFromSubOptions();
}

void VideoOptionsMenu::EnumerateDisplayMode( int bits, int w, int h, int refresh )
{
	for ( size_t i = 0; i < m_modes.size(); ++i )
	{
		SModeDesc & mode = m_modes[ i ];
		if ( mode.w == w && mode.h == h )
		{
			if ( bits > mode.b )
				mode.b = bits;
			return;
		}
	}

	m_modes.push_back( SModeDesc( w, h, bits, refresh ) );
}

void ControlOptionsMenu_Keyboard::KeyChanged( int oldKey, int newKey )
{
	if ( m_keyClockEntry.GetKeyCode() == newKey ) m_keyClockEntry.SetKeyCode( oldKey );
	if ( m_keyCounterEntry.GetKeyCode() == newKey ) m_keyCounterEntry.SetKeyCode( oldKey );
	if ( m_keyAccelEntry.GetKeyCode() == newKey ) m_keyAccelEntry.SetKeyCode( oldKey );
	if ( m_keyFireEntry.GetKeyCode() == newKey ) m_keyFireEntry.SetKeyCode( oldKey );
	if ( m_keyBombEntry.GetKeyCode() == newKey ) m_keyBombEntry.SetKeyCode( oldKey );
}

void ControlOptionsMenu_Gamepad::ControlChanged( JoystickMapping::EControl oldCtrl, JoystickMapping::EControl newCtrl )
{
	if ( m_padClockEntry.GetControl() == newCtrl ) m_padClockEntry.SetControl( oldCtrl );
	if ( m_padCounterEntry.GetControl() == newCtrl ) m_padCounterEntry.SetControl( oldCtrl );
	if ( m_padAccelEntry.GetControl() == newCtrl ) m_padAccelEntry.SetControl( oldCtrl );
	if ( m_padFireEntry.GetControl() == newCtrl ) m_padFireEntry.SetControl( oldCtrl );
	if ( m_padBombEntry.GetControl() == newCtrl ) m_padBombEntry.SetControl( oldCtrl );
}
