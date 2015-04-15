#pragma once
#include "MenuHelpers.h"
#include "BaseMenu.h"
#include "IRender.h"
#include "JoystickMapping.h"

/*
	Options-specific menus and menu entries
*/

  // A enum for identifying currently active options sub-menu
enum EOptions
{
	OPTIONS_NONE,
	OPTIONS_VIDEO,
	OPTIONS_SOUND,
	OPTIONS_CONTROLS_KEYBOARD,
	OPTIONS_CONTROLS_GAMEPAD,
	OPTIONS_CONTROLS_TOUCH,
	OPTIONS_GAME,
};

class IOptionsMenuCallback
{
public:
	virtual void BackFromOptions() = 0;
	virtual void BackFromSubOptions() = 0;
	virtual void ShowSubOptions( EOptions opt ) = 0;
};

class IKeyEntryCallback
{
public:
	virtual void KeyChanged( int oldKey, int newKey ) = 0;
};

  // Menu entry which allows exactly one key to be entered and displays its name.
  // For redefining controls
class KeyMenuEntry : public ValueMenuEntry
{
protected:
	int m_oldKeyCode;
	int m_keyCode;
	bool m_editing;
	std::string m_keyName;

	IKeyEntryCallback *m_pCallback;

public:
	KeyMenuEntry( BaseMenu *menu, int order, const std::string & label, int keyCode, IKeyEntryCallback *pCallback )
		: ValueMenuEntry( menu, order, label, 800, 64, false )
		, m_keyCode( keyCode )
		, m_oldKeyCode( keyCode )
		, m_editing( false )
		, m_pCallback( pCallback )
	{
		SetKeyCode( keyCode );
	}

	int GetKeyCode() const { return m_keyCode; }
	void SetKeyCode( int keycode ) { m_keyCode = keycode; m_keyName = al_keycode_to_name( m_keyCode ); }
	virtual void ProcessInput( IInput *pInput );

	void Activate()
	{
		m_editing = true;
	}

	const char *CurrentValue()
	{
		return m_editing ? "Press a key" : m_keyName.c_str();
	}
};

class IGamepadEntryCallback
{
public:
	virtual void ControlChanged( JoystickMapping::EControl oldCtrl, JoystickMapping::EControl newCtrl ) = 0;
};

  // Menu entry which allows exactly you to specify gamepad button or axis.
  // For redefining controls
class GamepadMenuEntry : public ValueMenuEntry
{
protected:
	JoystickMapping::EControl m_control;
	JoystickMapping::EControl m_oldControl;
	int m_sign;
	int m_oldSign;
	bool m_editing;
	std::string m_valueName;

	IGamepadEntryCallback *m_pCallback;

public:
	GamepadMenuEntry( BaseMenu *menu, int order, const std::string & label, JoystickMapping::EControl control, IGamepadEntryCallback *pCallback )
		: ValueMenuEntry( menu, order, label, 800, 64, false )
		, m_control( control )
		, m_oldControl( control )
		, m_editing( false )
		, m_pCallback( pCallback )
	{
		SetControl( control );
	}

	JoystickMapping::EControl GetControl() const { return m_control; }
	void SetControl( JoystickMapping::EControl control ) { m_control = control; m_valueName = JoystickMapping::GetControlNameLong( m_control ); }

	virtual void ProcessInput( IInput *pInput );

	void Activate()
	{
		m_editing = true;
	}

	const char *CurrentValue()
	{
		return m_editing ? "Define action" : m_valueName.c_str();
	}
};

enum EControlType
{
	CONTROLTYPE_KEYBOARD,
	CONTROLTYPE_GAMEPAD,
	CONTROLTYPE_TOUCH,
};

class IControlTypeCallback
{
public:
	virtual void ChangeControlType( EControlType type ) = 0;
};

  // Menu entry for types of input devices (Keyboard, Gamepad, Touch)
  // For redefining controls
