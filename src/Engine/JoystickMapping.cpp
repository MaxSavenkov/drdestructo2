#include "stdafx.h"
#include "JoystickMapping.h"

typedef std::map<std::string, JoystickMapping> Mappings;
static Mappings mappings;

JoystickMapping::SControlInfo JoystickMapping::m_controlInfo[ JoystickMapping::CONTROLS_COUNT ] =
{
	  // Yeah, that really shouldn't be hard-coded in an Engine class, but this late into development, I don't care
	{ "?", "?", "tutorial_gamepad_button" }, //CONTROL_INVALID,
	{ "A", "A", "tutorial_gamepad_button" }, //CONTROL_A,
	{ "B", "B", "tutorial_gamepad_button" },//CONTROL_B,
	{ "X", "X", "tutorial_gamepad_button" },//CONTROL_X,
	{ "Y", "Y", "tutorial_gamepad_button" },//CONTROL_Y,
	{ "",  "Back", "tutorial_gamepad_select" },//CONTROL_BACK,
	{ "",  "Guide", "tutorial_gamepad_button" },//CONTROL_GUIDE,
	{ "", "Start", "tutorial_gamepad_start" },//CONTROL_START,
	{ "L", "L.Stick", "tutorial_gamepad_hat_press" },//CONTROL_LEFTSTICK_PUSH,
	{ "R", "R.Stick", "tutorial_gamepad_hat_press" },//CONTROL_RIGHTSTICK_PUSH,
	{ "LS", "L.Shoulder", "tutorial_gamepad_shoulder" },//CONTROL_LEFTSHOULDER,
	{ "RS", "R.Shoulder", "tutorial_gamepad_shoulder" },//CONTROL_RIGHTSHOULDER,
	{ "", "Left", "tutorial_gamepad_dpad_left" },//CONTROL_DPAD_LEFT,
	{ "", "Right", "tutorial_gamepad_dpad_right" },//CONTROL_DPAD_RIGHT,
	{ "", "Up", "tutorial_gamepad_dpad_up" },//CONTROL_DPAD_UP,
	{ "", "Down", "tutorial_gamepad_dpad_down" },//CONTROL_DPAD_DOWN,
	{ "RT", "R.Trigger", "tutorial_gamepad_trigger" },//CONTROL_RIGHTTRIGGER,
	{ "LT", "L.Trigger", "tutorial_gamepad_trigger" },//CONTROL_LEFTTRIGGER,
	{ "L", "L.Stick Up", "tutorial_gamepad_dpad_up" },//CONTROL_LEFTSTICK_UP,
	{ "L", "L.Stick Down", "tutorial_gamepad_dpad_down" },///CONTROL_LEFTSTICK_DOWN,
	{ "L", "L.Stick Left", "tutorial_gamepad_dpad_left" },///CONTROL_LEFTSTICK_LEFT,
	{ "L", "L.Stick Right", "tutorial_gamepad_dpad_right" },///CONTROL_LEFTSTICK_RIGHT,
	{ "R", "R.Stick Up", "tutorial_gamepad_hat_up" },///CONTROL_RIGHTSTICK_UP,
	{ "R", "R.Stick Down", "tutorial_gamepad_hat_down" },//CONTROL_RIGHTSTICK_DOWN,
	{ "R", "R.Stick Left", "tutorial_gamepad_dpad_left" },//CONTROL_RIGHTSTICK_LEFT,
	{ "R", "R.Stick Right", "tutorial_gamepad_dpad_right" },//CONTROL_RIGHTSTICK_RIGHT,
};

