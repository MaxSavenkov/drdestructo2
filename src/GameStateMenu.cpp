#include "stdafx.h"
#include "GameStateMenu.h"
#include "ObjectsStorage.h"
#include "PlayerProfile.h"

void GameStateMenu::RenderBefore( IRender & render, const IGameContext & context )
{
}

void GameStateMenu::RenderAfter( IRender & render, const IGameContext & context )
{
	m_pMenu->Render( &render );

	  // Draw scrolling credits
	render.DrawText( m_creditsX, SCREEN_H - 40, Color( 155,155,155 ), 35, m_creditsText.c_str() );
}

void GameStateMenu::Update( float dt, IGameContext & context )
{
	m_pMenu->Update( dt );

	m_creditsX -= dt * 100;
	if ( m_creditsX < -m_creditsW - 100 )
		m_creditsX = SCREEN_W + 100;

	if ( m_startGame )
	{
		context.GetPlayerProfile().m_hardcore = true;
		context.RestartGame();
		context.RemoveState( GAMESTATE_MENU );
		context.PushState( GAMESTATE_NEXT_LEVEL );
		m_startGame = false;
	}

	if ( m_exitGame )
	{
		context.RemoveState( GAMESTATE_MENU );
		m_exitGame = false;
	}

	if ( m_selectLevel )
	{
		m_selectLevelMenu.ReInit( context );
		m_pMenu = &m_selectLevelMenu;
		m_pMenu->OnShow();
		m_selectLevel = false;
	}

	if ( m_pMenu == &m_selectLevelMenu )
		m_selectLevelMenu.UpdateLevel( context, dt );

	if ( m_showOptions )
	{
		m_pMenu = &m_options;
		m_pMenu->OnShow();
		m_showOptions = false;
	}

	if ( m_showSubOptions )
	{
		switch( m_showSubOptions )
		{
			case OPTIONS_VIDEO: m_pMenu = &m_vidOptions; break;
			case OPTIONS_SOUND: m_pMenu = &m_sndOptions; break;
			case OPTIONS_CONTROLS_KEYBOARD: m_pMenu = &m_ctrlOptionsKbd; break;
			case OPTIONS_CONTROLS_GAMEPAD: m_pMenu = &m_ctrlOptionsPad; break;
			case OPTIONS_CONTROLS_TOUCH: m_pMenu = &m_ctrlOptionsTouch; break;
			case OPTIONS_GAME: m_pMenu = &m_gameOptions; break;
		}

		m_pMenu->OnShow();
		
		m_showSubOptions = OPTIONS_NONE;
	}

	if ( m_backFromSubOptions )
	{
		m_pMenu = &m_options;
		m_pMenu->OnShow();
		m_backFromSubOptions = false;
	}

	if ( m_showScores )
	{
		context.RemoveState( GAMESTATE_MENU );
		context.PushState( GAMESTATE_SCORES );
		m_showScores = false;
	}

	if ( m_showAchs )
	{
		m_pMenu = &m_achMenu;
		m_pMenu->OnShow();
		m_achMenu.Init( context );
		m_showAchs = false;
	}

	if ( m_backFromOptions )
	{
		m_pMenu = &m_mainMenu;
		m_pMenu->OnShow();
		m_backFromOptions = false;
	}

	if ( m_backFromSelectLevel )
	{
		m_pMenu = &m_mainMenu;
		m_pMenu->OnShow();
		m_backFromSelectLevel = false;
	}

	if ( m_startLevel >= 0 )
	{
		m_pMenu = &m_mainMenu;
		m_pMenu->OnShow();
		context.GetPlayerProfile().m_hardcore = false;
		context.RestartGame( m_startLevel );
		context.RemoveState( GAMESTATE_MENU );
		context.PushState( GAMESTATE_NEXT_LEVEL );
		m_startLevel = -1;
	}

	if ( m_backFromAchs )
	{
		m_pMenu = &m_mainMenu;
		m_pMenu->OnShow();
		m_backFromAchs = false;
	}
}

void MainMenu::operator()( int id )
{
	switch( id )
	{
	case EID_EXIT:
		m_pCallback->ExitGame();
		return;
	case EID_OPTIONS:
		m_pCallback->ShowOptions();
		return;
	case EID_START:
		m_pCallback->StartGame();
		return;
	case EID_SELECT_LEVEL:
		m_pCallback->SelectLevel();
		return;
	case EID_SCORES:
		m_pCallback->ShowScores();
		return;
	case EID_ACHIEVEMENTS:
		m_pCallback->ShowAchievements();
		return;
	}
}

void GameStateMenu::OnPush( IGameContext & context )
{
	m_creditsText = "** Return of Dr. Destructo ** Programming: Max Savenkov    Art: Oleg Pavlov, Max Savenkov    Sound & Music: Max Savenkov"
		"        Original game ** Island of Dr. Destructo ** Programming: Eugene Messina    Art: David Lincoln-Howells        Thanks: Allegro developers & community "
		" (http://allegro.cc), all my friends and my co-workers at GloboGames.        I hope you enjoy this remake. It took more than 3 years to complete, which is too much time "
	    "for such a simple game, but, well, such is the fate of pet-projects :)        Go to http://zxstudio.org for updates & news on ** Return of Dr. Destructo ** and my future projects. "
		"See ya!                                             ";
	m_creditsX = SCREEN_W + 100;
	m_creditsW = GetRender().GetStringWidth( m_creditsText.c_str(), 35 );

	context.GetOjectsStorage().GetPlayer().m_score = 0;
	context.GetOjectsStorage().GetMusic().ReceiveEvent( "MENU" );
	
	//m_image = GetRender().LoadImage( "Data/Sprites2/title_screen.png" );
}
