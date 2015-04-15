#include "stdafx.h"
#include "GameStateLevel.h"
#include "ControlComponent.h"
#include "ObjectsStorage.h"
#include "LevelDirector.h"
#include "PlayerProfile.h"
#include "Vars.h"
#include "IImage.h"
#include "ISystem.h"
#include "Vector.h"
#include "Common.h"
#include "Random.h"

/*
	This is quite monstrous class, which contain a lot of code, all of the game's mechanics.
	I may not be willing to comment every single line, even every single line that needs to
	be commented.
*/

namespace
{
RegisterIntVar ANIM_AI_LEFT("ANIM_AI_LEFT");
RegisterIntVar ANIM_AI_RIGHT("ANIM_AI_RIGHT");
RegisterIntVar ANIM_AI_UP("ANIM_AI_UP");
RegisterIntVar ANIM_AI_DOWN("ANIM_AI_DOWN");
RegisterIntVar ANIM_AI_UP_LEFT("ANIM_AI_UP_LEFT");
RegisterIntVar ANIM_AI_UP_RIGHT("ANIM_AI_UP_RIGHT");
RegisterIntVar ANIM_AI_DOWN_LEFT("ANIM_AI_DOWN_LEFT");
RegisterIntVar ANIM_AI_DOWN_RIGHT("ANIM_AI_DOWN_RIGHT");
RegisterIntVar ANIM_FLIP_RTL("ANIM_FLIP_RTL");
RegisterIntVar ANIM_FLIP_LTR("ANIM_FLIP_LTR");

RegisterIntVar ANIM_PLAYER_STALL1("ANIM_PLAYER_STALL1");
RegisterIntVar ANIM_PLAYER_STALL2("ANIM_PLAYER_STALL2");

RegisterIntVar GRAPH_CAPS_TURN_ANIMS("GRAPH_CAPS_TURN_ANIMS");
RegisterIntVar GRAPH_CAPS_INDEPENDENT_ANIMS("GRAPH_CAPS_INDEPENDENT_ANIMS");
RegisterIntVar GRAPH_CAPS_INDEPENDENT_ANGLE("GRAPH_CAPS_INDEPENDENT_ANGLE");
RegisterIntVar GRAPH_CAPS_SPECIAL_FALLING("GRAPH_CAPS_SPECIAL_FALLING");
RegisterIntVar GRAPH_CAPS_TRAIL("GRAPH_CAPS_TRAIL");

RegisterIntVar CONTROL_TURN_UP("CONTROL_TURN_UP",		0x0001 );
RegisterIntVar CONTROL_TURN_DOWN("CONTROL_TURN_DOWN",	0x0002 );
RegisterIntVar CONTROL_SPEED_UP("CONTROL_SPEED_UP",		0x0004 );
RegisterIntVar CONTROL_VERT_UP("CONTROL_VERT_UP",		0x0008 );
RegisterIntVar CONTROL_VERT_DOWN("CONTROL_VERT_DOWN",	0x0010 );
RegisterIntVar CONTROL_FIRE1("CONTROL_FIRE1",			0x0020 );
RegisterIntVar CONTROL_FIRE2("CONTROL_FIRE2",			0x0040 );
RegisterIntVar CONTROL_STOP("CONTROL_STOP",				0x0080 );
RegisterIntVar CONTROL_GO("CONTROL_GO",					0x0100 );
RegisterIntVar CONTROL_FLIP("CONTROL_FLIP",				0x0200 );
RegisterIntVar CONTROL_RANDOM_TELEPORT("CONTROL_RANDOM_TELEPORT",	0x0400 );

RegisterIntVar GM_PLAYER("GM_PLAYER");
RegisterIntVar GM_ENEMY("GM_ENEMY");
RegisterIntVar GM_ENEMY_DOUBLE("GM_ENEMY_DOUBLE");
RegisterIntVar GM_BULLET("GM_BULLET");
RegisterIntVar GM_TARGET("GM_TARGET");
RegisterIntVar GM_SHOT_DOWN("GM_SHOT_DOWN");
RegisterIntVar GM_ENEMY_INVUL("GM_ENEMY_INVUL");

RegisterIntVar PHYS_CAPS_FALLING("PHYS_CAPS_FALLING");

UseIntVar PHYS_CAPS_FOLLOW_ANGLE("PHYS_CAPS_FOLLOW_ANGLE");

RegisterIntVar RL_BG_BEFORE("RL_BG_BEFORE");
RegisterIntVar RL_BG_AFTER("RL_BG_AFTER");
RegisterIntVar RL_BACKGROUND("RL_BACKGROUND");
RegisterIntVar RL_MIDGROUND("RL_MIDGROUND");
RegisterIntVar RL_FOREGROUND("RL_FOREGROUND");
}

  // Aiming Aid only searches for targets in this radius
const int MAX_AIM_AID_DIST = 300.0f;

const float FADE_TIME = 1.0f;
const float GAME_OVER_TIME = 4.0f;

  // Converts angle to Radians
float ToRad( float angleDeg )
{
	return angleDeg * ALLEGRO_PI / 180.0f;
}

  // Checks if angle is inside sector
bool AngleInside( float angleRad, float fromDegree, float toDegree )
{
	return angleRad >= ToRad( fromDegree ) && angleRad < ToRad( toDegree );
}

  // Calculates special object points, like gun muzzle, bomb spawn point etc.
void GetObjectPoint( const PhysicComponent & physComp, const GraphicComponent & grComp, EObjPoint pointType, float & x, float & y )
{
	x = physComp.m_x;
	y = physComp.m_y;

	int w = 0, h = 0;

	GraphicComponent::AnimMap::const_iterator iter = grComp.m_animations.find( grComp.m_nextAnim );

	if ( iter == grComp.m_animations.end() )
	{
		IImage *pImg = GetRender().GetImageByHandle( grComp.m_image );
		w = pImg->GetWidth();
		h = pImg->GetHeight();
	}
	else
	{
		const Animation & anim = iter->second;
		if ( anim.m_currentFrame < 0 || anim.m_currentFrame >= (int)anim.m_frames.size() )
			return;

		const SAnimFrame & frame = anim.m_frames[ anim.m_currentFrame ];
		w = frame.w;
		h = frame.h;
	}

	float dx = 0, dy = 0;

	switch( pointType )
	{
		case OBJPOINT_GUN:
		{
			dx = w / 2.0f + 5;
			dy = 0;

			const float cosa = cosf( physComp.m_angle );
			const float sina = sinf( physComp.m_angle );

			const float modx = cosa * dx + sina * dy;
			const float mody = -cosa * dy + sina * dx;

			x += modx;
			y += mody;

			return;
		}
		case OBJPOINT_BOMB:
		{
			y += h /2.0f + 5;
			return;
		}
		default:
			return;
	}
}

GameStateLevel::GameStateLevel()
: BaseGameState( GAMESTATE_LEVEL )
{
	AddKeyboardHandler( this, &GameStateLevel::PauseGame, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_ESCAPE );
	AddGamepadHandlerForControl( JoystickMapping::CONTROL_START, this, &GameStateLevel::PauseGame );
	
	m_aimAidEnabled = true;
	m_debugBulletType = 0;
}

void GameStateLevel::RenderBefore( IRender & render, const IGameContext & context )
{
	m_skyController.Render( render );
	if ( m_skyController.GetImage().IsValid())
	{
		render.DrawImage( m_skyController.GetImage(), 0, 0 );
		m_refController.Update( render, m_skyController.GetImage() );		
	}
}

void GameStateLevel::RenderAfter( IRender & render, const IGameContext & context )
{
	const int w = SCREEN_W;
	const int h = SCREEN_H;	

	m_ui.Render( context, m_playerObjectID, render );

	m_pbp.Render( render );

	m_lifeLossEffect.Render( render );

	if ( m_fade )
	{
		int c = 255 * ( m_gameOverType == GAMEOVER_TYPE_NONE ? (m_fadeTimer / FADE_TIME) : (1.0f-m_fadeTimer / FADE_TIME) );
		if ( c > 255 ) c = 255; if ( c < 0 ) c = 0;
		render.DrawRectFill( 0, 0, SCREEN_W, SCREEN_H, Color(0,0,0,c) );
	}
}

