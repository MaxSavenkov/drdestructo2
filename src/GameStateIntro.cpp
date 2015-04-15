#include "stdafx.h"
#include "GameStateIntro.h"
#include "Common.h"
#include "IRender.h"
#include "ISystem.h"
#include "ObjectsStorage.h"
#include "JoystickMapping.h"

const float FADE_TIME = 1.0f;

void GameStateIntro::RenderBefore( IRender & render, const IGameContext & context )
{
	if ( !m_image.IsValid() )
		m_image = GetRender().LoadImage( "Data/Sprites2/title_screen.png" );

	render.CopyRegion( m_image, 0, 0, 0, 0, -1, -1, SCREEN_W, SCREEN_H );
	int c = 100 + 155 * (cosf(3*m_flash) + 1.0f)/2.0f;
	if ( c > 255 )
		c = 255;
	render.DrawAlignedText( SCREEN_W/2, SCREEN_H - 40, Color(c,c,c), 20, TEXT_ALIGN_CENTER, "Press Enter" );

	if ( m_fade )
	{
		int alpha = 255.0f * (1.0f - m_timer / FADE_TIME);
		if ( alpha > 255 )
			alpha = 255;
		if ( alpha < 0 )
			alpha = 0;
		render.DrawRectFill( 0, 0, SCREEN_W, SCREEN_H, Color( 0, 0, 0, alpha ) );
	}
}

void GameStateIntro::RenderAfter( IRender & render, const IGameContext & context )
{

}

void GameStateIntro::Update( float dt, IGameContext & context )
{
	m_flash += dt;
	if ( m_flash >= ALLEGRO_PI )
		m_flash -= 2*ALLEGRO_PI;
	m_timer -= dt;
	if ( m_timer <= 0 )
	{
		if ( m_fade )
		{
			context.RemoveState( GAMESTATE_INTRO );
			context.PushState( GAMESTATE_MENU );
		}
	}
}

void GameStateIntro::OnPush( IGameContext & context )
{
	context.GetOjectsStorage().GetMusic().ReceiveEvent( "INTRO" );

	m_timer = 0.0f;
	m_fade = false;
	m_flash = 0.0f;

	const JoystickMapping & mapping = GetJoystickMapping(0);

	AddKeyboardHandler( this, &GameStateIntro::GoToMenu, KeyboardEvent::KBD_KEY_DOWN, ALLEGRO_KEY_ENTER );	
	AddTouchHandler( this, &GameStateIntro::GoToMenu, TouchEvent::TOUCH_BEGIN );
	AddGamepadHandlerForControl( JoystickMapping::CONTROL_A, this, &GameStateIntro::GoToMenu );
	AddGamepadHandlerForControl( JoystickMapping::CONTROL_START, this, &GameStateIntro::GoToMenuWithStart );
}

bool GameStateIntro::GoToMenu( const InputEvent & e )
{
	if ( !m_fade )
	{
		m_fade = true;
		m_timer = FADE_TIME;	
	}
	return true;
}

bool GameStateIntro::GoToMenuWithStart( const InputEvent & e )
{
	return GoToMenu(e);
}