class ControlTypeMenuEntry : public ValueListMenuEntry<EControlType>
{
protected:
	IControlTypeCallback *m_pCallback;

public:
	ControlTypeMenuEntry( BaseMenu *menu, int order, const std::string & label, IControlTypeCallback *pCallback );

	void Activate()
	{
		NextValue();
	}

	virtual void OnValueChanged( int index )
	{
		m_pCallback->ChangeControlType( m_values[ index ].m_data );
	}
};

  // Description of a display mode
struct SModeDesc
{
	SModeDesc( int _w, int _h, int _b, int _r )
		: w(_w)
		, h(_h)
		, b(_b)
		, r(_r)
	{}

	int w,h,b,r;

	bool operator < ( const SModeDesc & other ) const
	{
		if ( w != other.w )
			return w < other.w;
		if ( h != other.h )
			return h < other.h;

		return b < other.b;
	}
};

  // Menu options class
class MenuOptions : public GameMenu, public ButtonMenuEntry::ICallback
{
	static const int EID_VID_OPTIONS	= 1;
	static const int EID_SND_OPTIONS	= 2;
	static const int EID_CTRL_OPTIONS	= 3;
	static const int EID_GAME_OPTIONS	= 4;
	static const int EID_BACK			= 5;

	VerticalButtonEntry m_vidOptions;
	VerticalButtonEntry m_sndOptions;
	VerticalButtonEntry m_ctrlOptions;
	VerticalButtonEntry m_gameOptions;
	FooterAcceptButtonEntry m_back;

	IOptionsMenuCallback *m_pCallback;

public:
	MenuOptions( int x, int y, IOptionsMenuCallback *pCallback )
		: GameMenu( "Options", SCREEN_W/2 - 450/2, 120, 450, 460 )
		, m_pCallback( pCallback )
		, m_vidOptions		( this, 1, "Video options",		EID_VID_OPTIONS,	this )
		, m_sndOptions		( this, 2, "Sound options",		EID_SND_OPTIONS,	this )
		, m_ctrlOptions		( this, 3, "Control options",	EID_CTRL_OPTIONS,	this )
		, m_gameOptions		( this, 4, "Gameplay options",	EID_GAME_OPTIONS,	this )
		, m_back			( this, 5, "Back",				EID_BACK,			this )
	{
		SetSelected( &m_back );
	}

	void operator()( int id );
	void Back() { this->operator ()( EID_BACK ); }	
};

  // This menu is show when game is paused. It's a simplified version of Options Menu
class PauseMenuOptions : public GameMenu, public ButtonMenuEntry::ICallback
{
	static const int EID_SND_OPTIONS	= 2;
	static const int EID_CTRL_OPTIONS	= 3;
	static const int EID_BACK			= 5;

	VerticalButtonEntry m_sndOptions;
	VerticalButtonEntry m_ctrlOptions;
	FooterAcceptButtonEntry m_back;

	IOptionsMenuCallback *m_pCallback;

public:
	PauseMenuOptions( int x, int y, IOptionsMenuCallback *pCallback )
		: GameMenu( "Options", SCREEN_W/2 - 450/2, 120, 450, 320 )
		, m_pCallback( pCallback )
		, m_sndOptions		( this, 2, "Sound options",		EID_SND_OPTIONS,	this )
		, m_ctrlOptions		( this, 3, "Control options",	EID_CTRL_OPTIONS,	this )
		, m_back			( this, 5, "Back",				EID_BACK,			this )
	{
		SetSelected( &m_back );
	}

	void operator()( int id );
	void Back() { this->operator ()( EID_BACK ); }	
};

  // Base class for options sub-menu
class BaseOptionsMenu : public GameMenu, public ButtonMenuEntry::ICallback
{
	static const int CID_APPLY = 1;
	static const int CID_CANCEL = 2;

