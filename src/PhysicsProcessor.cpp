#include "stdafx.h"
#include "PhysicsProcessor.h"
#include "PhysicsContainer.h"
#include "CollisionData.h"
#include "SweepSpheres.h"
#include "Vector.h"
#include "Vars.h"
#include "Common.h"
#include "IRender.h"
#include "GameContext.h"

namespace
{
RegisterIntVar PHYS_CAPS_FOLLOW_ANGLE("PHYS_CAPS_FOLLOW_ANGLE");

RegisterIntVar COLLISIONS_PLAYER("COLLISIONS_PLAYER");
RegisterIntVar COLLISIONS_PLAYER_BULLET("COLLISIONS_PLAYER_BULLET");
RegisterIntVar COLLISIONS_ENEMY("COLLISIONS_ENEMY");
RegisterIntVar COLLISIONS_ENEMY_HARMLESS("COLLISIONS_ENEMY_HARMLESS");
RegisterIntVar COLLISIONS_ENEMY_INVUL("COLLISIONS_ENEMY_INVUL");
RegisterIntVar COLLISIONS_ENEMY_BULLET("COLLISIONS_ENEMY_BULLET");
RegisterIntVar COLLISIONS_ENEMY_SHOT_DOWN("COLLISIONS_ENEMY_SHOT_DOWN");
RegisterIntVar COLLISIONS_ENEMY_HARMLESS_SHOT_DOWN("COLLISIONS_ENEMY_HARMLESS_SHOT_DOWN");
RegisterIntVar COLLISIONS_PLAYER_SHOT_DOWN("COLLISIONS_PLAYER_SHOT_DOWN");
RegisterIntVar COLLISIONS_TARGET("COLLISIONS_TARGET");
}

void PhysicsProcessor::Update( float dt, PhysicComponent & comp, CollisionData & collisions )
{
	comp.m_prevX = comp.m_x;
	comp.m_prevY = comp.m_y;

	  // For cases where speed is set by current angle (player's plane)
	if ( comp.m_caps & PHYS_CAPS_FOLLOW_ANGLE )
	{
		const float cosa = cosf( comp.m_angle );
		const float sina = sinf( comp.m_angle );
		comp.m_x += comp.m_speed*cosa*dt;
		comp.m_y += comp.m_speed*sina*dt;
	}
	else // Usual cases
	{
		comp.m_x += comp.m_speedX * dt;
		comp.m_y += comp.m_speedY * dt;
		comp.m_angle = atan2f( comp.m_speedY, comp.m_speedX );
	}

	  // Calculate mask of border collisions
	int borderCollisions = 0;

	if ( comp.m_x < -50 )
	{
		borderCollisions |= BORDER_LEFT;
		comp.m_x = SCREEN_W + 50;
		comp.m_prevX = comp.m_x;
	}

	if ( comp.m_x > SCREEN_W + 50 )
	{
		borderCollisions |= BORDER_RIGHT;
		comp.m_x = -50;
		comp.m_prevX = comp.m_x;
	}

	if ( comp.m_y < -50 )
	{
		borderCollisions |= BORDER_TOP;
	}

	if ( comp.m_y > SCREEN_H )
	{
		borderCollisions |= BORDER_BOTTOM;
		comp.m_y = -100;
		comp.m_prevY = comp.m_y;
	}

	if ( borderCollisions != 0 )
		collisions.AddBorderCollision( comp.GetParentID(), borderCollisions );	

	//if ( comp.m_collisioMaskDebug != ImageHandle::INVALID )
	//	GetRender().DrawImage( comp.m_collisioMaskDebug, comp.m_x-comp.m_collisionMask.GetWidth()/2, comp.m_y-comp.m_collisionMask.GetHeight()/2 );

	/*if ( !comp.m_collisionMask.IsEmpty() )
	{
		const int w = comp.m_collisionMask.GetWidth();
		const int h = comp.m_collisionMask.GetHeight();

		static ImageHandle tmp = GetRender().CreateImage( w, h, true );
		GetRender().LockImage( tmp );
		for ( int x = 0; x < w; ++x )
		{
			for ( int y =0; y < h; ++y )
			{
				GetRender().DrawPixel( tmp, x, y, comp.m_collisionMask.Get(x,y) ? Color(255,255,255) : Color(0,0,0) );
			}
		}
		GetRender().UnlockImage( tmp );
		GetRender().DrawImage( tmp, comp.m_x, comp.m_y );
	}*/
}

  // Finds collisions between all objects from one layer and all objects from another layer
