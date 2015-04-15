#pragma once
#include "BaseGameState.h"
#include "BaseMenu.h"
#include "MenuHelpers.h"
#include "OptionsMenu.h"
#include "SelectLevelMenu.h"
#include "AchMenu.h"
#include "Common.h"

/*
	This interface is used by MainMenu class to
	pass events to State. It's not the most
	elegant solution, I must admin, but it works
	for now.
*/
class IMainMenuCallback
{
public:
	virtual void StartGame() = 0;
	virtual void SelectLevel() = 0;
	virtual void ShowOptions() = 0;
	virtual void ShowScores() = 0;
	virtual void ShowAchievements() = 0;
	virtual void ExitGame() = 0;
};

/*
	This class represents the main menu
*/
class MainMenu : public GameMenu, public ButtonMenuEntry::ICallback
{
public:
	static const int EID_START			= 1;
	static const int EID_SELECT_LEVEL	= 2;
	static const int EID_OPTIONS		= 3;
	static const int EID_SCORES			= 4;
	static const int EID_ACHIEVEMENTS	= 5;
	static const int EID_EXIT			= 6;

private:
	VerticalButtonEntry m_startGame;
	VerticalButtonEntry m_continueGame;
	VerticalButtonEntry m_options;
	VerticalButtonEntry m_scores;
	VerticalButtonEntry m_achs;
	FooterAcceptButtonEntry m_exit;

	IMainMenuCallback *m_pCallback;

public:
	MainMenu( IMainMenuCallback *pCallback )
		: GameMenu( "", SCREEN_W/2 - 450/2, 120, 450, 460 )
		, m_pCallback( pCallback )
		, m_startGame		( this, 0, "Start Hardcore Game",	EID_START,			this )
		, m_continueGame	( this, 1, "Start Casual Game",		EID_SELECT_LEVEL,	this )
		, m_options			( this, 2, "Options",				EID_OPTIONS,		this )
		, m_scores			( this, 3, "High Scores",			EID_SCORES,			this )
		, m_achs			( this, 4, "Achievements",			EID_ACHIEVEMENTS,	this )
		, m_exit			( this, 5, "Exit",					EID_EXIT,			this )
	{
		SetSelected( &m_startGame );
	}

	void operator()( int id );
};

/*
	This class represents a screen where main menu, options menu etc. are shown
*/
class GameStateMenu : public BaseGameState, public IMainMenuCallback, public IOptionsMenuCallback, public ISelectLevelCallback, public IAchMenuCallback, public IControlTypeCallback
{
	  // Currently active (sub-)menu.
	BaseMenu *m_pMenu;

	  // Main menu and its related flags
	MainMenu m_mainMenu;
	bool m_startGame;
	bool m_selectLevel;
	bool m_showOptions;
	bool m_showScores;
	bool m_showAchs;
	bool m_exitGame;

	  // Level selection menu
	MenuSelectLevel m_selectLevelMenu;
	bool m_backFromSelectLevel;
	int m_startLevel;
	
	  // Options menu and its sub-menus
	MenuOptions m_options;
	VideoOptionsMenu m_vidOptions;
	SoundOptionsMenu m_sndOptions;
	ControlOptionsMenu_Keyboard m_ctrlOptionsKbd;
	ControlOptionsMenu_Gamepad m_ctrlOptionsPad;
	ControlOptionsMenu_Touch m_ctrlOptionsTouch;
	GameplayOptionsMenu m_gameOptions;
	bool m_backFromOptions;
	bool m_backFromSubOptions;
	EOptions m_showSubOptions;
	AchMenu m_achMenu;
	bool m_backFromAchs;

	  // Background image
	ImageHandle m_image;

	std::string m_creditsText;
	float m_creditsX;
	int m_creditsW;

public:
	GameStateMenu()
		: BaseGameState( GAMESTATE_MENU )
		, m_mainMenu( this )
		, m_startGame( false )
		, m_showOptions( false )
		, m_selectLevel( false )
		, m_showSubOptions( OPTIONS_NONE )
		, m_showScores( false )
		, m_showAchs( false )
		, m_exitGame( false )
		, m_selectLevelMenu( 300, 200, this )
		, m_options(     150, 200, this )
		, m_vidOptions(  150, 200, this )
		, m_sndOptions(  150, 200, this )
		, m_ctrlOptionsKbd( 150, 200, this, this )
		, m_ctrlOptionsPad( 150, 200, this, this )
		, m_ctrlOptionsTouch( 150, 200, this, this )
		, m_gameOptions( 100, 200, this )
		, m_achMenu( this )
		, m_backFromOptions( false )
		, m_backFromSubOptions( false )
		, m_backFromSelectLevel( false )
		, m_startLevel( -1 )
		, m_pMenu( &m_mainMenu )
		, m_backFromAchs( false )
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

	void StartGame()
	{
		m_startGame = true;
	}

	void SelectLevel()
	{
		m_selectLevel = true;
	}
	
	void ShowOptions()
	{
		m_showOptions = true;
	}

	void ShowScores()
	{
		m_showScores = true;
	}

	void ShowAchievements()
	{
		m_showAchs = true;
	}

	void ExitGame()
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

	void BackFromSelectLevel()
	{
		m_backFromSelectLevel = true;
	}

	void StartLevel( int levelIndex )
	{
		m_startLevel = levelIndex;
	}

	void BackFromAchievements()
	{
		m_backFromAchs = true;
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