	FooterAcceptButtonEntry m_applyEntry;
	FooterDeclineButtonEntry m_cancelEntry;

	IOptionsMenuCallback *m_pCallback;

	virtual void Apply() = 0;

public:
	BaseOptionsMenu( const std::string & caption, int h, IOptionsMenuCallback *pCallback );

	void operator()( int id );

	void Back() { this->operator ()( CID_APPLY ); }		
};

class VideoOptionsMenu : public BaseOptionsMenu, public IEnumerateDisplayModesCallback
{
	ValueListMenuEntry<ERenderType> m_renderType;
	ValueListMenuEntry<bool> m_fullscreenEntry;
	ValueListMenuEntry<size_t> m_resolution;
	OnOffMenuEntry m_vsyncEntry;
	ValueListMenuEntry<EScalingMode> m_scaling;

	std::vector<SModeDesc> m_modes;

	void EnumerateDisplayMode( int bits, int w, int h, int refresh );	
	void Apply();
	void OnShow();

public:
	VideoOptionsMenu( int x, int y, IOptionsMenuCallback *pCallback );
};

class SoundOptionsMenu : public BaseOptionsMenu
{
	ValueListMenuEntry<int> m_masterVolume;
	ValueListMenuEntry<int> m_soundVolume;
	ValueListMenuEntry<int> m_musicVolume;

	void Apply();
	void OnShow();

public:
	SoundOptionsMenu( int x, int y, IOptionsMenuCallback *pCallback );
};

class ControlOptionsMenu_Keyboard : public BaseOptionsMenu, public IKeyEntryCallback
{
	ControlTypeMenuEntry m_controlsType;
	KeyMenuEntry m_keyClockEntry;
	KeyMenuEntry m_keyCounterEntry;
	KeyMenuEntry m_keyAccelEntry;
	KeyMenuEntry m_keyFireEntry;
	KeyMenuEntry m_keyBombEntry;

	void KeyChanged( int oldKey, int newKey );

	void Apply();
	void OnShow();

public:
	ControlOptionsMenu_Keyboard( int x, int y, IOptionsMenuCallback *pCallback, IControlTypeCallback *pControlTypeCallback );
};

class ControlOptionsMenu_Gamepad : public BaseOptionsMenu, public IGamepadEntryCallback
{
	ControlTypeMenuEntry m_controlsType;
	GamepadMenuEntry m_padClockEntry;
	GamepadMenuEntry m_padCounterEntry;
	GamepadMenuEntry m_padAccelEntry;
	GamepadMenuEntry m_padFireEntry;
	GamepadMenuEntry m_padBombEntry;

	void ControlChanged( JoystickMapping::EControl oldCtrl, JoystickMapping::EControl newCtrl );

	void Apply();
	void OnShow();

	void SetEntryFromConfig( const char *name, GamepadMenuEntry & entry, JoystickMapping::EControl defControl );
	void SetConfigFromEntry( const char *name, const GamepadMenuEntry & entry );

public:
	ControlOptionsMenu_Gamepad( int x, int y, IOptionsMenuCallback *pCallback, IControlTypeCallback *pControlTypeCallback );
};

class ControlOptionsMenu_Touch : public BaseOptionsMenu
{
	ControlTypeMenuEntry m_controlsType;

	void Apply();
	void OnShow();

public:
	ControlOptionsMenu_Touch( int x, int y, IOptionsMenuCallback *pCallback, IControlTypeCallback *pControlTypeCallback );
};


class GameplayOptionsMenu : public BaseOptionsMenu
{
	OnOffMenuEntry m_tutorialEntry;
	ValueListMenuEntry<int> m_livesEntry;
	ValueListMenuEntry<int> m_dayLengthEntry;
	OnOffMenuEntry m_aimAidEntry;
	
	void Apply();
	void OnShow();

public:
	GameplayOptionsMenu( int x, int y, IOptionsMenuCallback *pCallback );
};
