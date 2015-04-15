#include "stdafx.h"
#include "GameStateNextLevel.h"
#include "Common.h"
#include "IRender.h"
#include "ISystem.h"
#include "TextManager.h"
#include "LevelDirector.h"
#include "ObjectsStorage.h"
#include "PlayerProfile.h"

const float TIME_ACTOR_APPEAR	= 0.6f;
const float TIME_DRAW_CHAR		= 0.01f;
const float TIME_PAUSE			= 0.5f;
const float TIME_FADE			= 1.0f;

void GameStateNextLevel::RenderBefore( IRender & render, const IGameContext & context )
{
	m_border.Render( render );
	render.DrawImage( m_shaft, 255, 0 );

	if ( m_state < STATE_FINISH )
	{
		render.DrawAlignedText( SCREEN_W/2, SCREEN_H - 40, Color(200,200,200), 20, TEXT_ALIGN_CENTER, "Press Fire or Enter to skip to the next phrase" );
		render.DrawAlignedText( SCREEN_W/2, SCREEN_H - 20, Color(200,200,200), 20, TEXT_ALIGN_CENTER, "Press Escape to skip briefing" );
	}
	else
	{
		int c = 100 + 155 * (cosf(3*m_flash) + 1.0f)/2.0f;
		if ( c > 255 )
			c = 255;
		render.DrawAlignedText( SCREEN_W/2, SCREEN_H - 40, Color(c,c,c), 20, TEXT_ALIGN_CENTER, "Press Fire or Enter to continue" );
	}

	for ( int i = 0; i < (int)m_actors.size(); ++i )
		render.DrawImage( m_actors[i].m_image, 305, m_actors[i].m_y );

	if ( m_state == STATE_FADE )
	{
		int c = 255.0f * (1.0f - m_timer / TIME_FADE);
		if ( c < 0 )
			c = 0;
		if ( c > 255 )
			c = 255;
		render.DrawRectFill( 0, 0, SCREEN_W, SCREEN_H, Color( 0, 0, 0, c ) );
	}
}

void GameStateNextLevel::RenderAfter( IRender & render, const IGameContext & context )
{

}

void GameStateNextLevel::Update( float dt, IGameContext & context )
{
	m_flash += dt;
	if ( m_flash >= ALLEGRO_PI )
		m_flash -= 2*ALLEGRO_PI;

	m_timer -= dt;

	if ( m_skipPhrase && m_state != STATE_FADE )
		m_timer = 0;
		

	switch( m_state )
	{
		case STATE_INIT:
		{
			context.LoadNextLevel();
			context.GetPlayerProfile().ResetLevelStats();
			
			m_state = STATE_PAUSE;
		}
		break;
		
		case STATE_ACTOR_INIT:
			{
			if ( m_timer > 0.0f )
				break;
			
			if ( m_nextActor.IsValid() )
			{
				Actor actor;
				actor.m_image = GetRender().CreateImage( 1024, 128, false, "brief" );
				GetRender().ClearImage( actor.m_image, Color(0,0,0,0) );
				actor.m_y = -128;
				m_actors.push_back( actor );
			
				GetRender().DrawImage( actor.m_image, m_nextActor, 0, 0 );
			
				m_nextActor = ImageHandle();
				m_state = STATE_ACTOR_APPEAR;
				m_timer = TIME_ACTOR_APPEAR;
			}			
		}
		break;

		case STATE_ACTOR_APPEAR:
		{
			float t = 1.0f - m_timer / TIME_ACTOR_APPEAR;
			if ( t < 0.0f ) t = 0.0f;

			if ( !m_actors.empty() )
			{
				const int dy = m_areaY + 128;
				m_actors.back().m_y = -128 + sinf( ALLEGRO_PI / 2.0f * t ) * dy;
			}

			if ( m_timer <= 0.0f )
			{
				m_state = STATE_DRAW_TEXT;
				m_timer = TIME_DRAW_CHAR;
			}
		}
		break;

		case STATE_DRAW_TEXT:
		{
			if ( m_timer <= 0.0f )
			{
				if ( m_skipPhrase )
				{
					while( m_textPrinter.Update( m_actors.back().m_image, GetRender() ) );
					m_state = STATE_PAUSE;
					m_timer = TIME_PAUSE;
					if ( !m_skipAll )
						m_skipPhrase = false;
				}
				else
				{
					if ( !m_textPrinter.Update( m_actors.back().m_image, GetRender() ) )
					{
						m_state = STATE_PAUSE;
						m_timer = TIME_PAUSE;
					}
					else
					{
						m_state = STATE_DRAW_TEXT;
						m_timer = TIME_DRAW_CHAR;
					}
				}
			}			
		}
		break;

		case STATE_PAUSE:
		{
			if ( m_timer <= 0.0f )
			{
				const STalkEntry *pTalk = context.GetLevelDirector().GetBriefing( m_briefingIndex );

				if ( pTalk )
				{
					m_nextActor = pTalk->m_image;
					const Text *txt = GetTextManager().GetResource( pTalk->m_text );
					if ( txt )
						m_textPrinter.SetText( txt->m_text );
			
					if ( !m_skipAll )
						m_skipPhrase = false;

					if ( m_briefingIndex > 0 )
					{
						if ( m_textY > m_areaY + 120 )
							m_areaY = m_textY;
						else
							m_areaY = m_areaY + 120;
					}

					m_textPrinter.SetRegion( 128 + 20, 10, 512, 128 );

					++m_briefingIndex;

					m_state = STATE_ACTOR_INIT;
					m_timer = 0.0f;
				}
				else
				{
					m_state = STATE_FINISH;
					m_timer = 0.0f;
					m_nextActor = ImageHandle();
					m_textPrinter.SetText( UTFString() );
				}
			}
		}
		break;

		case STATE_FADE:
		{
			if ( m_timer <= 0.0f )
			{
				context.RemoveState( GAMESTATE_NEXT_LEVEL );
				context.PushState( GAMESTATE_LEVEL );
			}
		}
		break;
	}
}