void CreateDefaultMappings()
{
#ifdef WIN32
	{
		JoystickMapping & xinput = mappings.insert( std::make_pair( "78696e70757400000000000000000000", JoystickMapping() ) ).first->second;
		xinput.SetButton( JoystickMapping::CONTROL_A, 0 );
		xinput.SetButton( JoystickMapping::CONTROL_B, 1 );
		xinput.SetButton( JoystickMapping::CONTROL_X, 2 );
		xinput.SetButton( JoystickMapping::CONTROL_Y, 3 );
		xinput.SetButton( JoystickMapping::CONTROL_RIGHTSHOULDER, 4 );
		xinput.SetButton( JoystickMapping::CONTROL_LEFTSHOULDER, 5 );
		xinput.SetButton( JoystickMapping::CONTROL_RIGHTSTICK_PUSH, 6 );
		xinput.SetButton( JoystickMapping::CONTROL_LEFTSTICK_PUSH, 7 );
		xinput.SetButton( JoystickMapping::CONTROL_BACK, 8 );
		xinput.SetButton( JoystickMapping::CONTROL_START, 9 );
		xinput.SetButton( JoystickMapping::CONTROL_DPAD_RIGHT, 10 );
		xinput.SetButton( JoystickMapping::CONTROL_DPAD_LEFT, 11 );
		xinput.SetButton( JoystickMapping::CONTROL_DPAD_DOWN, 12 );
		xinput.SetButton( JoystickMapping::CONTROL_DPAD_UP, 13 );

		xinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_LEFT,	0, 0, -1  );
		xinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_RIGHT,	0, 0, 1  );
		xinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_UP,		0, 1, -1  );
		xinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_DOWN,	0, 1, 1  );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_LEFT,	1, 0, -1  );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_RIGHT, 1, 0, 1  );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_UP,	1, 1, -1  );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_DOWN,	1, 1, 1  );
		xinput.SetStick( JoystickMapping::CONTROL_LEFTTRIGGER,		2, 0, 1  );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTTRIGGER,		3, 0, 1  );
	}

	{
		JoystickMapping & dinput = mappings.insert( std::make_pair( "6d0416c2000000000000504944564944", JoystickMapping() ) ).first->second;

		dinput.SetButton( JoystickMapping::CONTROL_A, 1 );
		dinput.SetButton( JoystickMapping::CONTROL_B, 2 );
		dinput.SetButton( JoystickMapping::CONTROL_X, 0 );
		dinput.SetButton( JoystickMapping::CONTROL_Y, 3 );
		dinput.SetButton( JoystickMapping::CONTROL_LEFTSHOULDER, 4 );
		dinput.SetButton( JoystickMapping::CONTROL_RIGHTSHOULDER, 5 );		
		dinput.SetButton( JoystickMapping::CONTROL_LEFTTRIGGER, 6 );
		dinput.SetButton( JoystickMapping::CONTROL_RIGHTTRIGGER, 7 );
		dinput.SetButton( JoystickMapping::CONTROL_BACK, 8 );
		dinput.SetButton( JoystickMapping::CONTROL_START, 9 );
		dinput.SetButton( JoystickMapping::CONTROL_LEFTSTICK_PUSH, 10 );
		dinput.SetButton( JoystickMapping::CONTROL_RIGHTSTICK_PUSH, 11 );

		dinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_LEFT,	0, 0, -1  );
		dinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_RIGHT,	0, 0, 1  );
		dinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_UP,		0, 1, -1  );
		dinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_DOWN,	0, 1, 1  );
		dinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_LEFT,	0, 2, -1  );
		dinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_RIGHT, 0, 2, 1  );
		dinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_UP,	1, 0, -1  );
		dinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_DOWN,	1, 0, 1  );
		dinput.SetStick( JoystickMapping::CONTROL_DPAD_RIGHT,		2, 0, 1 );
		dinput.SetStick( JoystickMapping::CONTROL_DPAD_LEFT,		2, 0, -1 );
		dinput.SetStick( JoystickMapping::CONTROL_DPAD_DOWN,		2, 1, 1 );
		dinput.SetStick( JoystickMapping::CONTROL_DPAD_UP,			2, 1, -1 );
	}
