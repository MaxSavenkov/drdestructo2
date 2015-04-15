#include "stdafx.h"
#include "GameStateWin.h"
#include "IRender.h"
#include "Vars.h"
#include "ObjectsStorage.h"
#include "PlayerProfile.h"
#include "Achievements.h"
#include "Common.h"

namespace
{
UseIntVar GM_PLAYER("GM_PLAYER");
UseIntVar GM_ENEMY("GM_ENEMY");
UseIntVar GM_ENEMY_DOUBLE("GM_ENEMY_DOUBLE");
UseIntVar GM_BULLET("GM_BULLET");
UseIntVar GM_TARGET("GM_TARGET");
UseIntVar GM_SHOT_DOWN("GM_SHOT_DOWN");
}

const float FADE_TIME = 1.0f;
const float WAIT_TIME = 1.0f;

  // Dimensions of statistics image
const int RES_W = SCREEN_W / 4 + 80;
const int RES_H = SCREEN_H / 3;

  // Dimensions of achievement plaque
const int ACH_W = SCREEN_W / 4;
const int ACH_H = 59;

void GameStateWin::RenderBefore( IRender & render, const IGameContext & context )
{
	if ( m_stage == STAGE_CLEAR )
		render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2 - 32, Color(255,255,255), 64, TEXT_ALIGN_CENTER, "Level complete" );
}