void GameStateLevel::ControlsToPhysics( GameObject *pObject, IGameContext & context, float dt )
{
	/*
		This is somewhat complex and hacky function with a lot of special cases. It grown organically, so
		be prepared.
	*/

	PhysicComponent *pPhComp = context.GetOjectsStorage().GetPhysicComponent( pObject->GetPhysicComponent() );
	ControlComponent *pCnComp = context.GetOjectsStorage().GetControlComponent( pObject->GetControlComponent() );

	if ( !pCnComp || !pPhComp )
		return;

	  // Non-dead objects need to send controls to have vertical speed.
	  // Dead have it easy.
	if ( !(pPhComp->m_caps & PHYS_CAPS_FALLING) )
		pPhComp->m_speedY = 0;

	  // This is simple: is object wants to turn up or down - oblige it. Notice that turn speed changes with some inertia.
	if ( pCnComp->m_controls & CONTROL_TURN_UP )
	{
		pPhComp->m_angle -= pPhComp->m_turnSpeed * dt;
		pPhComp->m_turnSpeed += pPhComp->m_maxTurnSpeed * 10 * dt;
		if ( pPhComp->m_turnSpeed > pPhComp->m_maxTurnSpeed ) pPhComp->m_turnSpeed = pPhComp->m_maxTurnSpeed;
	}
	if ( pCnComp->m_controls & CONTROL_TURN_DOWN )
	{
		pPhComp->m_angle += pPhComp->m_turnSpeed * dt;
		pPhComp->m_turnSpeed += pPhComp->m_maxTurnSpeed * 10 * dt;
		if ( pPhComp->m_turnSpeed > pPhComp->m_maxTurnSpeed ) pPhComp->m_turnSpeed = pPhComp->m_maxTurnSpeed;
	}
	if ( (pCnComp->m_controls & ( CONTROL_TURN_DOWN | CONTROL_TURN_UP )) == 0 )
	{
		pPhComp->m_turnSpeed -= pPhComp->m_maxTurnSpeed * 10 * dt;
		if ( pPhComp->m_turnSpeed < 0 ) pPhComp->m_turnSpeed = 0;
	}

	  // If object wants to fly up or down - give it vertical speed
	if ( pCnComp->m_controls & CONTROL_VERT_UP )
		pPhComp->m_speedY = -pPhComp->m_vertSpeed;
	if ( pCnComp->m_controls & CONTROL_VERT_DOWN )
		pPhComp->m_speedY = pPhComp->m_vertSpeed;

	  // Normalize angle inside [0,2PI]
	if ( pPhComp->m_angle > 2*ALLEGRO_PI )
		pPhComp->m_angle -= 2*ALLEGRO_PI;
	if ( pPhComp->m_angle < 0 )
		pPhComp->m_angle += 2*ALLEGRO_PI;

	if ( pCnComp->m_controls & CONTROL_SPEED_UP )
	{
		  // If object wants to accelerate
		if ( pPhComp->m_caps & PHYS_CAPS_FOLLOW_ANGLE )
		{
			  // Player's plane just accelerates along its m_angle, so just change speed here
			if ( pPhComp->m_speed < pPhComp->m_maxSpeed )
			{
				pPhComp->m_speed += pPhComp->m_accel*dt;
				if ( pPhComp->m_speed > pPhComp->m_maxSpeed )
					pPhComp->m_speed = pPhComp->m_maxSpeed;
			}
		}
		else
		{
			  // Other object have separate speed for X and Y component
			  // Notice that we only measure speedX here. 
			if ( fabs(pPhComp->m_speedX) < pPhComp->m_maxSpeed )
			{
				pPhComp->m_speedX += ( pCnComp->m_direction == RIGHT_TO_LEFT ? -1 : 1 ) * pPhComp->m_accel*dt;
				if ( fabs(pPhComp->m_speedX) > pPhComp->m_maxSpeed )
					pPhComp->m_speedX = ( pCnComp->m_direction == RIGHT_TO_LEFT ? -1 : 1 ) * pPhComp->m_maxSpeed;
			}
		}
	}
	else
	{
		  // If object does not want to accelearate, it should de-accelerate to its minimum speed
		if ( pPhComp->m_speed > pPhComp->m_minSpeed )
		{
			pPhComp->m_speed -= pPhComp->m_accel*dt;
			if ( pPhComp->m_speed < pPhComp->m_minSpeed )
				pPhComp->m_speed = pPhComp->m_minSpeed;
		}
	}

	  // Some objects just want to stop dead.
	if ( pCnComp->m_controls & CONTROL_STOP )
	{
		pPhComp->m_speed = 0;
		pPhComp->m_speedX = 0;
		pPhComp->m_speedY = 0;
	}

	  // And some objects want to resume their movement after stops
	if ( pCnComp->m_controls & CONTROL_GO )
	{
		if ( pPhComp->m_caps & PHYS_CAPS_FOLLOW_ANGLE )
		{
			pPhComp->m_speed = pPhComp->m_minSpeed;
		}
		else
		{
			pPhComp->m_speedX = pPhComp->m_minSpeed * ( pCnComp->m_direction == LEFT_TO_RIGHT ? 1 : -1 );
		}
	}

	  // For UFOs
	if ( pCnComp->m_controls & CONTROL_RANDOM_TELEPORT )
	{
		pPhComp->m_x = RndInt( 0, SCREEN_W );
		pPhComp->m_y = RndInt( 0, SCREEN_H / 2 );
	}

	  // Update status of "about-face" action
	pPhComp->UpdateFlip( dt );

	if ( !pPhComp->IsFlipping() && (pCnComp->m_controls & CONTROL_FLIP) )
	{
		pPhComp->StartFlip();
	}

	if ( pPhComp->CheckFlipped() )
	{
		if ( pCnComp->m_direction == LEFT_TO_RIGHT )
			pCnComp->m_direction = RIGHT_TO_LEFT;
		else
			pCnComp->m_direction = LEFT_TO_RIGHT;
	}

	pPhComp->m_direction = pCnComp->m_direction;
}

void GameStateLevel::PhysicsToGraphics( GameObject *pObject,IGameContext & context, float dt )
{
	const PhysicComponent *pPhComp = context.GetOjectsStorage().GetPhysicComponent( pObject->GetPhysicComponent() );
	GraphicComponent *pGrComp = context.GetOjectsStorage().GetGraphicComponent( pObject->GetGraphicComponent() );
	MechanicComponent *pMcComp = context.GetOjectsStorage().GetMechanicComponent( pObject->GetMechanicComponent() );

	if ( !pPhComp || !pGrComp )
		return;

	  // Common
	pGrComp->m_x = pPhComp->m_x;
	pGrComp->m_y = pPhComp->m_y;
	
	if ( pGrComp->m_caps & GRAPH_CAPS_TURN_ANIMS )
	{
		  // Objects that has separate animations for different angles (player's plane)
		  // If player's plane is stalled, force its animation to be the one of stall.
		if ( pMcComp )
		{
			if ( pMcComp->IsStalledPhase1() )
			{
				pGrComp->m_angle = 0;
				pGrComp->m_nextAnim = ANIM_PLAYER_STALL1;
				return;
			}
			else if ( pMcComp->IsStalledPhase2() )
			{
				pGrComp->m_nextAnim = ANIM_PLAYER_STALL2;			
				return;
			}
		}		

		  // Select animation by angle. This is harcdoed.
		const float angle = pPhComp->m_angle;

		const int animID = (int)(( angle + ToRad( 15.0f ) ) / ToRad( 30.0f )) % 12;
		pGrComp->m_nextAnim = animID;
		pGrComp->m_angle = pPhComp->m_angle - animID * ToRad( 30.0f );
	}
	else if ( pGrComp->m_caps & GRAPH_CAPS_INDEPENDENT_ANIMS )
	{
		  // Some objects have animations which are independed of all directions
		  // And some objects do not even want their orientation to corrspond to their physic angle
		if ( !(pGrComp->m_caps & GRAPH_CAPS_INDEPENDENT_ANGLE) )
		{
			pGrComp->m_angle = pPhComp->m_angle;
		}
	}
	else if ( pGrComp->m_caps & GRAPH_CAPS_SPECIAL_FALLING )
	{
		  // Falling objects use DOWN_RIGHT and DOWN_LEFT animations
		if ( pPhComp->m_direction == LEFT_TO_RIGHT )
			pGrComp->m_nextAnim = ANIM_AI_DOWN_RIGHT;
		else
			pGrComp->m_nextAnim = ANIM_AI_DOWN_LEFT;
	}
	else
	{
		  // The most general code that selects animation based on direction and speed
		if ( pPhComp->m_speedY == 0 )
		{
			if ( fabs(pPhComp->m_speedX) < 0.00001f )
			{
				pGrComp->m_nextAnim = pPhComp->m_direction == LEFT_TO_RIGHT ? ANIM_AI_RIGHT : ANIM_AI_LEFT;
			}
			else
			{
				pGrComp->m_nextAnim = pPhComp->m_speedX < 0 ? ANIM_AI_LEFT : ANIM_AI_RIGHT;
			}
		}
		else
		{
			if ( pPhComp->m_speedX == 0 )
			{
				if ( pPhComp->m_speedY < 0 )
					pGrComp->m_nextAnim = pPhComp->m_direction == LEFT_TO_RIGHT ? ANIM_AI_UP_RIGHT : ANIM_AI_UP_LEFT;
				else
					pGrComp->m_nextAnim = pPhComp->m_direction == LEFT_TO_RIGHT ? ANIM_AI_DOWN_RIGHT : ANIM_AI_DOWN_LEFT;
			}
			else
			{
				if ( pPhComp->m_speedX > 0 )
					pGrComp->m_nextAnim = pPhComp->m_speedY < 0 ? ANIM_AI_UP_RIGHT : ANIM_AI_DOWN_RIGHT;
				else
					pGrComp->m_nextAnim = pPhComp->m_speedY < 0 ? ANIM_AI_UP_LEFT : ANIM_AI_DOWN_LEFT;
			}
		}

		pGrComp->m_angle = 0;
	}

	if ( pPhComp->IsFlipping() )
	{
		pGrComp->m_nextAnim = pPhComp->m_direction == LEFT_TO_RIGHT ? ANIM_FLIP_RTL : ANIM_FLIP_LTR;
	}

	// Debug code to draw collision spheres
/*	for ( PhysicComponent::SpheresVector::const_iterator iter = pPhComp->m_collisionSpheres.begin();
		iter != pPhComp->m_collisionSpheres.end();
		++iter)
	{
		float x = iter->m_dx;
		float y = iter->m_dy;

		if ( pPhComp->m_caps & PHYS_CAPS_FOLLOW_ANGLE )
		{
			const float cosa = cosf( pPhComp->m_angle );
			const float sina = sinf( pPhComp->m_angle );

			const float modx = cosa * x + sina * y;
			const float mody = -cosa * y + sina * x;
		
			x = modx + pPhComp->m_x;
			y = mody + pPhComp->m_y;
		}
		else
		{
			if ( pPhComp->m_direction == RIGHT_TO_LEFT )
				x = -x;
			
			x = x + pPhComp->m_x;
			y = y + pPhComp->m_y;
		}
		GetRender().DrawCircle( x,y, iter->m_r, Color(255,255,255));	
	}*/
}

void GameStateLevel::PhysicsToSound( GameObject *pObject,IGameContext & context, float dt )
{
	PhysicComponent *pPhComp = context.GetOjectsStorage().GetPhysicComponent( pObject->GetPhysicComponent() );
	SoundComponent *pSnComp = context.GetOjectsStorage().GetSoundComponent( pObject->GetSoundComponent() );

	if ( !pSnComp || !pPhComp )
		return;

	pSnComp->m_angle = pPhComp->m_angle;
}

void GameStateLevel::PhysicsToAI( GameObject *pObject,IGameContext & context, float dt )
{
	PhysicComponent *pPhComp = context.GetOjectsStorage().GetPhysicComponent( pObject->GetPhysicComponent() );
	ControlComponent *pCnComp = context.GetOjectsStorage().GetControlComponent( pObject->GetControlComponent() );

	if ( !pCnComp || !pPhComp )
		return;

	pCnComp->m_aiData.m_x = pPhComp->m_x;
	pCnComp->m_aiData.m_y = pPhComp->m_y;
	pCnComp->m_aiData.m_angle = pPhComp->m_angle;
}

void GameStateLevel::GraphicsToSound( GameObject *pObject,IGameContext & context, float dt )
{
	GraphicComponent *pGrComp = context.GetOjectsStorage().GetGraphicComponent( pObject->GetGraphicComponent() );
	SoundComponent *pSnComp = context.GetOjectsStorage().GetSoundComponent( pObject->GetSoundComponent() );

	if ( !pGrComp )
		return;

	bool changeSound = pGrComp->m_currentAnim != pGrComp->m_nextAnim;
	pGrComp->m_currentAnim = pGrComp->m_nextAnim;

	  // If animation changed - send associated event into soudn system
	if ( pSnComp && changeSound )
	{
		GraphicComponent::AnimMap::const_iterator iter = pGrComp->m_animations.find( pGrComp->m_currentAnim );
		if ( iter != pGrComp->m_animations.end() )
			pSnComp->PushEvent( iter->second.m_soundEvent );
	}
}

