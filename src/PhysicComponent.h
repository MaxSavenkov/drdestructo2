#pragma once
#include "BaseComponent.h"
#include "BitArray2D.h"
#include "IRender.h"

/*
	Collision sphere description
*/
struct SSphere
{
	SSphere()
		: m_dx( 0 )
		, m_dy( 0 )
		, m_r( 0 )
	{}

	float m_dx;
	float m_dy;
	float m_r;
};

/*
	Collision layer's ID. Collision layers are used to separate objects
	into groups and avoid unnecessary collision checks.
*/
typedef int CollisionLayerID;
const CollisionLayerID INVALID_COLLISION_LAYER = -1;

/*
	This component describes current physic state of object
*/
class PhysicComponent : public SubComponent<PhysicComponent, GameObject>
{
public:
	PhysicComponent()
	{}

	PhysicComponent( const PhysicCompID & id )
		: SubComponent<PhysicComponent, GameObject>( id )
		, m_speed( 0 )
		, m_angle( 0 )
		, m_caps( 0 )
		, m_collisionLayer( INVALID_COLLISION_LAYER )
		, m_collisionLayerUpdated( false )
		, m_flipTimer( 0.0f )
		, m_flipCurrentTimer( 0.0f )
		, m_flipped( false )
		, m_direction( 1 )
		, m_turnSpeed( 0 )
	{}

	  // Previous position, for collision purposes
	float m_prevX;
	float m_prevY;

	  // Current position and speed
	float m_x;
	float m_y;
	float m_angle;
	float m_speed;

	  // Data from prototype
	  // Speeds
	float m_minSpeed;
	float m_maxSpeed;
	  // Acceleration
	float m_accel;
	  // Vertical speed for objects which use speedX and speedY instead of speed+angle
	float m_vertSpeed;
	  // Current angle change speed
	float m_turnSpeed;
	  // Maximum angle change speed
	float m_maxTurnSpeed;
	  // Separate speeds for X and Y axis for objects that need them instead of speed+angle
	float m_speedX;
	float m_speedY;
	  // Timer for objects doing "about-face"
	float m_flipTimer;
	  // Physical capabilities of object
	int m_caps;

	  // See EDirection
	int m_direction;

	  // Current flip timer value
	float m_flipCurrentTimer;
	  // Set to true if object has completed "about-face"
	bool m_flipped;

	void StartFlip()
	{
		m_flipCurrentTimer = m_flipTimer;
	}

	void UpdateFlip( float dt )
	{
		if ( m_flipCurrentTimer > 0.0f )
		{
			m_flipCurrentTimer -= dt;
			if ( m_flipCurrentTimer < 0.0f )
			{
				m_flipCurrentTimer = 0.0f;
				m_flipped = true;
			}
		}
	}

	bool IsFlipping() const
	{
		return m_flipCurrentTimer > 0.0f;
	}

	bool CheckFlipped()
	{
		bool ret = m_flipped;
		m_flipped = false;
		return ret;
	}

private:
	  // Object collision layer information
	CollisionLayerID m_collisionLayer;
	bool m_collisionLayerUpdated;

public:
	void SetCollisionLayer( CollisionLayerID layer )
	{
		if ( m_collisionLayer != layer )
		{
			m_collisionLayer = layer;
			m_collisionLayerUpdated = true;
		}
	}

	bool CheckUpdatedCollisionLayer()
	{
		bool ret = m_collisionLayerUpdated;
		m_collisionLayerUpdated = false;
		return ret;
	}

	CollisionLayerID GetCollisionLayerID() const
	{
		return m_collisionLayer;
	}

	  // Collision spheres of object
	typedef std::vector<SSphere> SpheresVector;
	SpheresVector m_collisionSpheres;

	  // Mask for pixel-perfect collisions (only used for targets (ships & castles etc.) )
	BitArray2D m_collisionMask;
	  // Mask as image for drawing for debug purposes
	ImageHandle m_collisioMaskDebug;
};