void GameStateWin::RenderAfter( IRender & render, const IGameContext & context )
{
	if ( m_stage == STAGE_RESULTS )
	{
		if ( !m_results.IsValid() )
			m_results = render.CreateImage( RES_W, RES_H, false, "Level results" );

		if ( m_generateResults )
		{
			m_generateResults = false;

			const PlayerProfile & prof = context.GetPlayerProfile();
			const int hitsPerc = prof.m_levelShotsFired > 0 ? prof.m_levelHits * 100 / prof.m_levelShotsFired : 0;
			const int totalDowned = prof.m_levelTargetHits + prof.m_levelTargetMisses;
			const int targetHitsPerc = totalDowned > 0 ? prof.m_levelTargetHits * 100 / totalDowned : 0;
			
			render.ClearImage( m_results, Color( 0, 0, 0, 128 ) );
			render.DrawAlignedText( m_results, RES_W / 2, 3, Color(255,255,255), 35, TEXT_ALIGN_CENTER, "Level Statistic:" );
			render.DrawLine( m_results, 10, 40, RES_W - 10, 40, Color(255,255,255) );
			
			render.DrawText( m_results, 10, 45 + 32 * 0, Color(255,255,255), 30, "Shots fired" );
			render.DrawText( m_results, 10 + 166, 45 + 32 * 0, Color(255,255,255), 30, ": %i", prof.m_levelShotsFired );
			
			render.DrawText( m_results, 10, 45 + 32 * 1, Color(255,255,255), 30, "Hits");
			render.DrawText( m_results, 10 + 166, 45 + 32 * 1, Color(255,255,255), 30, ": %i (%i%%)", prof.m_levelHits, hitsPerc );
			
			render.DrawText( m_results, 10, 45 + 32 * 2, Color(255,255,255), 30, "Enemies kiiled" );
			render.DrawText( m_results, 10 + 166, 45 + 32 * 2, Color(255,255,255), 30, ": %i", totalDowned );
			
			render.DrawText( m_results, 10, 45 + 32 * 3, Color(255,255,255), 30, "Hits on target" );
			render.DrawText( m_results, 10 + 166, 45 + 32 * 3, Color(255,255,255), 30, ": %i (%i%%)", prof.m_levelTargetHits, targetHitsPerc );
			
			render.DrawText( m_results, 10, 45 + 32 * 4, Color(255,255,255), 30, "Level time" );
			render.DrawText( m_results, 10 + 166, 45 + 32 * 4, Color(255,255,255), 30, ": %.1f sec.", prof.m_levelTime );
			
			render.DrawText( m_results, 10, 45 + 32 * 5, Color(255,255,255), 30, "Kills per sec." );
			render.DrawText( m_results, 10 + 166, 45 + 32 * 5, Color(255,255,255), 30, ": %.2f", prof.m_levelHits / prof.m_levelTime );
			
			render.DrawRect( m_results, 1, 1, RES_W - 1, RES_H - 1, Color( 30, 30, 30 ) );
			render.DrawRect( m_results, 2, 2, RES_W - 2, RES_H - 2, Color( 100, 100, 100 ) );
		}

		render.DrawImage( m_results, SCREEN_W / 2 - RES_W / 2, m_resultsY );
	}
	else if ( m_stage == STAGE_ACHIEVEMENTS || m_stage == STAGE_FADE )
	{
		render.DrawImage( m_results, SCREEN_W / 2 - RES_W / 2, m_resultsY );

		if ( m_achievements.size() > 0 )
		{
			render.DrawAlignedText( SCREEN_W/2+2, 460+2, Color(0,0,0), 30, TEXT_ALIGN_CENTER, "New achievements! (%i)", m_achievements.size() );
			render.DrawAlignedText( SCREEN_W/2, 460, Color(255,255,255), 30, TEXT_ALIGN_CENTER, "New achievements! (%i)", m_achievements.size() );
		}
		else
		{
			render.DrawAlignedText( SCREEN_W/2+2, 460+2, Color(0,0,0), 30, TEXT_ALIGN_CENTER, "No new achievements" );
			render.DrawAlignedText( SCREEN_W/2, 460, Color(255,255,255), 30, TEXT_ALIGN_CENTER, "No new achievements" );
		}

		for ( int i = 0; i < (int)m_achievements.size(); ++i )
		{
			const int gx = i % 3;
			const int gy = i / 3;
			const int x = 100 + gx * (SCREEN_W-200) / 3;
			const int y = 490 + gy * ( ACH_H + 10 );

			if ( gy > 2 )
			{
				render.DrawAlignedText( SCREEN_W/2+2, 490 + 3 * ( ACH_H + 10 )+2, Color(0,0,0), 20, TEXT_ALIGN_CENTER, "Too many achievements to display! Check out list in the main menu" );
				render.DrawAlignedText( SCREEN_W/2, 490 + 3 * ( ACH_H + 10 ), Color(255,255,255), 20, TEXT_ALIGN_CENTER, "Too many achievements to display! Check out list in the main menu" );				
				break;
			}

			SAchDisp & ach = m_achievements[i];

			if ( i == m_currentAch )
			{
				ach.m_alpha += 255 * 1/60.0f;
				if ( ach.m_alpha > 255 )
				{
					ach.m_alpha = 255;
					++m_currentAch;
				}
			}
			int a,b,c;
			al_get_blender( &a, &b, &c );
			al_set_blender( ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA );
			render.DrawTintedImage( ach.m_image, Color(255,255,255,ach.m_alpha), x, y );
			al_set_blender( a, b, c );
		}

		render.DrawAlignedText( SCREEN_W/2+2, SCREEN_H - 50+2, Color(0,0,0), 20, TEXT_ALIGN_CENTER, "Press fire to continue" );
		render.DrawAlignedText( SCREEN_W/2, SCREEN_H - 50, Color(255,255,255), 20, TEXT_ALIGN_CENTER, "Press fire to continue" );		
	}

	if ( STAGE_FADE )
	{
		int c = 255.0f * (1.0f - m_fadeTimer / FADE_TIME);
		if ( c < 0 )
			c = 0;
		if ( c > 255 )
			c = 255;
		render.DrawRectFill( 0, 0, SCREEN_W, SCREEN_H, Color( 0, 0, 0, c ) );		
	}
}