void GameStateLevel::FireWeapon( int weaponIndex, const PhysicComponent *pPhComp, const ControlComponent *pCnComp, const GraphicComponent *pGrComp, MechanicComponent *pMechComp, SoundComponent *pSnComp, IGameContext & context, const std::string & soundEvent )
{
	if ( weaponIndex < 0 || weaponIndex >= MAX_WEAPONS )
		return;

	SWeapon & weapon = pMechComp->m_weapons[ weaponIndex ];

	  // Check angle and height limits
	if ( weapon.m_heightLimit > 0 && pPhComp->m_y > weapon.m_heightLimit )
		return;

	const float testAngle = acosf( fabs(cosf( pPhComp->m_angle )) );
	if ( testAngle > weapon.m_angleLimit * ALLEGRO_PI / 180.0f )
		return;

	if ( !weapon.m_object.empty() && weapon.m_currentCooldown <= 0 && ( weapon.m_ammoType == AMMO_INFINITE || weapon.m_currentAmmo > 0 ) )
	{
		float x, y;
		GetObjectPoint( *pPhComp, *pGrComp, weapon.m_point, x, y );
		SCreateObjectRequest::SData data;
		data.x = x;
		data.y = y;
		data.ai = weapon.m_ai;
		data.dir = (EDirection)pCnComp->m_direction;

		  // Special case for player, because we should fire along plane's direction AND we may use aiming aid
		if ( pCnComp->m_type == ControlComponent::TYPE_PLAYER && weaponIndex == m_debugBulletType )
		{
			data.angle = pPhComp->m_angle;
			++context.GetPlayerProfile().m_levelShotsFired;

			if ( m_aimAidEnabled )
			{
				float smallestDist = MAX_AIM_AID_DIST;
				float targetA = 0.0f;
				GameObjectID targetID;

				/*
					The idea is simple: find closes target which is in Aiming Aid sector and
					fire bullet at it, not straight ahead. It does not take movement into account
					and therefore can be inaccurate in judging firing direction, but it still
					slightly easier to hit close-flying planes with this option.
				*/
				ObjectsStorage::GameObjectContainer::Iterator iter = context.GetOjectsStorage().GetGameObjects().GetIterator();
				for ( ; !iter.AtEnd(); ++iter )
				{
					const MechanicComponent *pMech = context.GetOjectsStorage().GetMechanicComponent( (*iter)->GetMechanicComponent() );
					const PhysicComponent *pPhys = context.GetOjectsStorage().GetPhysicComponent( (*iter)->GetPhysicComponent() );
					if ( !pMech || !pPhys || ( pMech->m_type != GM_ENEMY && pMech->m_type != GM_ENEMY_DOUBLE ) )
						continue;

					physics::VECTOR myDir( cosf(pPhComp->m_angle), sinf(pPhComp->m_angle), 0 );
					physics::VECTOR dirToTarget( pPhys->m_x	- pPhComp->m_x, pPhys->m_y - pPhComp->m_y, 0 );

					const float dist = dirToTarget.length();
					if ( dist > MAX_AIM_AID_DIST )
						continue;
					const float maxDA = ALLEGRO_PI / 20.0f + ( dist > MAX_AIM_AID_DIST ? 0 : ( 1.0f - ( dist / MAX_AIM_AID_DIST ) ) ) * ( ALLEGRO_PI / 6.0f - ALLEGRO_PI / 20.0f );

					myDir.normalize();
					dirToTarget.normalize();

					const float cosAngle = myDir.dot( dirToTarget );
					if ( cosAngle <= 0 )
						continue;

					if ( acosf(cosAngle) < maxDA )
					{
						const float angle = atan2f( dirToTarget.y, dirToTarget.x );
						float da = fabs(angle - pPhComp->m_angle);
						if ( da >= 2*ALLEGRO_PI )
							da -= 2*ALLEGRO_PI;
						if ( dist < smallestDist )
						{

							smallestDist = dist;
							targetA = angle;
							targetID = (*iter)->GetID();
						}
						break;
					}
				}

				if ( targetID.IsValid() )
				{
					data.angle = targetA;
					//context.GetOjectsStorage().GetGraphicComponent( context.GetOjectsStorage().GetGameObject( targetID )->GetGraphicComponent() )->m_debugFlag = true;
				}
			}
		}
		else
		{
			if ( weaponIndex == 0 )
			{
				data.angle = pPhComp->m_angle;
			}
			else
			{
				data.angle = cosf( pPhComp->m_angle ) >= 0.0f ? 0 : ALLEGRO_PI;
				data.dir = cosf( pPhComp->m_angle ) >= 0.0f ? LEFT_TO_RIGHT : RIGHT_TO_LEFT;
			}
		}

		context.CreateGameObject( weapon.m_object, data );
		
		if ( pCnComp->m_type == ControlComponent::TYPE_PLAYER )
		{
			if ( context.IsGodMode() )
				weapon.m_currentCooldown = 0.01f;
			weapon.m_currentCooldown = weapon.m_cooldown;
		}
		else
			weapon.m_currentCooldown = weapon.m_cooldown;

		if ( weapon.m_ammoType != AMMO_INFINITE && !context.IsGodMode() )
			weapon.m_currentAmmo -= 1;

		if ( pSnComp && !soundEvent.empty() )
			pSnComp->PushEvent( soundEvent );
	}
}

void GameStateLevel::ControlsToMechanic( GameObject *pObject,IGameContext & context, float dt )
{
	ControlComponent *pCnComp = context.GetOjectsStorage().GetControlComponent( pObject->GetControlComponent() );
	MechanicComponent *pMechComp = context.GetOjectsStorage().GetMechanicComponent( pObject->GetMechanicComponent() );

	if ( !pMechComp )
		return;
	
	for ( int i = 0; i < MAX_WEAPONS; ++i )
	{
		SWeapon & wpn = pMechComp->m_weapons[ i ];
		if ( wpn.m_currentCooldown > 0 )
			wpn.m_currentCooldown -= dt;
		if ( wpn.m_currentCooldown < 0 )
			wpn.m_currentCooldown = 0;
	}

	if ( !pCnComp )
		return;

	PhysicComponent *pPhComp = context.GetOjectsStorage().GetPhysicComponent( pObject->GetPhysicComponent() );
	GraphicComponent *pGrComp = context.GetOjectsStorage().GetGraphicComponent( pObject->GetGraphicComponent() );
	SoundComponent *pSnComp = context.GetOjectsStorage().GetSoundComponent( pObject->GetSoundComponent() );
	if ( !pPhComp || !pGrComp )
		return;

	if ( pCnComp->m_controls & CONTROL_FIRE1 )
	{
		if ( pMechComp->m_type == GM_PLAYER )
		{
			if ( m_resetWeaponCooldown )
			{
				if ( pMechComp->m_weapons[0].m_currentCooldown > 1 / 20.0f )
				{
					const float passed = pMechComp->m_weapons[0].m_cooldown - pMechComp->m_weapons[0].m_currentCooldown;
					pMechComp->m_weapons[0].m_currentCooldown = 1 / 20.0f - passed;
					if ( passed > pMechComp->m_weapons[0].m_currentCooldown < 0 )
						pMechComp->m_weapons[0].m_currentCooldown = 0;
				}

				m_resetWeaponCooldown = false;
			}
		}

		int weaponIdx = 0;
		if ( m_debugBulletType == 0 )
		{
			if ( pMechComp->m_type == GM_PLAYER )
			{
				time_t t = time( 0 );
				struct tm *tm = gmtime( &t );
			
				if ( tm ) 
				{
					for ( int i = 0; i < MAX_WEAPONS; ++i )
					{
						if ( pMechComp->m_weapons[i].m_date.m_month == ( tm->tm_mon + 1 ) && pMechComp->m_weapons[i].m_date.m_day == tm->tm_mday )
						{
							weaponIdx = i;
							break;
						}
					}
				}
			}
		}
		else
			weaponIdx = m_debugBulletType;

		FireWeapon( weaponIdx, pPhComp, pCnComp, pGrComp, pMechComp, pSnComp, context, "fire1" );
	}
	if ( pCnComp->m_controls & CONTROL_FIRE2 )
		FireWeapon( 1, pPhComp, pCnComp, pGrComp, pMechComp, pSnComp, context, "fire2" );
}

void GameStateLevel::CheckWinConditions( IGameContext & context )
{
		// If we're not dead and we have destroyed enough waterline pixels - we win level!
	    // Also, wait until the last PBP message is hidden to avoid bad-looking behavior
	if ( !m_pbp.IsActive() && m_gameOverTimer <= 0.0f && context.GetLevelDirector().CheckWinConditions() && !m_lifeLossEffect.IsActive() )
	{
		PlayerProfile & prof = context.GetPlayerProfile();
		context.PushState( GAMESTATE_AFTER_LEVEL );
		prof.m_levelTime = m_levelTimer;
		if ( prof.m_levelLivesLost == 0 && context.GetCurrentLevel() > 1 )
		{
			++prof.m_unkillableStreak;
		}
		m_pbp.Reset();
	}
}

