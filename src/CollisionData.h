#pragma once

#include "GameObject.h"

/*
	Information about a single collision between objects
	(Just ojects' IDs).
*/
struct SObjectCollision
{
	GameObjectID m_object1;
	GameObjectID m_object2;
};

/*
	ID of screen edge with which object has
	collided.
*/
enum EBorder
{
	BORDER_LEFT		= 1 << 0,
	BORDER_RIGHT    = 1 << 1,
	BORDER_TOP      = 1 << 2,
	BORDER_BOTTOM   = 1 << 3,
};

/*
	Information about a single collision between object
	and screen edge
*/
struct SBorderCollision
{
	GameObjectID m_objectID;
	int m_borders;
};

/*
	Class that contains information about all collisions that
	happened this frame. Used to pass this information from
	PhysicProcessor to logic code.
*/
class CollisionData
{
	typedef std::vector<SObjectCollision> ObjectCollisions;
	ObjectCollisions m_objectCollisions;

	typedef std::vector<SBorderCollision> BorderCollisions;
	BorderCollisions m_borderCollisions;

	  // Non-copyable
	CollisionData( const CollisionData & );
	void operator = ( const CollisionData & );

public:
	CollisionData()
	{}

	void Reset()
	{
		m_objectCollisions.clear();
		m_borderCollisions.clear();
	}

	void AddObjectCollision( const GameObjectID & id1, const GameObjectID & id2 )
	{
		m_objectCollisions.push_back( SObjectCollision() );
		m_objectCollisions.back().m_object1 = id1;
		m_objectCollisions.back().m_object2 = id2;
	}

	void AddBorderCollision( const GameObjectID & id, int borders )
	{
		m_borderCollisions.push_back( SBorderCollision() );
		m_borderCollisions.back().m_objectID = id;
		m_borderCollisions.back().m_borders = borders;
	}

	unsigned int GetObjectCollisionsCount() const { return (int)m_objectCollisions.size(); }
	unsigned int GetBorderCollisionsCount() const { return (int)m_borderCollisions.size(); }

	const SObjectCollision *GetObjectCollision( size_t index ) const { return index >= m_objectCollisions.size() ? 0 : &m_objectCollisions[ index ]; }
	const SBorderCollision *GetBorderCollision( size_t index ) const { return index >= m_borderCollisions.size() ? 0 : &m_borderCollisions[ index ]; }
};