void GameStateWin::Update( float dt, IGameContext & context )
{
	if ( m_waitTimer > 0 )
	{
		m_waitTimer -= dt;
		return;
	}

	if ( m_stage == STAGE_FADE && m_fadeTimer > 0 )
	{
		m_fadeTimer -= dt;
	}

	if ( m_skipTimer > 0 )
	{
		m_skipTimer -= dt;
	}

	if ( m_fadeTimer < 0 )
	{
		context.RemoveState( GAMESTATE_LEVEL );
		if ( context.IsLastLevel() )
			context.PushState( GAMESTATE_WIN );
		else
		{
			context.UpdateUnlockedLevel();
			++context.GetOjectsStorage().GetPlayer().m_lives;
			context.PushState( GAMESTATE_NEXT_LEVEL );
		}
	}

	if ( m_stage == STAGE_CLEAR )
	{
		  // At this stagw, we accelerate all game objects to the edges of the screen and destroy them when they are invisible.
		  // After all objects but target are removed, sink target
		ObjectsStorage::GameObjectContainer & objs = context.GetOjectsStorage().GetGameObjects();
		if ( objs.GetActiveCount() <= 0 )
			m_stage = STAGE_RESULTS;

		GameObjectID targetObjectID;
		bool hasOtherObjects = false;

		for ( ObjectsStorage::GameObjectContainer::ConstIterator iter = objs.GetConstIterator(); !iter.AtEnd(); ++iter )
		{
			const GameObject *pObj = *iter;
			const MechanicComponent *pMech = context.GetOjectsStorage().GetMechanicComponent( pObj->GetMechanicComponent() );
			if ( !pMech )
				context.DestroyGameObject( pObj->GetID() );
			else
			{
				if ( pMech->m_type == GM_BULLET )
				{
					context.DestroyGameObject( pObj->GetID() );
					continue;
				}
				else if ( pMech->m_type == GM_TARGET )
				{
					targetObjectID = pObj->GetID();
					continue;
				}

				hasOtherObjects = true;

				PhysicComponent *pPhys = context.GetOjectsStorage().GetPhysicComponent( pObj->GetPhysicComponent() );
				GraphicComponent *pGraph = context.GetOjectsStorage().GetGraphicComponent( pObj->GetGraphicComponent() );
				if ( pPhys && pGraph )
				{
					pPhys->m_x += pPhys->m_speedX * dt;
					pGraph->m_x = pPhys->m_x;
					const int sign = (pObj->GetID().GetIndex() % 2) == 0 ? 1 : -1;				
					pPhys->m_speedX += sign * pPhys->m_accel * dt;

					if ( pPhys->m_x < -49 || pPhys->m_x >= SCREEN_W+49 || pPhys->m_y >= SCREEN_H + 100 )
						context.DestroyGameObject( pObj->GetID() );
				}
			}
		}

		if ( !hasOtherObjects )
		{
			GameObject *pObj = context.GetOjectsStorage().GetGameObject( targetObjectID );
			if ( pObj )
			{
				PhysicComponent *pPhys = context.GetOjectsStorage().GetPhysicComponent( pObj->GetPhysicComponent() );
				GraphicComponent *pGraph = context.GetOjectsStorage().GetGraphicComponent( pObj->GetGraphicComponent() );

				if ( pPhys && pGraph )
				{
					pPhys->m_y += 100 * dt;
					pGraph->m_y = pPhys->m_y;

					if ( pPhys->m_x < -49 || pPhys->m_x >= SCREEN_W+49 || pPhys->m_y >= SCREEN_H + 100 )
						context.DestroyGameObject( pObj->GetID() );
				}
			}
		}
	}
	else if ( m_stage == STAGE_RESULTS )
	{
		  // Move results image, if it's in place - move to the next stage
		if ( m_resultsY > 200 )
		{
			m_resultsY += m_resultsSpeed*dt;
			m_resultsSpeed -= 500*dt;
		}
		else
		{
			m_resultsY = 200;
			m_stage = STAGE_ACHIEVEMENTS;
		}
	}
}

