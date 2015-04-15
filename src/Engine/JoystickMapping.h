#pragma once

/*
	This class maps gamepad buttons and axes reported by Allegro to a set of universal XInput-compatible values

	EControl -> { button OR stick + axis + sign }
*/

class JoystickMapping
{
public:
	JoystickMapping();

	enum EControl
	{
		CONTROL_INVALID,

		CONTROL_A,
		CONTROL_B,
		CONTROL_X,
		CONTROL_Y,
		CONTROL_BACK,
		CONTROL_GUIDE,
		CONTROL_START,
		CONTROL_LEFTSTICK_PUSH,
		CONTROL_RIGHTSTICK_PUSH,
		CONTROL_LEFTSHOULDER,
		CONTROL_RIGHTSHOULDER,
		CONTROL_DPAD_LEFT,
		CONTROL_DPAD_RIGHT,
		CONTROL_DPAD_UP,
		CONTROL_DPAD_DOWN,
		CONTROL_RIGHTTRIGGER,
		CONTROL_LEFTTRIGGER,
		CONTROL_LEFTSTICK_UP,
		CONTROL_LEFTSTICK_DOWN,
		CONTROL_LEFTSTICK_LEFT,
		CONTROL_LEFTSTICK_RIGHT,
		CONTROL_RIGHTSTICK_UP,
		CONTROL_RIGHTSTICK_DOWN,
		CONTROL_RIGHTSTICK_LEFT,
		CONTROL_RIGHTSTICK_RIGHT,

		CONTROLS_COUNT
	};

public:
	  // Returns name of mapping (not used)
	const char *GetName() const { return m_name.c_str(); }

	  // Returns short control name (like "A", "RT" etc)
	static const char *GetControlNameShort( EControl control );
	  // Returns long control name (like "A", "R.Trigger" etc)
	static const char *GetControlNameLong( EControl control );
	  // Returns hard-coded icon name for control
	static const char *GetControlNameIcon( EControl control );

	  // Returns EControl by button index reported by Allegro
	EControl FindButton( int button ) const;
	  // Returns EControl by stick index + axis index + position reported by Allegro
	EControl FindStick( int stick, int axis, float pos ) const;

	  // Returns mapping for a given control in supplied variables
	bool GetControl( EControl control, int & button, int & stick, int & axis, int & sign, int & baseValue ) const;

	  // Returns base, neutral value for an axis, because some systems have -1 as a base values for triggers (Linux, I'm looking at you)
	float GetAxisBase( int stick, int axis ) const;

private:
	std::string m_name;

	struct SControlInfo
	{
		std::string m_shortName;
		std::string m_longName;
		std::string m_icon;
	};

	static SControlInfo m_controlInfo[ CONTROLS_COUNT ];

	struct SControlMapping
	{
		int button;
		int stick;
		int axis;
		int sign;
		int baseValue;
	};

	SControlMapping m_controlMap[ CONTROLS_COUNT ];

	static const int MAX_STICKS = 10;
	static const int MAX_AXES   = 5;
	float m_axisBases[ MAX_STICKS ][ MAX_AXES ];

	friend void CreateDefaultMappings();

	void SetButton( EControl control, int button )
	{
		m_controlMap[ control ].button = button;
		m_controlMap[ control ].stick = -1;
		m_controlMap[ control ].axis = -1;
		m_controlMap[ control ].sign = 0;
	}

	void SetStick( EControl control, int stick, int axis, int sign, int baseValue = 0 )
	{
		m_controlMap[ control ].button = -1;
		m_controlMap[ control ].stick = stick;
		m_controlMap[ control ].axis = axis;
		m_controlMap[ control ].sign = sign;
		m_controlMap[ control ].baseValue = baseValue;

		if ( stick >= 0 && stick < MAX_STICKS && axis >= 0 && axis < MAX_AXES )
			m_axisBases[ stick ][ axis ] = (float)baseValue;
	}
};

const JoystickMapping & GetJoystickMapping( int joystickIndex );
