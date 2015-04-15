#include "stdafx.h"
#include "GameStateWinGame.h"
#include "Common.h"
#include "IRender.h"
#include "ObjectsStorage.h"

/*
	Animation sequence:

	Stage 1: "Space"

	Dr. Destructo fortress slowly falls from the top of the screen against
	background of scrolling starts. The sky is getting brighter as scene
	nears the end. There is a fade out to black at the end

	Stage 2: "Earth"

	Scene fades in from black to view of oceans with an island from level 5.
	Dr. Destructo's fortress falls out of the sky and hits the island. As
	it nears the island, there is a fade-out to white.

	Stage 3: "Text"

	Fade-in from white.
	A Dr. Destructo's icon appears. Underneath it there is ending text from
	the	original game.

	After player presses Enter, game moves onto High Scores
*/

void GameStateWinGame::RenderBefore( IRender & render, const IGameContext & context )
{
	switch( m_stage )
	{
		case STAGE_SPACE:
		{
			if ( !m_ship.IsValid() )
				m_ship = render.LoadImage( "Data/Sprites2/target21.png" );

			if ( !m_space.IsValid() )
			{
				m_space = render.CreateImage( SCREEN_W, SCREEN_H, false, "GameEnd Space" );
				render.ClearImage( m_space, Color( 0, 0, 0, 0 ) );
				render.LockImage( m_space );
				for ( int i = 0; i < 200; ++i )
				{
					const int x = rand() % SCREEN_W;
					const int y = rand() % SCREEN_H;
					render.DrawPixel( m_space, x, y, Color( 100, 150, 250 ) );
				}
				render.UnlockImage( m_space );
			}

			ColorPoint p1( 0, m_y, Color( 0, 0, 0 ) );
			ColorPoint p2( SCREEN_W, m_y, Color( 0, 0, 0 ) );
			ColorPoint p3( SCREEN_W, m_y + SCREEN_H*3, Color( 0, 100, 200 ) );
			ColorPoint p4( 0, m_y + SCREEN_H*3, Color( 0, 100, 200 ) );
			render.DrawColoredQuad( p1, p2, p3, p4 );
			
			
			TexPoint t1( 0, m_y, 0, 0 );
			TexPoint t2( SCREEN_W, m_y, SCREEN_W, 0 );
			TexPoint t3( SCREEN_W, m_y + SCREEN_H*3, SCREEN_W, SCREEN_H );
			TexPoint t4( 0, m_y + SCREEN_H*3, 0, SCREEN_H );
			render.DrawTexturedQuad( m_space, t1, t2, t3, t4 );

			render.DrawImage( m_ship, 200 + ((rand()%10)-5), m_yShip+ ((rand()%10)-5) );
			break;
		}
		case STAGE_EARTH:
		{
		if ( !m_ship.IsValid() )
			m_ship = render.LoadImage( "Data/Sprites2/target21.png" );

			if ( !m_island.IsValid() )
				m_island = render.LoadImage( "Data/Sprites2/target05.png" );

			{
				ColorPoint p1( 0, 0, Color( 100, 150, 200 ) );
				ColorPoint p2( SCREEN_W, 0, Color( 100, 150, 200 ) );
				ColorPoint p3( SCREEN_W, SCREEN_H*0.75f, Color( 100, 150, 255 ) );
				ColorPoint p4( 0, SCREEN_H*0.75f, Color( 100, 150, 255 ) );
				render.DrawColoredQuad( p1, p2, p3, p4 );			
			}
			{
				ColorPoint p1( 0, SCREEN_H*0.75f, Color( 0, 0, 255 ) );
				ColorPoint p2( SCREEN_W, SCREEN_H*0.75f, Color( 0, 0, 255 ) );
				ColorPoint p3( SCREEN_W, SCREEN_H, Color( 0, 0, 155 ) );
				ColorPoint p4( 0, SCREEN_H, Color( 0, 0, 155 ) );
				render.DrawColoredQuad( p1, p2, p3, p4 );
			}

			render.StretchImage( m_ship, 390 + ((rand()%4)-2), m_yShip+ ((rand()%4)-2), 0.25f );
			render.StretchImage( m_island, 300, 500, 0.5f );

			break;
		}
		case STAGE_TEXT:
		{
			if ( !m_question.IsValid() )
				m_question = render.LoadImage( "Data/Sprites2/question.png" );
			render.DrawImage( m_question, SCREEN_W/2 - 140/2, SCREEN_H/2 - 150 );
			render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2 - 20, Color(0,200,200), 30, TEXT_ALIGN_CENTER, "Today you win" );
			render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2 + 20, Color(200,0,200), 30, TEXT_ALIGN_CENTER, "Tommorow you shall kneel" );
			render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2 + 60, Color(200,200,200), 30, TEXT_ALIGN_CENTER, "before me!" );
			break;
		}
	}
}

void GameStateWinGame::RenderAfter( IRender & render, const IGameContext & context )
{
	if ( m_fadeIn || m_fadeOut )
	{
		int alpha = (m_fadeTimer / m_fadeMax) * 255;
		if ( alpha < 0 )
			alpha = 0;
		if ( alpha > 255 )
			alpha = 255;

		Color c = m_fadeColor;

		if ( m_fadeIn )
			c.a = alpha;
		else if ( m_fadeOut )
			c.a = 255 - alpha;

		int a, b, d;
		al_get_blender( &a, &b, &d );
		al_set_blender( ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA );
		render.DrawRectFill( 0, 0, SCREEN_W, SCREEN_H, c );
		al_set_blender( a, b, d );
	}
}