void GameStateWin::OnPush( IGameContext & context ) 
{
	m_generateResults = true;
	m_skipTimer = 0;

	const int keyFire		= GetSystem().GetConfigValue_Int( "DD_Controls", "Kbd_Fire" );	
	AddKeyboardHandler( this, &GameStateWin::Skip,  KeyboardEvent::KBD_KEY_DOWN, keyFire    ? keyFire    : ALLEGRO_KEY_Q );	
	AddTouchHandler( this, &GameStateWin::Skip,  TouchEvent::TOUCH_END );
	AddGamepadHandlerFromConfig( "Pad_Fire", this, &GameStateWin::Skip );

	context.GetOjectsStorage().GetMusic().ReceiveEvent( "WIN_LEVEL" );
	m_speedY = 0;
	m_waitTimer = WAIT_TIME;
	m_fadeTimer = FADE_TIME;
	m_stage = STAGE_CLEAR;
	m_resultsY = SCREEN_H;
	m_resultsSpeed = -100;

	ObjectsStorage::GameObjectContainer & objs = context.GetOjectsStorage().GetGameObjects();
	for ( ObjectsStorage::GameObjectContainer::ConstIterator iter = objs.GetConstIterator(); !iter.AtEnd(); ++iter )
	{
		const GameObject *pObj = *iter;
		PhysicComponent *pPhys = context.GetOjectsStorage().GetPhysicComponent( pObj->GetPhysicComponent() );
		if ( pPhys )
		{
			pPhys->m_speedX = 0;
			pPhys->m_accel = RndInt( 20, 50 ) * 10;
		}
	}

	CheckAchievements( context.GetPlayerProfile(), context );
}

void GameStateWin::CheckAchievements( PlayerProfile & prof, const IGameContext & context )
{
      // Iterate over all achievements and check if we earned new ones
	m_achievements.clear();

	const Achievements & achs = GetAchievements();
	for ( int i = 0; i < achs.GetAchievementCount(); ++i )
	{
		const SAchievement *pAch = achs.GetAchievement( i );
		if ( !pAch )
			continue;

		if ( prof.m_achievements.find( pAch->m_guid ) != prof.m_achievements.end() )
			continue;

		if ( context.IsDebugAchMode() || pAch->Check( prof, context ) )
		{
			IRender & render = GetRender();
			prof.m_achievements.insert( std::string(pAch->m_guid) );
			m_achievements.push_back( SAchDisp() );
			SAchDisp & newAch = m_achievements.back();
			newAch.m_image = render.CreateImage( ACH_W, ACH_H, false, "Achievement" );
			newAch.m_alpha = 0;
			render.ClearImage( newAch.m_image, Color( 0, 0, 0, 200 ) );

			ImageHandle icon = render.LoadImage( (std::string( "Data/Sprites2/" ) + pAch->m_icon).c_str() );
			if ( icon.IsValid() )
				render.CopyRegion( newAch.m_image, icon, 5, 4, 0, 0, -1, -1, 50, 50 );
				//render.DrawImage( newAch.m_image, icon, 5, 5 );

			render.DrawRect( newAch.m_image, 2, 2, ACH_W - 2, ACH_H - 2, Color( 100, 100, 100 ) );
			render.DrawText( newAch.m_image, 60, 5, Color( 255, 255, 255 ), 20, "%s", pAch->m_name );
			render.DrawTextInArea( newAch.m_image, 60 + 1, 22 + 1, ACH_W - 6 - 60, ACH_H - 20, Color(10,10,10), 14, "%s", pAch->m_desc );
			render.DrawTextInArea( newAch.m_image, 60, 22, ACH_W - 6 - 60, ACH_H - 20, Color(200,200,200), 14, "%s", pAch->m_desc );
		}
	}

	m_currentAch = 0;
	prof.Save();
}

  // Called when player presses Escape
bool GameStateWin::Skip( const InputEvent & e)
{
	  // To prevent player from accidently skipping everything
	if ( m_skipTimer > 0 )
		return true;

	switch( m_stage )
	{
		case STAGE_CLEAR:
		{
			if ( m_waitTimer > 0 )
				m_waitTimer = 0;

			break;
		}
		case STAGE_RESULTS:
		{
			m_resultsY = 200;
			break;		
		}
		case STAGE_ACHIEVEMENTS:
		{
			if ( m_currentAch < (int)m_achievements.size() )
			{
				for ( int i = 0; i < (int)m_achievements.size(); ++i )
				{
					m_achievements[ i ].m_alpha = 255;
				}
				m_currentAch = (int)m_achievements.size();
			}
			else
			{
				m_stage = STAGE_FADE;
				m_fadeTimer = FADE_TIME;
			}
			break;
		}

		m_skipTimer = 1;
	}
	return true;
}
