#include "stdafx.h"
#include "GameStatePause.h"
#include "ObjectsStorage.h"
#include "PlayerProfile.h"

void GameStatePause::RenderBefore( IRender & render, const IGameContext & context )
{
}

void GameStatePause::RenderAfter( IRender & render, const IGameContext & context )
{
	render.DrawRectFill( 0, 0, SCREEN_W, SCREEN_H, Color( 0, 0, 0 ) );
	m_pMenu->Render( &render );
}

void GameStatePause::Update( float dt, IGameContext & context )
{
	m_pMenu->Update( dt );

	if ( m_resumeGame )
	{
		context.RemoveState( GAMESTATE_PAUSE );
		m_resumeGame = false;
	}

	if ( m_exitGame )
	{
		context.RemoveState( GAMESTATE_LEVEL );
		context.PushState( GAMESTATE_MENU );
		m_exitGame = false;
	}

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

	if ( m_backFromOptions )
	{
		m_pMenu = &m_pauseMenu;
		m_pMenu->OnShow();
		m_backFromOptions = false;
	}
}

void PauseMenu::operator()( int id )
{
	switch( id )
	{
	case EID_EXIT:
		m_pCallback->ExitToMenu();
		return;
	case EID_OPTIONS:
		m_pCallback->ShowOptions();
		return;
	case EID_RESUME:
		m_pCallback->ResumeGame();
		return;
	}
}

void GameStatePause::OnPush( IGameContext & context )
{
}