void GameStateLevel::ProcessSingleCollision( const MechanicComponent *pMe, const MechanicComponent *pOther, IGameContext & context )
{
	if ( pMe->m_type == GM_PLAYER )
	{
		if ( pMe->m_invulTimer >= 0 || m_lifeLossEffect.IsActive() || context.IsGodMode() )
			return;

		  // If player's object collided with anything, it's dead. It does not collide with harmless things.
		const GameObject *pMeObj = context.GetOjectsStorage().GetGameObject( pMe->GetParentID() );
		const PhysicComponent *pMePhys = context.GetOjectsStorage().GetPhysicComponent( pMeObj->GetPhysicComponent() );
		SCreateObjectRequest::SData data;
		data.x = pMePhys->m_x;
		data.y = pMePhys->m_y;
		data.ai = pMe->m_deathAI;
		data.dir = (EDirection)pMePhys->m_direction;
		data.hasSpeed = true;
		data.speedX = 0;
		data.speedY = 200;

		m_playerObjectRequestID = context.CreateGameObject( pMe->m_deathObject, data, m_playerObjectID );
		m_playerObjectID = GameObjectID();

		PlayerProfile & prof = context.GetPlayerProfile();
		++prof.m_levelLivesLost;
		prof.m_unkillableStreak = 0;
		
		--context.GetOjectsStorage().GetPlayer().m_lives;
		if ( context.GetOjectsStorage().GetPlayer().m_lives <= 0 )
		{
			m_gameOverTimer = GAME_OVER_TIME;
			m_gameOverType = GAMEOVER_TYPE_LIVES;
		}
		else
		{
			const GameObject *pOtherObj = context.GetOjectsStorage().GetGameObject( pOther->GetParentID() );
			const GraphicComponent *pOtherGraph = context.GetOjectsStorage().GetGraphicComponent( pOtherObj->GetGraphicComponent() );

			const Animation & anim = pOtherGraph->m_animations.find(pOtherGraph->m_currentAnim)->second;
			m_lifeLossEffect.Restart( pOtherGraph->m_image, anim.m_frames.empty() ? 0 : &anim.m_frames[0] );
		}
	}
	else if ( pMe->m_type == GM_BULLET )
	{
		  // If bullet collided with anything, remove the bullet and increase hits statistic
		context.DestroyGameObject( pMe->GetParentID() );
		++context.GetPlayerProfile().m_levelHits;
	}
	else if ( pMe->m_type == GM_ENEMY )
	{
		  // If enemy collided with bullet, destroy enemy and increase various stats.
		if ( pOther->m_type == GM_BULLET )
		{
			if ( pMe->m_statType >= 0 && pMe->m_statType < STAT_TYPES_COUNT )
				++context.GetPlayerProfile().m_levelKillsByType[ pMe->m_statType ];

			PlayerProfile & prof = context.GetPlayerProfile();
			if ( !prof.m_levelTimedKills.empty() )
			{
				if ( (int)prof.m_levelTimedKills.size() > prof.m_maxLevelTimedKills )
					prof.m_maxLevelTimedKills = (int)prof.m_levelTimedKills.size();;

				if ( m_levelTimer - prof.m_levelTimedKills.back() >= 5.0f )
					prof.m_levelTimedKills.clear();
				else
				{
					while( !prof.m_levelTimedKills.empty() && ( m_levelTimer - prof.m_levelTimedKills.front() >= 5.0f ) )
						prof.m_levelTimedKills.pop_front();
				}
			}

			prof.m_levelTimedKills.push_back( m_levelTimer );

			context.GetOjectsStorage().GetPlayer().m_score += pMe->m_score;
			if ( !pMe->m_deathObject.empty() )
			{
				const GameObject *pMeObj = context.GetOjectsStorage().GetGameObject( pMe->GetParentID() );
				const PhysicComponent *pMePhys = context.GetOjectsStorage().GetPhysicComponent( pMeObj->GetPhysicComponent() );
				SCreateObjectRequest::SData data;
				data.x = pMePhys->m_x;
				data.y = pMePhys->m_y;
				data.dir = (EDirection)pMePhys->m_direction;
				data.hasSpeed = true;
				data.ai = pMe->m_deathAI;
				data.speedX = pMePhys->m_speedX;
				data.speedY = 0;

				context.CreateGameObject( pMe->m_deathObject, data, pMe->GetParentID() );
			}
			else
			{
				context.DestroyGameObject( pMe->GetParentID() );
			}
		}
	}
	else if ( pMe->m_type == GM_SHOT_DOWN && pOther->m_type == GM_TARGET )
	{
		  // If falling enemy or bomb collided with target, destroy a chunk of target

		PlayerProfile & prof = context.GetPlayerProfile();
		++prof.m_levelTargetHits;
		if ( pMe->m_statType >= 0 && pMe->m_statType < STAT_TYPES_COUNT )
			++prof.m_levelTargetHitsByType[ pMe->m_statType ];

		prof.m_levelLastTargetHitType = pMe->m_statType;

		  // Create explosion on target
		const GameObject *pMeObj = context.GetOjectsStorage().GetGameObject( pMe->GetParentID() );
		const PhysicComponent *pMePhys = context.GetOjectsStorage().GetPhysicComponent( pMeObj->GetPhysicComponent() );
		SCreateObjectRequest::SData data;
		data.x = pMePhys->m_x;
		data.y = pMePhys->m_y;
		data.ai = pMe->m_deathAI;
		data.dir = (EDirection)pMePhys->m_direction;
		data.hasSpeed = true;
		data.speedX = 0;
		data.speedY = 0;

		context.CreateGameObject( pMe->m_deathObject, data, pMe->GetParentID() );

		const GameObject *pOtherObj = context.GetOjectsStorage().GetGameObject( pOther->GetParentID() );
		const GraphicComponent *pOtherGraph = context.GetOjectsStorage().GetGraphicComponent( pOtherObj->GetGraphicComponent() );
		PhysicComponent *pOtherPhys = context.GetOjectsStorage().GetPhysicComponent( pOtherObj->GetPhysicComponent() );
		
		  // Here and in other places you'll see this direct call to Allegro functions instead of wrapper. I'm lazy.
		int a,b,c;
		al_get_blender( &a, &b, &c );
		al_set_blender( ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ONE, ALLEGRO_ONE );

		const int dx = pMePhys->m_x - pOtherGraph->m_x;
		const int dy = pMePhys->m_y - pOtherGraph->m_y;
		const int w =  pOtherPhys->m_collisionMask.GetWidth();
		const int h =  pOtherPhys->m_collisionMask.GetHeight();

		LevelDirector & director = context.GetLevelDirector();

		float radius = 20;
		const int MAX_ADD_ALL = 1 + (1.0f + 1.0f)/2.0f * 15;
		const float maxRadius = radius + MAX_ADD_ALL;

		GetRender().DrawImage( pOtherGraph->m_image, m_damageImage, dx + w/2 - maxRadius, dy + h/2 - maxRadius );
		SDamageCache cache;
		cache.x = dx + w/2 - maxRadius;
		cache.y = dy + h/2 - maxRadius;
		m_damageCache.push_back( cache );

		for ( int x = 0; x < maxRadius * 2; ++x )
		for ( int y = 0; y < maxRadius * 2; ++y )
		{
			const int tx = x + dx + w/2 - maxRadius;
			const int ty = y + dy + h/2 - maxRadius;
			if ( m_damageMap.Get( x, y ) )
			{
				if ( pOtherPhys->m_collisionMask.Get( tx, ty ) )
				{
					pOtherPhys->m_collisionMask.Set( tx, ty, false );
					//if ( pOtherPhys->m_collisioMaskDebug.IsValid() ) GetRender().DrawPixel( pOtherPhys->m_collisioMaskDebug, tx, ty, Color( 50, 0, 50, 100 ), true );
					if ( ty == director.GetSinkLineY() )
					{
						director.UpdateSinkPixels( -1 );
					}
				}
			}
		}
		
		al_set_blender( a, b, c );
		//if ( pOtherPhys->m_collisioMaskDebug.IsValid() ) GetRender().UnlockImage( pOtherPhys->m_collisioMaskDebug );

		CheckWinConditions( context );
		
		  // PBP messages are shown based on percent of destroyed waterline pixels
		m_pbp.UpdatePBPIndex( director.GetPBPIndex() );
		// Debug code to display current target's collision mask
		/*if ( pOtherPhys->m_collisioMaskDebug == ImageHandle::INVALID )
		{
			pOtherPhys->m_collisioMaskDebug = GetRender().CreateImage( pOtherPhys->m_collisionMask.GetWidth(), pOtherPhys->m_collisionMask.GetHeight(), true );

			GetRender().ClearImage( pOtherPhys->m_collisioMaskDebug, Color(0,0,0,50));
			al_get_blender( &a, &b, &c );
			al_set_blender( ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO );
			GetRender().LockImage( pOtherPhys->m_collisioMaskDebug );				
			int pp = 0;
			for ( int x = 0; x < pOtherPhys->m_collisionMask.GetWidth(); ++x )
			{
				for ( int y = 0; y < pOtherPhys->m_collisionMask.GetHeight(); ++y )
				{
					if ( pOtherPhys->m_collisionMask.Get( x, y ) )
					{
						++pp;
						GetRender().DrawPixel( pOtherPhys->m_collisioMaskDebug, x, y, Color( 255, 0, 0, 100 ), true );
					}
					else
					{
						GetRender().DrawPixel( pOtherPhys->m_collisioMaskDebug, x, y, Color( 0, 0, 0, 100 ), true );
					}
				}
			}
			GetRender().UnlockImage( pOtherPhys->m_collisioMaskDebug );
			al_set_blender( a, b, c );
		}
		GetRender().DrawRectFill( pOtherPhys->m_collisioMaskDebug, pMePhys->m_x - pOtherGraph->m_x, pMePhys->m_y - pOtherGraph->m_y, pMePhys->m_x - pOtherGraph->m_x+20, pMePhys->m_y - pOtherGraph->m_y + 20, Color(255,0,255) );*/
	}
}

void GameStateLevel::ProcessCollisions( IGameContext & context )
{
	const CollisionData & collisions = context.GetOjectsStorage().GetCollisions();
	
	  // Most objects are destroyed when the collide with borders
	  // Player's plane teleports to the other side.
	const size_t borderCollisions = collisions.GetBorderCollisionsCount();
	for ( size_t i = 0; i < borderCollisions; ++i )
	{
		const SBorderCollision *pColl = collisions.GetBorderCollision( i );
		const GameObject *pObject = context.GetOjectsStorage().GetGameObject( pColl->m_objectID );
		if ( !pObject )
			return;

		ControlComponent *pCnComp = pObject ? context.GetOjectsStorage().GetControlComponent( pObject->GetControlComponent() ) : 0;
		if ( pColl->m_borders & ( BORDER_LEFT | BORDER_RIGHT | BORDER_TOP | BORDER_BOTTOM ) )
		{
			if ( !pCnComp || pCnComp->m_type != ControlComponent::TYPE_PLAYER )
			{
				const MechanicComponent *pMcComp = context.GetOjectsStorage().GetMechanicComponent( pObject->GetMechanicComponent() );
				if ( pMcComp && pMcComp->m_type == GM_SHOT_DOWN )
					++context.GetPlayerProfile().m_levelTargetMisses;

				context.DestroyGameObject( pColl->m_objectID );
				
			}
			else if ( pColl->m_borders & BORDER_TOP )
			{
				PhysicComponent *pPhComp = context.GetOjectsStorage().GetPhysicComponent( pObject->GetPhysicComponent() );
				MechanicComponent *pMcComp = context.GetOjectsStorage().GetMechanicComponent( pObject->GetMechanicComponent() );

				pCnComp->m_controls = 0;

				if ( pPhComp )
				{
					pPhComp->m_angle = ALLEGRO_PI/2.0f;
					pPhComp->m_speed = pPhComp->m_minSpeed;
				}
				
				if ( pMcComp )
					pMcComp->StartStall();
			}
		}
	}

	const unsigned int objectCollisions = collisions.GetObjectCollisionsCount();
	for ( size_t i = 0; i < objectCollisions; ++i )
	{
		const SObjectCollision *pColl = collisions.GetObjectCollision( i );
		const GameObject *pObject1 = context.GetOjectsStorage().GetGameObject( pColl->m_object1 );
		const GameObject *pObject2 = context.GetOjectsStorage().GetGameObject( pColl->m_object2 );

		const MechanicComponent *pMech1 = context.GetOjectsStorage().GetMechanicComponent( pObject1->GetMechanicComponent() );
		const MechanicComponent *pMech2 = context.GetOjectsStorage().GetMechanicComponent( pObject2->GetMechanicComponent() );

		if ( !pMech1 || !pMech2 )
		{
			if ( !pMech1 )
				context.DestroyGameObject( pObject1->GetID() );
			if ( !pMech2 )
				context.DestroyGameObject( pObject2->GetID() );
			continue;
		}

		ProcessSingleCollision( pMech1, pMech2, context );
		ProcessSingleCollision( pMech2, pMech1, context );
	}
}