#elif __APPLE__
	{
		JoystickMapping & dinput = mappings.insert( std::make_pair( "6d0400000000000016c2000000000000", JoystickMapping() ) ).first->second;

		dinput.SetButton( JoystickMapping::CONTROL_A, 1 );
		dinput.SetButton( JoystickMapping::CONTROL_B, 2 );
		dinput.SetButton( JoystickMapping::CONTROL_X, 0 );
		dinput.SetButton( JoystickMapping::CONTROL_Y, 3 );
		dinput.SetButton( JoystickMapping::CONTROL_LEFTSHOULDER, 4 );
		dinput.SetButton( JoystickMapping::CONTROL_RIGHTSHOULDER, 5 );		
		dinput.SetButton( JoystickMapping::CONTROL_LEFTTRIGGER, 6 );
		dinput.SetButton( JoystickMapping::CONTROL_RIGHTTRIGGER, 7 );		
		dinput.SetButton( JoystickMapping::CONTROL_BACK, 8 );
		dinput.SetButton( JoystickMapping::CONTROL_START, 9 );
		dinput.SetButton( JoystickMapping::CONTROL_LEFTSTICK_PUSH, 10 );
		dinput.SetButton( JoystickMapping::CONTROL_RIGHTSTICK_PUSH, 11 );

		dinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_LEFT,	0, 0, -1  );
		dinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_RIGHT,	0, 0, 1  );
		dinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_UP,		0, 1, -1  );
		dinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_DOWN,	0, 1, 1  );
		dinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_LEFT,	0, 2, -1  );
		dinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_RIGHT, 0, 2, 1  );
		dinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_UP,	1, 0, -1  );
		dinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_DOWN,	1, 0, 1  );
		dinput.SetStick( JoystickMapping::CONTROL_DPAD_RIGHT,		2, 0, 1 );
		dinput.SetStick( JoystickMapping::CONTROL_DPAD_LEFT,		2, 0, -1 );
		dinput.SetStick( JoystickMapping::CONTROL_DPAD_DOWN,		2, 1, 1 );
		dinput.SetStick( JoystickMapping::CONTROL_DPAD_UP,			2, 1, -1 );
	}

#else
	{
		JoystickMapping & xinput = mappings.insert( std::make_pair( "030000006d0400001dc2000014400000", JoystickMapping() ) ).first->second;
		xinput.SetButton( JoystickMapping::CONTROL_A, 0 );
		xinput.SetButton( JoystickMapping::CONTROL_B, 1 );
		xinput.SetButton( JoystickMapping::CONTROL_X, 2 );
		xinput.SetButton( JoystickMapping::CONTROL_Y, 3 );
		xinput.SetButton( JoystickMapping::CONTROL_RIGHTSHOULDER, 5 );
		xinput.SetButton( JoystickMapping::CONTROL_LEFTSHOULDER, 4 );
		xinput.SetButton( JoystickMapping::CONTROL_RIGHTSTICK_PUSH, 10 );
		xinput.SetButton( JoystickMapping::CONTROL_LEFTSTICK_PUSH, 9 );
		xinput.SetButton( JoystickMapping::CONTROL_BACK, 6 );
		xinput.SetButton( JoystickMapping::CONTROL_START, 7 );
		xinput.SetButton( JoystickMapping::CONTROL_GUIDE, 8 );

		xinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_LEFT,	0, 0, -1  );
		xinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_RIGHT,	0, 0, 1  );
		xinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_UP,		0, 1, -1  );
		xinput.SetStick( JoystickMapping::CONTROL_LEFTSTICK_DOWN,	0, 1, 1  );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_LEFT,	1, 1, -1  );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_RIGHT, 1, 1, 1  );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_UP,	2, 0, -1  );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTSTICK_DOWN,	2, 0, 1  );
		xinput.SetStick( JoystickMapping::CONTROL_LEFTTRIGGER,		1, 0, 1, -1 );
		xinput.SetStick( JoystickMapping::CONTROL_RIGHTTRIGGER,		2, 1, 1, -1 );
		xinput.SetStick( JoystickMapping::CONTROL_DPAD_RIGHT,		3, 0, 1 );
		xinput.SetStick( JoystickMapping::CONTROL_DPAD_LEFT,		3, 0, -1 );
		xinput.SetStick( JoystickMapping::CONTROL_DPAD_DOWN,		3, 1, 1 );
		xinput.SetStick( JoystickMapping::CONTROL_DPAD_UP,			3, 1, -1 );
	}