void PhysicsProcessor::CollideLayers( int layerID1, int layerID2, PhysicsContainer & physics, CollisionData & collisions )
{
	  // Get layers
	PhysicsContainer::CollisionLayerMap::const_iterator layerIt1 = physics.GetLayers().find( layerID1 );
	PhysicsContainer::CollisionLayerMap::const_iterator layerIt2 = physics.GetLayers().find( layerID2 );
	if ( layerIt1 == physics.GetLayers().end() || layerIt2 == physics.GetLayers().end() )
		return;

	const PhysicsContainer::PhysCompIDVector & layer1 = layerIt1->second;
	const PhysicsContainer::PhysCompIDVector & layer2 = layerIt2->second;

	  // Process objects in all layers
	for ( PhysicsContainer::PhysCompIDVector::const_iterator iter1 = layer1.begin(); iter1 != layer1.end(); ++iter1 )
	{
		PhysicComponent *pPhComp1 = physics.Get( *iter1 );

		for ( PhysicsContainer::PhysCompIDVector::const_iterator iter2 = layer2.begin(); iter2 != layer2.end(); ++iter2 )
		{
			PhysicComponent *pPhComp2 = physics.Get( *iter2 );

			  // Process all spheres of both objects
			for ( PhysicComponent::SpheresVector::const_iterator s1 = pPhComp1->m_collisionSpheres.begin(); s1 != pPhComp1->m_collisionSpheres.end(); ++s1 )
			{
				const SSphere & sp1 = *s1;
				for ( PhysicComponent::SpheresVector::const_iterator s2 = pPhComp2->m_collisionSpheres.begin(); s2 != pPhComp2->m_collisionSpheres.end(); ++s2 )
				{
					const SSphere & sp2 = *s2;
					
					float dx1 = sp1.m_dx;
					float dy1 = sp1.m_dy;
					float dx2 = sp2.m_dx;
					float dy2 = sp2.m_dy;

					if ( pPhComp1->m_caps & PHYS_CAPS_FOLLOW_ANGLE )
						Rotate( dx1, dy1, pPhComp1->m_angle );
					else if ( pPhComp1->m_direction == RIGHT_TO_LEFT )
							dx1 = -dx1;

					if ( pPhComp2->m_caps & PHYS_CAPS_FOLLOW_ANGLE )
						Rotate( dx2, dy2, pPhComp2->m_angle );
					else if ( pPhComp2->m_direction == RIGHT_TO_LEFT )
						dx2 = -dx2;

					const float x10 = pPhComp1->m_prevX + dx1;
					const float y10 = pPhComp1->m_prevY + dy1;
					const float x11 = pPhComp1->m_x + dx1;
					const float y11 = pPhComp1->m_y + dy1;

					const float x20 = pPhComp2->m_prevX + dx2;
					const float y20 = pPhComp2->m_prevY + dy2;
					const float x21 = pPhComp2->m_x + dx2;
					const float y21 = pPhComp2->m_y + dy2;

					physics::VECTOR A0( x10, 0, y10 );
					physics::VECTOR A1( x11, 0, y11 );
					physics::VECTOR B0( x20, 0, y20 );
					physics::VECTOR B1( x21, 0, y21 );

					physics::SCALAR t1, t2;

					  // Sweep spheres against each other
					if ( physics::SphereSphereSweep( sp1.m_r, A0, A1, sp2.m_r, B0, B1, t1, t2 ) )
					{
						if ( t1 >= 0 && t1 <= 1 )
						{
							physics::VECTOR A = (A1-A0)*t1;
							physics::VECTOR B = (B1-B0)*t1;
							//IRender & r = GetRender();
							//r.DrawCircleFill( A0.x, A0.z, sp1.m_r, Color(255,0,0));
							//r.DrawCircleFill( A1.x, A1.z, sp1.m_r, Color(255,0,0));
							//r.DrawCircleFill( B0.x, B0.z, sp2.m_r, Color(255,255,0));
							//r.DrawCircleFill( B1.x, B1.z, sp2.m_r, Color(255,255,0));
							//r.DrawCircleFill( A.x, A.z, sp1.m_r, Color(255,0,0));
							//r.DrawCircleFill( B.x, B.z, sp2.m_r, Color(255,255,0));

							  // Add collision information (TIME of collision is lost!)
							collisions.AddObjectCollision( pPhComp1->GetParentID(), pPhComp2->GetParentID() );
						}
					}
				}
			}			
		}
	}
}

  // Process collisions with target (ship, castle etc.)