void GameStateLevel::Update( float dt, IGameContext & context )
{
	m_lifeLossEffect.Update( dt );

	if ( m_tutorialEnabled )
	{
		if ( m_tutorialTimer > 0.0f )
		{
			m_tutorialTimer -= dt;
			if ( m_tutorialTimer <= 0.0f )
			{
				m_tutorialTimer = 0.0f;
				context.PushState( GAMESTATE_TUTORIAL );
			}
		}
	}

	m_levelTimer += dt;
	if ( m_fadeTimer > 0.0f )
	{
		m_fadeTimer -= dt;
		if ( m_fadeTimer < 0 )
		{
			m_fade = false;
			m_fadeTimer = 0;
		}
	}

	if ( m_gameOverTimer > 0.0f )
	{
		m_gameOverTimer -= dt;
		if ( !m_fade && m_gameOverTimer < FADE_TIME )
		{
			m_fade = true;
			m_fadeTimer = m_gameOverTimer;
		}
		if ( m_gameOverTimer < 0 )
		{
			m_gameOverTimer = 0;
			if ( m_gameOverType == GAMEOVER_TYPE_LIVES )
				context.PushState( GAMESTATE_GAME_OVER_LIVES );
			else if ( m_gameOverType == GAMEOVER_TYPE_TIME )
				context.PushState( GAMESTATE_GAME_OVER_TIME );
			else
				assert( false && "Game over time expired, but game over type is not valid" );

			context.RemoveState( GAMESTATE_LEVEL );
		}
	}

	m_pbp.Update( context, dt );

	  // If our saved options version does not equal globel options version,
	  // we need to re-apply keyboard settings and gameplay flags.
	if ( m_optionsVersion != g_options_version )
	{
		ISystem & system = GetSystem();
		const int keyClock		= system.GetConfigValue_Int( "DD_Controls", "Kbd_Clock" );
		const int keyCounter	= system.GetConfigValue_Int( "DD_Controls", "Kbd_Counter" );
		const int keyAccel		= system.GetConfigValue_Int( "DD_Controls", "Kbd_Accel" );
		const int keyFire		= system.GetConfigValue_Int( "DD_Controls", "Kbd_Fire" );
		const int keyBomb		= system.GetConfigValue_Int( "DD_Controls", "Kbd_Bomb" );

		m_aimAidEnabled = system.HasConfigValue("DD_Gameplay", "AimAid" ) ? system.GetConfigValue_Int( "DD_Gameplay", "AimAid" ) : true;

		AddKeyboardHandler( this, &GameStateLevel::TurnUp,   KeyboardEvent::KBD_KEY_PRESSED, keyCounter ? keyCounter : ALLEGRO_KEY_LEFT );
		AddKeyboardHandler( this, &GameStateLevel::TurnDown, KeyboardEvent::KBD_KEY_PRESSED, keyClock   ? keyClock   : ALLEGRO_KEY_RIGHT );
		AddKeyboardHandler( this, &GameStateLevel::SpeedUp,  KeyboardEvent::KBD_KEY_PRESSED, keyAccel   ? keyAccel   : ALLEGRO_KEY_UP );
		AddTouchHandler( this, &GameStateLevel::TouchBegin,  TouchEvent::TOUCH_BEGIN );
		AddTouchHandler( this, &GameStateLevel::TouchMove,   TouchEvent::TOUCH_MOVE );
		AddTouchHandler( this, &GameStateLevel::TouchEnd,    TouchEvent::TOUCH_END );
		AddTouchHandler( this, &GameStateLevel::TouchCancel, TouchEvent::TOUCH_CANCEL );
		AddGamepadHandlerFromConfig( "Pad_Clock",   this, &GameStateLevel::PadTurnDown );
		AddGamepadHandlerFromConfig( "Pad_Counter", this, &GameStateLevel::PadTurnUp );
		AddGamepadHandlerFromConfig( "Pad_Accel",   this, &GameStateLevel::PadAccel );
		AddGamepadHandlerFromConfig( "Pad_Fire",    this, &GameStateLevel::PadFireGun );
		AddGamepadHandlerFromConfig( "Pad_Bomb",    this, &GameStateLevel::PadFireBomb );

		AddKeyboardHandler( this, &GameStateLevel::FireGun,  KeyboardEvent::KBD_KEY_PRESSED, keyFire    ? keyFire    : ALLEGRO_KEY_Q );
		AddKeyboardHandler( this, &GameStateLevel::FireGunRelease,  KeyboardEvent::KBD_KEY_UP, keyFire    ? keyFire    : ALLEGRO_KEY_Q );

		AddKeyboardHandler( this, &GameStateLevel::FireBomb, KeyboardEvent::KBD_KEY_DOWN,    keyBomb    ? keyBomb    : ALLEGRO_KEY_A );

		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType0, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_0 );
		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType1, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_1 );
		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType2, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_2 );
		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType3, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_3 );
		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType4, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_4 );
		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType5, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_5 );
		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType6, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_6 );
		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType7, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_7 );
		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType8, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_8 );
		AddKeyboardHandler( this, &GameStateLevel::DebugSetBulletType9, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_9 );

		m_optionsVersion = g_options_version;
	}

	context.UpdateDayTime( dt );
	if ( context.GetDaysLeft() <= 0 )
	{
		m_gameOverTimer = GAME_OVER_TIME;
		m_gameOverType = GAMEOVER_TYPE_TIME;
		return;
	}

	m_skyController.Update( context.GetDayTime() );

	context.GetLevelDirector().Update( dt, context );

	  // Create render components for drawing background, foreground, reflections etc.
	  
	if ( !m_beforeComp.IsValid() )
	{
		m_beforeComp = context.CreateGraphCompImmediate();
		assert( m_beforeComp.IsValid() );
		GraphicComponent *pBefore = context.GetOjectsStorage().GetGraphicComponent( m_beforeComp );
		assert( pBefore );
		pBefore->SetLayer( RL_BG_BEFORE );
		pBefore->m_renderProc.bind( &m_refController, &ReflectionController::RenderBefore );
	}

	if ( !m_afterComp.IsValid() )
	{
		m_afterComp = context.CreateGraphCompImmediate();
		assert( m_beforeComp.IsValid() );
		GraphicComponent *pAfter = context.GetOjectsStorage().GetGraphicComponent( m_afterComp );
		assert( pAfter );
		pAfter->SetLayer( RL_BG_AFTER );
		pAfter->m_renderProc.bind( &m_refController, &ReflectionController::RenderAfter );
	}

	if ( !m_groundCompBg.IsValid() )
	{
		m_groundCompBg = context.CreateGraphCompImmediate();
		assert( m_groundCompBg.IsValid() );
		GraphicComponent *pGround = context.GetOjectsStorage().GetGraphicComponent( m_groundCompBg );
		assert( pGround );
		pGround->SetLayer( RL_BACKGROUND );
		pGround->m_renderProc.bind( &m_groundController, &GroundController::RenderBG );
	}

	if ( !m_groundCompMg.IsValid() )
	{
		m_groundCompMg = context.CreateGraphCompImmediate();
		assert( m_groundCompMg.IsValid() );
		GraphicComponent *pGround = context.GetOjectsStorage().GetGraphicComponent( m_groundCompMg );
		assert( pGround );
		pGround->SetLayer( RL_MIDGROUND );
		pGround->m_renderProc.bind( &m_groundController, &GroundController::RenderMG );
	}

	if ( !m_groundCompFg.IsValid() )
	{
		m_groundCompFg = context.CreateGraphCompImmediate();
		assert( m_groundCompFg.IsValid() );
		GraphicComponent *pGround = context.GetOjectsStorage().GetGraphicComponent( m_groundCompFg );
		assert( pGround );
		pGround->SetLayer( RL_FOREGROUND );
		pGround->m_renderProc.bind( &m_groundController, &GroundController::RenderFG );
	}

	if ( m_exitGame )
	{
		context.GetOjectsStorage().GetPlayer().m_score = 0;
		context.RemoveState( GAMESTATE_LEVEL );
		context.PushState( GAMESTATE_MENU );
		m_exitGame = false;
		return;
	}

	m_gamePaused = false;
	if ( m_pauseGame )
	{
		context.PushState( GAMESTATE_PAUSE );
		m_pauseGame = false;
		m_gamePaused = true;
	}

	  // If we have requested player object to be created
	if ( m_playerObjectRequestID.IsValid() )
	{
		  // Get result. If object was not created - crash, resources are corrupted beyond imagination
		m_playerObjectID = context.GetCreateRequestResult( m_playerObjectRequestID );
		if ( !m_playerObjectID.IsValid() )
		{
			ALLEGRO_ASSERT( false );
		}

		  // Clear request
		m_playerObjectRequestID.Invalidate();

		  // Set after-respawn invulnerability
		const GameObject *pObject = context.GetOjectsStorage().GetGameObject( m_playerObjectID );
		if ( pObject )
		{
			MechanicComponent *pMech = context.GetOjectsStorage().GetMechanicComponent( pObject->GetMechanicComponent() );
			if ( pMech && pMech->m_type == GM_PLAYER )
				pMech->m_invulTimer = INVUL_TIMER;
		}
	}

	  // If player's object is dead and we're not going to game over screen, request it to be spawned
	if ( !m_playerObjectID.IsValid() && m_gameOverTimer <= 0.0f && !m_lifeLossEffect.IsActive() )
	{
		SCreateObjectRequest::SData data;
		data.x = -49;
		data.y = 512;
		m_playerObjectRequestID = context.CreateGameObject( "player_plane", data );
	}
	  
	GameObject *pPlayerObject = context.GetOjectsStorage().GetGameObject( m_playerObjectID );
	if ( pPlayerObject )
	{
		MechanicComponent *pMech = context.GetOjectsStorage().GetMechanicComponent( pPlayerObject->GetMechanicComponent() );
		ControlComponent *pControl = context.GetOjectsStorage().GetControlComponent( pPlayerObject->GetControlComponent() );
		PhysicComponent *pPhys = context.GetOjectsStorage().GetPhysicComponent( pPlayerObject->GetPhysicComponent() );
		GraphicComponent *pGraph = context.GetOjectsStorage().GetGraphicComponent( pPlayerObject->GetGraphicComponent() );

		m_touchControls &= ~CONTROL_TURN_UP;
		m_touchControls &= ~CONTROL_TURN_DOWN;
		if ( m_touchControlsValue > 0.1f )
		{
			m_touchControls |= CONTROL_TURN_DOWN;
		}
		else if ( m_touchControlsValue < -0.1f )
		{
			m_touchControls |= CONTROL_TURN_UP;
		}

		if ( pMech )
		{
			pMech->UpdateStall( dt, ( m_controls | m_touchControls | m_padControls ) != 0 );
			if ( pMech->m_invulTimer >= 0 )
			{
				pMech->m_invulTimer -= dt;
				if ( pGraph && GetSystem().GetConfigValue_Int( "DD_Gameplay", "DisableTutorial" ) != 0 )
					pGraph->m_invulnerabile = true;
			}
			else
			{
				if ( pGraph )
					pGraph->m_invulnerabile = false;
			}
		}
		if ( ( !pMech || !pMech->IsStalledPhase1() ) && ( !m_tutorialEnabled || m_tutorialTimer <= 0.0f ) )
			pControl->m_controls = ( m_controls | m_touchControls | m_padControls );

		m_controls = 0;
		m_touchControls &= ~CONTROL_FIRE1;
		
		m_touchControlsValue *= 0.5f;
		if ( fabsf( m_touchControlsValue ) < 1.0f )
			m_touchControlsValue = 0.0f;

		if ( pPhys )
		{
			PlayerProfile & prof = context.GetPlayerProfile();
			if ( pPhys->m_y > SCREEN_H / 2 && pMech && pMech->m_type == GM_PLAYER )
			{
				if ( prof.m_highFlyerTimer < 5.0f )
					prof.m_highFlyerTimer += dt;
			}
			else
			{
				if ( prof.m_highFlyerTimer < 5.0f )
					prof.m_highFlyerTimer = 0;
			}
		}
	}
	else
	{
		m_playerObjectID = GameObjectID();
	}

	ObjectsStorage::GameObjectContainer & gameObjects = context.GetOjectsStorage().GetGameObjects();
	for ( ObjectsStorage::GameObjectContainer::Iterator iter = gameObjects.GetIterator(); !iter.AtEnd(); ++iter )
	{
		GameObject *pObject = *iter;
		ControlsToPhysics( pObject, context, dt );
		PhysicsToGraphics( pObject, context, dt );
		PhysicsToSound( pObject, context, dt );
		PhysicsToAI( pObject, context, dt );		
		ControlsToMechanic( pObject, context, dt );
		GraphicsToSound( pObject, context, dt );
	}

	ProcessCollisions( context );
}