#endif
}

JoystickMapping::JoystickMapping()
{
	for ( int i = 0; i < CONTROLS_COUNT; ++i )
	{
		m_controlMap[i].button = -1;
		m_controlMap[i].stick = -1;
		m_controlMap[i].axis = -1;
		m_controlMap[i].sign = 0;
	}

	for ( int s = 0; s < MAX_STICKS; ++s )
	for ( int a = 0; a < MAX_AXES; ++a )
	{
		m_axisBases[s][a] = 0.0f;
	}
}

const JoystickMapping & GetJoystickMapping( int joystickIndex )
{
	static bool init = false;
	if ( !init )
	{
		init = true;
		CreateDefaultMappings();
	}

	Mappings::const_iterator iter = mappings.end();

	ALLEGRO_JOYSTICK *pJoy = al_get_joystick( joystickIndex );	
	if ( pJoy )
	{
		const char *guid = al_get_joystick_guid_string( pJoy );
		if ( guid )
			iter = mappings.find( guid );
	}

	if ( iter == mappings.end() )
	{
#ifdef WIN32
		iter = mappings.find( "6d0416c2000000000000504944564944" ); // return generic DirectInput device if no other mapping is found
#elif __APPLE__
		iter = mappings.find( "6d0400000000000016c2000000000000" ); // return generic DirectInput device if no other mapping is found
#else // LINUX
		iter = mappings.find( "030000006d0400001dc2000014400000" ); // return generic XInput device if no other mapping is found
#endif
	}

	return iter->second;
}

const char *JoystickMapping::GetControlNameShort( JoystickMapping::EControl control )
{
	return m_controlInfo[ control ].m_shortName.c_str();
}

const char *JoystickMapping::GetControlNameLong( JoystickMapping::EControl control )
{
	return m_controlInfo[ control ].m_longName.c_str();
}

const char *JoystickMapping::GetControlNameIcon( JoystickMapping::EControl control )
{
	return m_controlInfo[ control ].m_icon.c_str();
}

JoystickMapping::EControl JoystickMapping::FindButton( int button ) const
{
	for ( int i = 0; i < CONTROLS_COUNT; ++i )
	{
		if ( m_controlMap[i].button == button )
			return (EControl)i;
	}

	return CONTROL_INVALID;
}

JoystickMapping::EControl JoystickMapping::FindStick( int stick, int axis, float pos ) const
{
	for ( int i = 0; i < CONTROLS_COUNT; ++i )
	{
		const float value = pos - m_controlMap[i].baseValue;
		const int sign = value < 0 ? -1 : 1;
		if ( m_controlMap[i].stick == stick && m_controlMap[i].axis == axis && m_controlMap[i].sign == sign )
			return (EControl)i;
	}

	return CONTROL_INVALID;
}

bool JoystickMapping::GetControl( JoystickMapping::EControl control, int & button, int & stick, int & axis, int & sign, int & baseValue ) const
{
	const SControlMapping & m = m_controlMap[control];
	if ( m.button < 0 && m.stick < 0 )
		return false;

	button = m.button;
	stick = m.stick;
	axis = m.axis;
	sign = m.sign;
	baseValue = m.baseValue;

	return true;
}

float JoystickMapping::GetAxisBase( int stick, int axis ) const
{
	if ( stick < 0 || stick >= MAX_STICKS || axis < 0 || axis >= MAX_AXES )
		return 0.0f;

	return m_axisBases[ stick ][ axis ];
}

