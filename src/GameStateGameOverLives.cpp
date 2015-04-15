#include "stdafx.h"
#include "GameStateGameOverLives.h"
#include "Common.h"
#include "IRender.h"
#include "ObjectsStorage.h"

void GameStateGameOverLives::RenderBefore( IRender & render, const IGameContext & context )
{
	render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2, Color(255,0,0), 30, TEXT_ALIGN_CENTER, "Loss of lives too severe" );
	render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2 + 30, Color(255,0,0), 30, TEXT_ALIGN_CENTER, "Game over" );
}

void GameStateGameOverLives::RenderAfter( IRender & render, const IGameContext & context )
{

}

void GameStateGameOverLives::Update( float dt, IGameContext & context )
{
	m_timer -= dt;
	if ( m_timer <= 0 )
	{
		context.RemoveState( GAMESTATE_GAME_OVER_LIVES );
		context.PushState( GAMESTATE_SCORES );
	}
}

void GameStateGameOverLives::OnPush( IGameContext & context )
{
	m_timer = 5.0f;
	context.GetOjectsStorage().GetMusic().ReceiveEvent( "GAME_OVER" );
}