void PhysicsProcessor::CollideWithTarget( int layerID, PhysicsContainer & physics, CollisionData & collisions )
{
	  // Get layers
	PhysicsContainer::CollisionLayerMap::const_iterator layerIt1 = physics.GetLayers().find( COLLISIONS_TARGET );
	PhysicsContainer::CollisionLayerMap::const_iterator layerIt2 = physics.GetLayers().find( layerID );
	if ( layerIt1 == physics.GetLayers().end() || layerIt2 == physics.GetLayers().end() )
		return;

	const PhysicsContainer::PhysCompIDVector & layer1 = layerIt1->second;
	const PhysicsContainer::PhysCompIDVector & layer2 = layerIt2->second;

	for ( PhysicsContainer::PhysCompIDVector::const_iterator iter1 = layer1.begin(); iter1 != layer1.end(); ++iter1 )
	{
		PhysicComponent *pPhComp1 = physics.Get( *iter1 );

		const BitArray2D & mask = pPhComp1->m_collisionMask;

		if ( mask.IsEmpty() )
			continue;

		const int tx1 = pPhComp1->m_x - mask.GetWidth() / 2;
		const int ty1 = pPhComp1->m_y - mask.GetHeight() / 2;
		const int tx2 = pPhComp1->m_x + mask.GetWidth() / 2;
		const int ty2 = pPhComp1->m_y + mask.GetHeight() / 2;

		for ( PhysicsContainer::PhysCompIDVector::const_iterator iter2 = layer2.begin(); iter2 != layer2.end(); ++iter2 )
		{
			PhysicComponent *pPhComp2 = physics.Get( *iter2 );

			  // Find first collision while moving from old Y coordinate to the new one.
			  // Only falling objects collide with targets and therefore we only need to
			  // care about vertical movement (it's actually not true, but I don't care)
			for ( int y = pPhComp2->m_prevY; y <= pPhComp2->m_y; ++y )
			{
				const int x = pPhComp2->m_x;
				//const int y = pPhComp2->m_y;

				if ( x < tx1 || x >= tx2 || y < ty1 || y >= ty2 )
					continue;

				const int dx = x - tx1;
				const int dy = y - ty1;

				if ( mask.Get( dx, dy ) )
				{
					collisions.AddObjectCollision( pPhComp1->GetParentID(), pPhComp2->GetParentID() );
					break;
				}
			}
		}
	}
}

void PhysicsProcessor::CalculateCollisions( PhysicsContainer & physics, CollisionData & collisions )
{
	CollideLayers( COLLISIONS_PLAYER_BULLET, COLLISIONS_ENEMY, physics, collisions );
	CollideLayers( COLLISIONS_PLAYER_BULLET, COLLISIONS_ENEMY_HARMLESS, physics, collisions );
	CollideLayers( COLLISIONS_PLAYER, COLLISIONS_ENEMY, physics, collisions );
	CollideLayers( COLLISIONS_PLAYER, COLLISIONS_ENEMY_SHOT_DOWN, physics, collisions );
	CollideLayers( COLLISIONS_PLAYER, COLLISIONS_ENEMY_INVUL, physics, collisions );
	CollideWithTarget( COLLISIONS_ENEMY_SHOT_DOWN, physics, collisions );
	CollideWithTarget( COLLISIONS_ENEMY_HARMLESS_SHOT_DOWN, physics, collisions );
	CollideWithTarget( COLLISIONS_PLAYER_SHOT_DOWN, physics, collisions );
}