void GameStateNextLevel::OnPush( IGameContext & context )
{
	context.GetOjectsStorage().GetMusic().ReceiveEvent( "BRIEFING" );

	if ( !m_slot.IsValid() )
	{
		m_border.Init( GetRender() );
		//m_bg		= GetRender().LoadImage( "Data/Sprites2/menu_screen.png" );
		m_slot		= GetRender().LoadImage( "Data/Sprites2/ui/brdr_3.png" );
		m_shaft		= GetRender().LoadImage( "Data/Sprites2/ui/shift_elem_1.png" );
	}

	m_state = STATE_INIT;

	m_timer = 0.0f;
	m_briefingIndex = 0;

	m_next = true;
	m_skipPhrase = false;
	m_skipAll = false;

	m_areaY = 50;
	m_textY = 0;
	m_textX = 0;

	m_flash = 0.0f;

	m_textPrinter.SetTextParams( 24 );

	AddKeyboardHandler( this, &GameStateNextLevel::SkipAll, KeyboardEvent::KBD_KEY_DOWN, ALLEGRO_KEY_ESCAPE );
	AddKeyboardHandler( this, &GameStateNextLevel::SkipPhraseAlt, KeyboardEvent::KBD_KEY_DOWN, ALLEGRO_KEY_ENTER );

	const JoystickMapping & mapping = GetJoystickMapping(0);
	
	AddGamepadHandlerFromConfig( "Pad_Fire", this, &GameStateNextLevel::SkipPhrase );
	AddGamepadHandlerFromConfig( "Pad_Bomb", this, &GameStateNextLevel::SkipAll );
	
	const int keyFire		= GetSystem().GetConfigValue_Int( "DD_Controls", "Kbd_Fire" );	
	AddKeyboardHandler( this, &GameStateNextLevel::SkipPhrase,  KeyboardEvent::KBD_KEY_DOWN, keyFire    ? keyFire    : ALLEGRO_KEY_Q );
	AddTouchHandler( this, &GameStateNextLevel::SkipPhrase,  TouchEvent::TOUCH_BEGIN );
}

void GameStateNextLevel::OnRemove( IGameContext & context )
{
	for ( int i = 0; i < (int)m_actors.size(); ++i )
		GetRender().DestroyImage( m_actors[i].m_image );

	m_actors.clear();
}

bool GameStateNextLevel::SkipPhrase( const InputEvent & e)
{
	if ( m_state == STATE_FINISH )
	{
		m_state = STATE_FADE;
		m_timer = TIME_FADE;
	}
	else
	{
		m_skipPhrase = true;
	}

	return true;
}

bool GameStateNextLevel::SkipPhraseAlt( const InputEvent & e)
{
	return SkipPhrase( e );
}

bool GameStateNextLevel::SkipAll( const InputEvent & e)
{
	if ( m_state == STATE_FINISH )
	{
		m_state = STATE_FADE;
		m_timer = TIME_FADE;
	}
	else
	{
		m_skipAll = true;
		m_skipPhrase = true;
	}

	return true;
}