void GameStateWinGame::Update( float dt, IGameContext & context )
{
	m_timer -= dt;
	if ( m_timer <= 0 )
	{
		NextStage();
	}

	if ( (m_fadeIn || m_fadeOut) && m_fadeTimer > 0.0f )
		m_fadeTimer -= dt;

	  // Update stage
	switch( m_stage )
	{
		case STAGE_SPACE:
			m_y -= 100 * dt;
			m_yShip += 90 * dt;

			for ( int i = 0; i < MAX_EXPLOSIONS; ++i )
			{
				if ( !m_explosions[ i ].m_objID.IsValid() )
				{
					if ( m_explosionRequestID[ i ].IsValid() )
					{
						m_explosions[ i ].m_objID = context.GetCreateRequestResult( m_explosionRequestID[ i ] );
						m_explosions[ i ].m_lifeTimer = 0.5f;
						m_explosionRequestID[ i ].Invalidate();
					}
					else
					{
						if ( rand() % 100 < 5 )
						{
							SCreateObjectRequest::SData data;
							data.x = (rand() % 600) + 220;
							data.y = (rand() % 150) + m_yShip + 100;
							m_explosionRequestID[ i ] = context.CreateGameObject( "enemy_prop_dead", data );
						}
					}
				}
				else
				{
					GameObject *pObj = context.GetOjectsStorage().GetGameObject( m_explosions[ i ].m_objID );
					if ( pObj )
					{
						m_explosions[ i ].m_lifeTimer -= dt;

						GraphicComponent *pGraph = context.GetOjectsStorage().GetGraphicComponent( pObj->GetGraphicComponent() );
						if ( pGraph )
						{
							pGraph->m_currentAnim = 0;

							pGraph->m_y -= 100*dt;
						}

						if ( m_explosions[ i ].m_lifeTimer <= 0.0f )
						{
							context.DestroyGameObject( m_explosions[ i ].m_objID );
							m_explosions[ i ].m_objID = GameObjectID();
						}
					}
				}

				if ( !m_fadeOut )
				{
					if ( m_yShip > SCREEN_H )
					{
						m_fadeOut = true;
						m_fadeTimer = m_timer;
						m_fadeMax = m_timer;
						m_fadeColor = Color(0,0,0);
					}
				}
			}

			break;
		case STAGE_EARTH:
			m_yShip += 90 * dt;
			if ( m_yShip > 350 && !m_fadeOut )
			{
				m_fadeOut = true;
				m_fadeTimer = 1.0f;
				m_fadeMax = 1.0f;
				m_fadeColor = Color(255,255,255,0);
			}
			if ( m_fadeIn && m_fadeTimer <= 0.0f )
				m_fadeIn = false;
			if ( m_fadeOut && m_fadeTimer <= 0.0f )
				m_timer = 0.0f;
			break;
		case STAGE_LAST:
			context.DestroyAllObjects();
			context.RemoveState( GAMESTATE_WIN );
			context.PushState( GAMESTATE_SCORES );
			break;
	}
}

void GameStateWinGame::NextStage()
{
	  // Free resources used by old stage
	switch( m_stage )
	{
		case STAGE_INVALID:
			m_stage = STAGE_SPACE;
			break;
		case STAGE_SPACE:
			GetRender().DestroyImage( m_space );
			m_space = ImageHandle();
			m_stage = STAGE_EARTH;
			m_fadeOut = false;
			break;
		case STAGE_EARTH:
			GetRender().DestroyImage( m_ship );
			GetRender().DestroyImage( m_island );
			m_ship = ImageHandle();
			m_island = ImageHandle();
			m_fadeOut = false;
			m_stage = STAGE_TEXT;
			break;
		case STAGE_TEXT:
			m_stage = STAGE_LAST;
			break;
	}	

	  // Set up new stage
	switch( m_stage )
	{
		case STAGE_SPACE:
			m_y = 0;
			m_yShip = -300;
			m_timer = 15.0f;
			break;
		case STAGE_EARTH:
			m_yShip = -200;
			m_timer = 10.0f;
			m_fadeIn = true;
			m_fadeTimer = 2.0f;
			m_fadeMax = 2.0f;
			m_fadeColor = Color(0,0,0,0);
			break;
		case STAGE_TEXT:
			m_timer = 10.0f;
			m_fadeIn = true;
			m_fadeTimer = 2.0f;
			m_fadeMax = 2.0f;
			m_fadeColor = Color(255,255,255,255);
			break;
		case STAGE_LAST:
			GetRender().DestroyImage( m_island );
			GetRender().DestroyImage( m_ship );
			GetRender().DestroyImage( m_question );
			GetRender().DestroyImage( m_space );
			m_island = ImageHandle();
			m_ship = ImageHandle();
			m_question = ImageHandle();
			m_space = ImageHandle();
		default:
			;
	}
}

bool GameStateWinGame::Skip( const InputEvent & e )
{
	m_stage = STAGE_LAST;
	return true;
}

void GameStateWinGame::OnPush( IGameContext & context )
{
	context.GetOjectsStorage().GetMusic().ReceiveEvent( "WIN_GAME" );
	m_stage = STAGE_INVALID;
	NextStage();
}