bool GameStateLevel::ExitGame( const InputEvent & e)
{
	m_exitGame = true;
	return true;
}

bool GameStateLevel::PauseGame( const InputEvent & e)
{
	if ( m_gamePaused )
		return true;

	m_pauseGame = true;
	return true;
}

bool GameStateLevel::TurnUp( const InputEvent & e)
{
	m_controls |= CONTROL_TURN_UP;
	return true;
}

bool GameStateLevel::TurnDown( const InputEvent & e)
{
	m_controls |= CONTROL_TURN_DOWN;
	return true;
}

bool GameStateLevel::SpeedUp( const InputEvent & e)
{
	m_controls |= CONTROL_SPEED_UP;
	return true;
}

bool GameStateLevel::FireGun( const InputEvent & e)
{
	m_controls |= CONTROL_FIRE1;
	m_lifeLossEffect.OnFire();
	return true;
}

bool GameStateLevel::FireGunRelease( const InputEvent & e )
{
	m_controls &= ~CONTROL_FIRE1;
	m_resetWeaponCooldown = true;
	return true;
}

bool GameStateLevel::FireBomb( const InputEvent & e)
{
	m_controls |= CONTROL_FIRE2;
	return true;
}

bool GameStateLevel::TouchBegin( const InputEvent & e)
{
	STouch *pTouch = 0;

	if ( m_touch1.id < 0 )
		pTouch = &m_touch1;
	else if ( m_touch2.id < 0 )
		pTouch = &m_touch2;
	else
		return true;

	pTouch->id = e.m_touchEvent.m_id;
	pTouch->time = e.m_touchEvent.m_timestamp;
	pTouch->sx = e.m_touchEvent.m_x;
	pTouch->sy = e.m_touchEvent.m_y;
	return true;
}

#include "Engine/ILogger.h"
#include "LogCommon.h"
bool GameStateLevel::TouchMove( const InputEvent & e)
{
	GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroInput: event=%i id=%i x=%i y=%i dx=%i dy=%i", e.m_touchEvent.m_type, e.m_touchEvent.m_id, e.m_touchEvent.m_x, e.m_touchEvent.m_y, e.m_touchEvent.m_dx, e.m_touchEvent.m_dy );

	if ( e.m_touchEvent.m_id == m_touch1.id )
	{
		m_touchControls &= ~CONTROL_TURN_UP;
		m_touchControls &= ~CONTROL_TURN_DOWN;

		if ( abs(e.m_touchEvent.m_dx) > fabsf(m_touchControlsValue) )
		{
			//GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroInput: move! dx=%i m_touchControlsValue=%f", e.m_touchEvent.m_dx, m_touchControlsValue );
			m_touchControlsValue = e.m_touchEvent.m_dx;
		}

		//if ( e.m_touchEvent.m_x - m_touch1.sx > 1 )
		//{
		//	m_touchControls |= CONTROL_TURN_DOWN;
		//}
		//else if ( e.m_touchEvent.m_x - m_touch1.sx < -1 )
		//{
		//	m_touchControls |= CONTROL_TURN_UP;
		//}	
	}

	return true;
}

bool GameStateLevel::TouchEnd( const InputEvent & e)
{
	if ( e.m_touchEvent.m_id == m_touch1.id )
	{
		if ( e.m_touchEvent.m_timestamp - m_touch1.time < 1 )
		{
			m_touchControls |= CONTROL_FIRE1;
			m_lifeLossEffect.OnFire();
		}

		m_touchControls &= ~CONTROL_TURN_UP;
		m_touchControls &= ~CONTROL_TURN_DOWN;

		m_touch1 = m_touch2;
		m_touch2 = STouch();
	}
	else if ( e.m_touchEvent.m_id == m_touch2.id )
	{
		if ( e.m_touchEvent.m_timestamp - m_touch2.time < 1 )
		{
			m_touchControls |= CONTROL_FIRE1;
			m_lifeLossEffect.OnFire();
		}

		m_touch2 = STouch();
	}

	return true;
}

bool GameStateLevel::TouchCancel( const InputEvent & e)
{
	if ( e.m_touchEvent.m_id == m_touch1.id )
	{
		if ( e.m_touchEvent.m_timestamp - m_touch1.time < 200 )
		{
			m_touchControls |= CONTROL_FIRE1;
			m_lifeLossEffect.OnFire();
		}

		m_touchControls &= ~CONTROL_TURN_UP;
		m_touchControls &= ~CONTROL_TURN_DOWN;

		m_touch1 = m_touch2;
		m_touch2 = STouch();
	}
	else if ( e.m_touchEvent.m_id == m_touch2.id )
	{
		if ( e.m_touchEvent.m_timestamp - m_touch2.time < 200 )
		{
			m_touchControls |= CONTROL_FIRE1;
			m_lifeLossEffect.OnFire();
		}

		m_touch2 = STouch();
	}

	return true;
}

bool GameStateLevel::PadTurnUp( const InputEvent & e )
{
	m_padControls &= ~CONTROL_TURN_UP;
	m_padControls &= ~CONTROL_TURN_DOWN;

	if ( fabs( e.m_gamepadEvent.m_pos ) > 0.00001f || e.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_DOWN )
		m_padControls |= CONTROL_TURN_UP;

	return true;
}

bool GameStateLevel::PadTurnDown( const InputEvent & e )
{
	m_padControls &= ~CONTROL_TURN_UP;
	m_padControls &= ~CONTROL_TURN_DOWN;

	if ( fabs( e.m_gamepadEvent.m_pos ) > 0.00001f || e.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_DOWN )
		m_padControls |= CONTROL_TURN_DOWN;

	return true;
}

bool GameStateLevel::PadAccel( const InputEvent & e )
{
	m_padControls &= ~CONTROL_SPEED_UP;

	if ( fabs( e.m_gamepadEvent.m_pos ) > 0.00001f || e.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_DOWN )
		m_padControls |= CONTROL_SPEED_UP;

	return true;
}

bool GameStateLevel::PadFireGun( const InputEvent & e )
{
	m_padControls &= ~CONTROL_FIRE1;

	if ( fabs( e.m_gamepadEvent.m_pos ) > 0.00001f || e.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_DOWN )
	{
		m_padControls |= CONTROL_FIRE1;
		m_lifeLossEffect.OnFire();
	}

	return true;
}

bool GameStateLevel::PadFireBomb( const InputEvent & e )
{
	m_padControls &= ~CONTROL_FIRE2;

	if ( fabs( e.m_gamepadEvent.m_pos ) > 0.00001f || e.m_gamepadEvent.m_type == GamepadEvent::GP_BUTTON_DOWN )
		m_padControls |= CONTROL_FIRE2;

	return true;
}

