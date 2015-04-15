#include "stdafx.h"
#include "GameStateGameOverTime.h"
#include "Common.h"
#include "IRender.h"
#include "ObjectsStorage.h"

void GameStateGameOverTime::RenderBefore( IRender & render, const IGameContext & context )
{
	render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2, Color(255,0,0), 30, TEXT_ALIGN_CENTER, "You ran out of time" );
	render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2 + 30, Color(255,0,0), 30, TEXT_ALIGN_CENTER, "Game over" );
}

void GameStateGameOverTime::RenderAfter( IRender & render, const IGameContext & context )
{

}

void GameStateGameOverTime::Update( float dt, IGameContext & context )
{
	m_timer -= dt;
	if ( m_timer <= 0 )
	{
		context.RemoveState( GAMESTATE_GAME_OVER_TIME );
		context.PushState( GAMESTATE_SCORES );
	}
}

void GameStateGameOverTime::OnPush( IGameContext & context )
{
	m_timer = 5.0f;
	context.GetOjectsStorage().GetMusic().ReceiveEvent( "GAME_OVER" );
}
