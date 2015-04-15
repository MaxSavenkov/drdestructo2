#pragma once
#include "PrototypeManager.h"
#include "PhysicComponent.h"

struct SPhysicsProto
{
	float m_minSpeed;
	float m_maxSpeed;
	float m_turnSpeed;
	float m_verticalSpeed;
	float m_accel;
	float m_flipTimer;
	int m_caps;
	int m_collisionLayer;
	typedef std::vector<SSphere> SpheresVector;
	SpheresVector m_collisionSpheres;
	std::string m_collisionMask;
	
	SPhysicsProto()
		: m_minSpeed( 0 )
		, m_maxSpeed( 0 )
		, m_turnSpeed( 0 )
		, m_verticalSpeed( 0 )
		, m_accel( 0 )
		, m_caps( 0 )
		, m_flipTimer( 1.0f )
		, m_collisionLayer( 0 )
	{
		
	}

	bool SetCaps( const int & caps )
	{
		m_caps |= caps;
		return true;
	}

	bool SetCollisionMask( const std::string & fileName );

	void AddSphere( const SSphere & s )
	{
		m_collisionSpheres.push_back( s );
	}
};

class PhysicsProtoManager : public PrototypeManager<SPhysicsProto>
{
public:
	void LoadResources();
};

const PhysicsProtoManager & GetPhysics();
void LoadPhysics();