void GameStateLevel::OnPush( IGameContext & context ) 
{
	context.GetOjectsStorage().GetMusic().ReceiveEvent( "LEVEL" );
	m_fade = true;
	m_fadeTimer = FADE_TIME;
	m_exitGame = false;
	m_pauseGame = false;
	m_controls = 0;
	m_touchControls = 0;
	m_touchControlsNum = 0;
	m_touchControlsValue = 0;	
	m_padControls = 0;
	m_resetWeaponCooldown = false;

	m_gamePaused = false;

	m_playerObjectID = GameObjectID();
	m_playerObjectRequestID = RequestID();

	m_pbp.Reset();
	m_lifeLossEffect.Stop();

	m_optionsVersion = -1;

	m_refController.Setup( context.GetLevelDirector().GetReflectionControllerParams() );
	m_groundController.Setup( context.GetLevelDirector().GetGroundControllerParams() );
	m_skyController.Setup( context.GetLevelDirector().GetSkyControllerParams() );

	m_gameOverTimer = 0;
	m_gameOverType = GAMEOVER_TYPE_NONE ;

	m_levelTimer = 0.0f;
	m_tutorialTimer = 1.2f;
	m_tutorialEnabled = ( GetSystem().GetConfigValue_Int( "DD_Gameplay", "DisableTutorial" ) == 0 );

	m_ui.Init( GetRender() );
	m_pbp.Init( GetRender() );	

	if ( !m_damageImage.IsValid() )
	{
		float radius = 20;
		const int MAX_ADD_ALL = 1 + (1.0f + 1.0f)/2.0f * 15;
		const float maxRadius = radius + MAX_ADD_ALL;

		IRender & render = GetRender();
		m_damageImage = render.CreateImage( maxRadius*2, maxRadius*2, false, "damage_image" );
		m_damageMap.Create( maxRadius*2, maxRadius*2 );

		render.ClearImage( m_damageImage, Color( 0, 0, 0, 0 ) );

		int a, b, c;
		al_get_blender( &a, &b, &c );
		al_set_blender( ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO );

		render.LockImage( m_damageImage, 0, 0, maxRadius*2, maxRadius*2 );

		const int cx = maxRadius;
		const int cy = maxRadius;

		  // First, we destroy a round chunk of target while updating its mask and counting destroyed waterline pixels
		for ( int x = -radius; x < radius; ++x )
		{
		for ( int y = -radius; y < radius; ++y )
		{
				if ( x*x + y*y <= radius*radius )
				{
					const int tx = x+cx;
					const int ty = y+cy;

					m_damageMap.Set( tx, ty, true );
					render.DrawPixel( m_damageImage, tx, ty, Color(255,255,255,255) );
				}
			}
		}
		
		  // Then, we add uneven edges to destroyed chunk
		for ( float angle = 0.0f; angle < 2*ALLEGRO_PI; angle += 0.01f )
		{
			const int MAX_ADD = 1 + (sinf(angle) + 1.0f)/2.0f * 15;
			const int add = RndInt( 0, MAX_ADD );
			for ( int r = radius; r < radius + add; ++r )
			{
				float x = r * cosf(angle);
				float y = r * sinf(angle);
				const int tx = x+cx;
				const int ty = y+cy;

				m_damageMap.Set( tx, ty, true );
				render.DrawPixel( m_damageImage, tx, ty, Color(255,255,255,255) );
			}		
		}

		GetRender().UnlockImage( m_damageImage );
		al_set_blender( a, b, c );
	}
}

void GameStateLevel::OnRemove( IGameContext & context )
{
	context.RemoveGraphCompImmediate( m_beforeComp );
	context.RemoveGraphCompImmediate( m_afterComp );
	context.RemoveGraphCompImmediate( m_groundCompBg );
	context.RemoveGraphCompImmediate( m_groundCompMg );
	context.RemoveGraphCompImmediate( m_groundCompFg );
	m_beforeComp = GraphicCompID();
	m_afterComp = GraphicCompID();
	m_groundCompBg = GraphicCompID();
	m_groundCompMg = GraphicCompID();
	m_groundCompFg = GraphicCompID();

	context.DestroyAllObjects();
}

void GameStateLevel::OnDisplayFound( IGameContext & context )
{
	GameObjectID targetID = context.GetLevelDirector().GetTargetObjectID();
	const GameObject *pObject = context.GetOjectsStorage().GetGameObject( targetID );
	if ( !pObject )
		return;

	GraphicComponent *pGrComp = context.GetOjectsStorage().GetGraphicComponent( pObject->GetGraphicComponent() );
	if ( !pGrComp )
		return;

	int a,b,c;
	al_get_blender( &a, &b, &c );
	al_set_blender( ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ONE, ALLEGRO_ONE );

	pGrComp->m_image = GetRender().CloneImage( pGrComp->m_cache );
	for ( unsigned int i = 0; i < m_damageCache.size(); ++i )
	{
		const SDamageCache & c = m_damageCache[i];
		GetRender().DrawImage( pGrComp->m_image, m_damageImage, c.x, c.y );
	}

	al_set_blender( a, b, c );
}

void UIRect::Init( IRender & render )
{
	m_tl	 = GetRender().LoadImage( "Data/Sprites2/ui/corner_left_top_mini.png" );
	m_tr	 = GetRender().LoadImage( "Data/Sprites2/ui/corner_right_top_mini.png" );
	m_bl	 = GetRender().LoadImage( "Data/Sprites2/ui/corner_left_bottom.png" );
	m_br	 = GetRender().LoadImage( "Data/Sprites2/ui/corner_right_bottom.png" );
	m_left	 = GetRender().LoadImage( "Data/Sprites2/ui/bordr_left.png" );
	m_right	 = GetRender().LoadImage( "Data/Sprites2/ui/bordr_rght.png" );
	m_top	 = GetRender().LoadImage( "Data/Sprites2/ui/bordr_top.png" );
	m_bottom = GetRender().LoadImage( "Data/Sprites2/ui/bordr_bottom.png" );
}

void UIRect::Render( IRender & render )
{
	const int x1 = m_x;
	const int x2 = m_x + m_w;
	const int y1 = m_y;
	const int y2 = m_y + m_h;

	static const int VSIDE_LEN = m_h - 128 - 6;
	static const int TOP_LEN = m_w - 6 - 6;
	static const int BOT_LEN = m_w - 128 - 128;

	render.DrawRectFill( x1, y1, x2, y2, Color( 0, 0, 0, 127 ) );

	render.DrawImage( m_tl, x1, y1 );
	render.DrawImage( m_tr, x2 - 6, y1 );
	render.DrawImage( m_bl, x1, y2 - 128 );
	render.DrawImage( m_br, x2 - 128, y2 - 128 );

	render.CopyRegion( m_left,   x1,       y1 + 6,   0, 0, 16, 16, 16, VSIDE_LEN );
	render.CopyRegion( m_right,  x2 - 16,  y1 + 6,   0, 0, 16, 16, 16, VSIDE_LEN );
	render.CopyRegion( m_top,    x1 + 6,   y1,       0, 0, 16, 16, TOP_LEN, 16 );
	render.CopyRegion( m_bottom, x1 + 128, y2 - 16,  0, 0, 16, 16, BOT_LEN, 16 );
}

void UIRect::Render( IRender & render, ImageHandle target )
{
	const int x1 = m_x;
	const int x2 = m_x + m_w;
	const int y1 = m_y;
	const int y2 = m_y + m_h;

	static const int VSIDE_LEN = m_h - 128 - 6;
	static const int TOP_LEN = m_w - 6 - 6;
	static const int BOT_LEN = m_w - 128 - 128;

	render.DrawRectFill( target, x1, y1, x2, y2, Color( 0, 0, 0, 127 ) );

	render.DrawImage( target, m_tl, x1, y1 );
	render.DrawImage( target, m_tr, x2 - 6, y1 );
	render.DrawImage( target, m_bl, x1, y2 - 128 );
	render.DrawImage( target, m_br, x2 - 128, y2 - 128 );

	render.CopyRegion( target, m_left, x1, y1 + 6, 0, 0, 16, 16, 16, VSIDE_LEN );
	render.CopyRegion( target, m_right, x2 - 16, y1 + 6, 0, 0, 16, 16, 16, VSIDE_LEN );
	render.CopyRegion( target, m_top, x1 + 6, y1, 0, 0, 16, 16, TOP_LEN, 16 );
	render.CopyRegion( target, m_bottom, 128, y2 - 16, 0, 0, 16, 16, BOT_LEN, 16 );
}

void UIRect::SetRect( int x, int y, int w, int h )
{
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
}

void PlayByPlay::Init( IRender & render )
{
	m_useOpacity = false;
	m_rect.Init( render );
	m_rect.SetRect( 0, 0, WIDTH, HEIGHT );
}

void PlayByPlay::UpdatePBPIndex( int index )
{
	if ( m_pbpIndex != index )
	{
		m_pbpIndex = index;
		m_pbpUpdated = true;
		m_pauseAfter = false;
	}
}

void PlayByPlay::Update( IGameContext & context, float dt )
{
	if ( m_opacityTimer > 0 )
		m_opacityTimer -= dt;

	if ( m_opacityTimer <= 0.0f )
		m_useOpacity = false;

	const float prevTimer = m_timer;
	m_timer -= dt;
	if ( m_timer < 0 && m_pauseAfter )
	{
		m_textPrinter.SetText( UTFString() );
	}

	if ( m_pauseAfter && prevTimer > 0.5f && m_timer <= 0.5f )
	{
		m_opacityTimer = 0.5f;
		m_useOpacity = true;
	}

	if ( m_pbpUpdated )
	{
		const SPlayByPlayEntry *pPBP = context.GetLevelDirector().GetPBP( m_pbpIndex );
		if ( pPBP )
		{
			m_currentActor = pPBP->m_image;
			const Text *pText = GetTextManager().GetResource( pPBP->m_text );
			if ( pText )
				m_textPrinter.SetText( pText->m_text );
			m_textPrinter.SetTextParams( 25 );
			
			const int TEXT_X = 140 + 6 + 10;
			const int TEXT_Y = 10;
			const int TEXT_W = WIDTH - TEXT_X - 10;
			const int TEXT_H = HEIGHT - 20;
			
			m_textPrinter.SetRegion( TEXT_X, TEXT_Y, TEXT_W, TEXT_H );

			m_opacityTimer = 0.5f;
			m_useOpacity = true;
		}
		else
		{
			m_textPrinter.SetText( UTFString() );
			m_currentActor = ImageHandle();
		}

		m_pbpUpdated = false;
	}
}

