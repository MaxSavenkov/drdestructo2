#pragma once

#include "BaseGameState.h"
#include "BaseMenu.h"
#include "MenuHelpers.h"
#include "OptionsMenu.h"
#include "Common.h"

/*
	Pause menu is just like main menu, only with fewer options
	and an option to resume game.
*/

class IPauseMenuCallback
{
public:
	virtual void ResumeGame() = 0;
	virtual void ShowOptions() = 0;
	virtual void ExitToMenu() = 0;
};

class PauseMenu : public GameMenu, public ButtonMenuEntry::ICallback
{
public:
	static const int EID_RESUME			= 1;
	static const int EID_OPTIONS		= 3;
	static const int EID_EXIT			= 6;

private:
	VerticalButtonEntry m_resumeGame;
	VerticalButtonEntry m_options;
	FooterAcceptButtonEntry m_exit;

	IPauseMenuCallback *m_pCallback;

public:
	PauseMenu( IPauseMenuCallback *pCallback )
		: GameMenu( "Pause", SCREEN_W/2 - 450/2, 120, 450, 320 )
		, m_pCallback( pCallback )
		, m_resumeGame		( this, 0, "Resume game",		EID_RESUME,			this )
		, m_options			( this, 1, "Options",			EID_OPTIONS,		this )
		, m_exit			( this, 2, "Exit to title",		EID_EXIT,			this )
	{
		SetSelected( &m_resumeGame );
	}

	void operator()( int id );
};

class GameStatePause : public BaseGameState, public IPauseMenuCallback, public IOptionsMenuCallback, public IControlTypeCallback
{
	BaseMenu *m_pMenu;

	PauseMenu m_pauseMenu;
	
	bool m_resumeGame;
	bool m_showOptions;
	bool m_exitGame;

	PauseMenuOptions m_options;
	VideoOptionsMenu m_vidOptions;
	SoundOptionsMenu m_sndOptions;
	ControlOptionsMenu_Keyboard m_ctrlOptionsKbd;
	ControlOptionsMenu_Gamepad m_ctrlOptionsPad;
	ControlOptionsMenu_Touch m_ctrlOptionsTouch;
	GameplayOptionsMenu m_gameOptions;
	bool m_backFromOptions;
	bool m_backFromSubOptions;
	EOptions m_showSubOptions;

public:
	GameStatePause()
		: BaseGameState( GAMESTATE_PAUSE )
		, m_pauseMenu( this )
		, m_resumeGame( false )
		, m_showOptions( false )
		, m_showSubOptions( OPTIONS_NONE )
		, m_exitGame( false )
		, m_options( 300, 200, this )
		, m_vidOptions( 300, 200, this )
		, m_sndOptions( 300, 200, this )
		, m_ctrlOptionsKbd( 300, 200, this, this )
		, m_ctrlOptionsPad( 300, 200, this, this )
		, m_ctrlOptionsTouch( 300, 200, this, this )
		, m_gameOptions( 300, 200, this )
		, m_backFromOptions( false )
		, m_backFromSubOptions( false )
		, m_pMenu( &m_pauseMenu )
	{
	}

	void RenderBefore( IRender & render, const IGameContext & context );
	void RenderAfter( IRender & render, const IGameContext & context );
	void Update( float dt, IGameContext & context );
	void ProcessInput( IInput & input ) { m_pMenu->ProcessInput( &input ); }

	bool AllowPhysicsUpdate() const { return false; }
	bool AllowGraphicsUpdate() const { return false; }
	bool AllowGraphicsRender() const { return false; }
	bool AllowAIUpdate() const { return false; }

	void OnPush( IGameContext & context );
	void OnRemove( IGameContext & context ) {}

	void ResumeGame()
	{
		m_resumeGame = true;
	}
	
	void ShowOptions()
	{
		m_showOptions = true;
	}

	void ExitToMenu()
	{
		m_exitGame = true;
	}

	void BackFromOptions()
	{
		m_backFromOptions = true;
	}

	void BackFromSubOptions()
	{
		m_backFromSubOptions = true;
	}

	void ShowSubOptions( EOptions opt )
	{
		m_showSubOptions = opt;
	}

	void ChangeControlType( EControlType type )
	{
		switch( type )
		{
		case CONTROLTYPE_KEYBOARD:
			m_showSubOptions = OPTIONS_CONTROLS_KEYBOARD;
			break;
		case CONTROLTYPE_GAMEPAD:
			m_showSubOptions = OPTIONS_CONTROLS_GAMEPAD;
			break;
		case CONTROLTYPE_TOUCH:
			m_showSubOptions = OPTIONS_CONTROLS_TOUCH;
			break;
		}
	}
};