void PlayByPlay::Render( IRender & render )
{
	if ( m_timer <= 0.0f )
	{
		if ( !m_image.IsValid() )
			m_image = render.CreateImage( WIDTH, HEIGHT, true, "pbp" );

		if ( m_currentActor.IsValid() )
		{
			render.ClearImage( m_image, Color(0,0,0,0) );
			
			m_rect.Render( render, m_image );

			render.DrawImage( m_image, m_currentActor, 10, 10 );
			
			m_currentActor = ImageHandle();
		}

		m_timer = 0.03f;

		if ( m_textPrinter.HasText() )
		{
			if ( !m_textPrinter.Update( m_image, render ) )
			{
				m_pauseAfter = true;
				m_timer = 5.0f;
			}
		}
	}

	if ( m_textPrinter.HasText() )
	{
		if ( m_useOpacity )
		{
			const float t = m_opacityTimer / 0.5f;
			const float opacity = m_pauseAfter ? t : 1.0f - t;

			int a, b, c;
			al_get_blender( &a, &b, &c );
			al_set_blender( ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA );
			render.DrawTintedImage( m_image, Color( 255, 255, 255, 255*opacity ), SCREEN_W / 2 - WIDTH / 2, SCREEN_H - HEIGHT - 50 );
			al_set_blender( a, b, c );
		}
		else
			render.DrawImage( m_image, SCREEN_W / 2 - WIDTH / 2, SCREEN_H - HEIGHT - 50 );
	}
}

bool PlayByPlay::IsActive()
{
	return m_textPrinter.HasText();
}

void CombatUI::Init( IRender & render )
{
	m_e1 = render.LoadImage( "Data/Sprites2/ui/1_elmnt.png" );
	m_e2 = render.LoadImage( "Data/Sprites2/ui/2_elmnt.png" );
	m_e3 = render.LoadImage( "Data/Sprites2/ui/3_elmnt.png" );
	m_e4 = render.LoadImage( "Data/Sprites2/ui/4_elmnt.png" );
	m_bomb = render.LoadImage( "Data/Sprites2/ui/bomb.png" );

	render.DrawText( 0, 0, Color(0,0,0), 35, "%s", "1234567890" );
}

void CombatUI::Render( const IGameContext & context, GameObjectID playerObjectID, IRender & render )
{
	const IImage *pImg = render.GetImageByHandle( m_e1 );
	if ( !pImg )
		return;

	const int gy = SCREEN_H - pImg->GetHeight();

	const GameObject *pPlayerObject = context.GetOjectsStorage().GetGameObject( playerObjectID );
	const MechanicComponent *pGM = pPlayerObject ? context.GetOjectsStorage().GetMechanicComponent( pPlayerObject->GetMechanicComponent() ) : 0;

	if ( pGM && pGM->m_weapons[1].m_currentAmmo > 0 )
		render.DrawImage( m_bomb, 35, SCREEN_H - 82 );

	render.DrawImage( m_e1, 256 * 0, gy );
	render.DrawImage( m_e2, 256 * 1, gy );
	render.DrawImage( m_e3, 256 * 2, gy );
	render.DrawImage( m_e4, 256 * 3, gy );

	const Player & player = context.GetOjectsStorage().GetPlayer();

	Color numValue  = Color( 246, 129, 20 );
	Color label     = Color( 222, 222, 222 );
	Color levelName = Color( 179, 179, 179 );
	const int fontSize = 35;
	const int ty = 726;

	render.DrawText( 142, ty, numValue, fontSize, "%i", player.m_score );
	render.DrawText( 321, ty, label,    fontSize, "Days:" );
	render.DrawText( 400, ty, numValue, fontSize, "%i", context.GetDaysLeft() );
	render.DrawText( 529, ty, numValue, fontSize, "%i", player.m_lives );

	int a,b,c;
	al_get_blender( &a, &b, &c );
	al_set_blender( ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA );
	render.DrawRectFill( 579, 728, 579 + ( 312 * context.GetLevelDirector().GetCompletionPercent() ), 728 + 35, Color(255,0,0, 60) );
	al_set_blender( a, b, c );

	render.DrawAlignedText( 744, ty, levelName, fontSize, TEXT_ALIGN_CENTER, "%s", context.GetLevelDirector().GetName().c_str() );
}

bool LifeLossEffect::IsActive() const
{
	return m_state != STATE_INACTIVE;
}

void LifeLossEffect::OnFire()
{
	if ( m_state == STATE_WAIT )
	{
		m_state = STATE_FADE_OUT;
		m_pauseTimerInit = 1.0f;
		m_pauseTimer = m_pauseTimerInit;			
	}
}

void LifeLossEffect::Restart( ImageHandle enemyImage,  const SAnimFrame *pFrame )
{
	m_state = STATE_CRACK;

	m_lines.clear();

	for ( int i = 0; i < 50; ++i )
	{
		SLine line;
		line.m_lifetime = ( rand() % 15 ) + 1;
		line.m_x = SCREEN_W / 2;
		line.m_y = SCREEN_H / 2;
		line.m_angle = ( rand() / (float)RAND_MAX ) * ALLEGRO_PI * 2.0f;
		m_lines.push_back( line );
	}

	IRender & render = GetRender();

	if ( !m_image.IsValid() )
	{
		m_image = render.CreateImage( SCREEN_W, SCREEN_H, "cracks" );
		m_textImage = render.CreateImage( 400, 160, "cracks_text" );
	}

	render.ClearImage( m_image, Color(0,0,0,0) );
	render.ClearImage( m_textImage, Color(0,0,0,0) );

	m_textRect.Init( GetRender() );
	m_textRect.SetRect( 0, 0, 400, 160 );
	m_textRect.Render( render, m_textImage );

	if ( pFrame )
		render.CopyRegion( m_textImage, enemyImage, 400/2 - pFrame->w/2, 160/2 - pFrame->h/2, pFrame->x, pFrame->y, pFrame->w, pFrame->h, pFrame->w, pFrame->h );
	else
	{
		const IImage *pImage = render.GetImageByHandle( enemyImage );
		render.DrawImage( m_textImage, enemyImage, 400/2 - pImage->GetWidth()/2, 160/2 - pImage->GetHeight()/2 );
	}

	m_pauseTimer = 0.0f;
	m_pauseTimerInit = 0.1f;		
}

void LifeLossEffect::Stop()
{
	m_state = STATE_INACTIVE;
}

void LifeLossEffect::Update( float dt )
{
	if ( m_state == STATE_INACTIVE )
		return;

	switch( m_state )
	{
	case STATE_CRACK:
		m_pauseTimer -= dt;
		break;

	case STATE_TEXT:
		m_pauseTimer -= dt;
		if ( m_pauseTimer <= 0 )
		{
			m_state = STATE_WAIT;
			m_blinkTimer = 0.0f;
		}
		break;

	case STATE_FADE_OUT:
		m_pauseTimer -= dt;
		if ( m_pauseTimer <= 0 )
			m_state = STATE_INACTIVE;
		break;

	case STATE_WAIT:
		m_blinkTimer += dt;
		break;
	}
}

void LifeLossEffect::RenderCracks( IRender & render )
{
	if ( m_pauseTimer > 0 )
		return;

	bool hasMoreCracksToRender = false;

	m_pauseTimer = m_pauseTimerInit;
	m_pauseTimerInit *= 0.75f;

	for ( int i = 0; i < (int)m_lines.size(); ++i )
	{
		SLine & line = m_lines[i];
		if ( line.m_lifetime <= 0 )
			continue;

		hasMoreCracksToRender = true;

		line.m_lifetime -= 1;
		const int length = ( rand() % 70 ) + 30;
		const int dx = length * cosf( line.m_angle );
		const int dy = length * sinf( line.m_angle );
		const int thikness = (line.m_lifetime / 10.0f) * 3.0f + 1;

		render.DrawLine( m_image, line.m_x, line.m_y, line.m_x + dx, line.m_y + dy, Color(200,200,200), thikness );				

		line.m_x += dx;
		line.m_y += dy;
		//const float maxDiff = ALLEGRO_PI / 30.0f;
		const float maxDiff = ALLEGRO_PI / 3.0f;
		line.m_angle += (rand() / (float)RAND_MAX) * maxDiff * 2 - maxDiff;
	}

	if ( !hasMoreCracksToRender )
	{
		m_state = STATE_TEXT;
		m_pauseTimerInit = 1.0f;
		m_pauseTimer = m_pauseTimerInit;
	}
}

void LifeLossEffect::RenderText( IRender & render )
{	
	int dx = 0;
	if ( m_state == STATE_TEXT )
	{
		const  float t = m_pauseTimer / m_pauseTimerInit;
		dx = sinf( t * ALLEGRO_PI / 2.0f ) * ( SCREEN_W / 2 + 200 );
	}

	int alpha  = 255;
	int a, b, c;
	if ( m_state == STATE_FADE_OUT )
	{
		alpha = m_pauseTimer / m_pauseTimerInit * 255;
		const int alpha = (m_pauseTimer / m_pauseTimerInit) * 255;			
		al_get_blender( &a, &b, &c );
		al_set_blender( ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA );
	}

	render.DrawTintedImage( m_textImage, Color(255,255,255,alpha), SCREEN_W/2 - 200 + dx, SCREEN_H/2 - 90 );

	if ( m_state >= STATE_TEXT )
		render.DrawAlignedText( SCREEN_W/2 + dx, SCREEN_H/2 - 70, Color(255,0,0, alpha), 40, TEXT_ALIGN_CENTER, "You have crashed into:" );
	if ( m_state >= STATE_WAIT )
	{
		const int c = (int)(( sinf( m_blinkTimer * ALLEGRO_PI * 2.0f / 2.0f ) + 1.0f ) / 2.0f * 255);
		render.DrawAlignedText( SCREEN_W/2, SCREEN_H/2 + 20, Color(c,c,c, alpha), 20, TEXT_ALIGN_CENTER, "Press fire to continue" );		
	}

	if ( m_state == STATE_FADE_OUT )
		al_set_blender( a, b, c );
}

void LifeLossEffect::Render( IRender & render )
{
	if ( m_state == STATE_INACTIVE )
		return;

	if ( m_state == STATE_CRACK )
		RenderCracks( render );

	if ( m_state == STATE_FADE_OUT )
	{
		const int alpha = (m_pauseTimer / m_pauseTimerInit) * 255;
		int a, b, c;
		al_get_blender( &a, &b, &c );
		al_set_blender( ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA );
		render.DrawTintedImage( m_image, Color(255,255,255,alpha), 0, 0 );
		al_set_blender( a, b, c );
	}
	else
		render.DrawImage( m_image, 0, 0 );

	if ( m_state >= STATE_TEXT )
		RenderText( render );
}
