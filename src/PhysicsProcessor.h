#pragma once

class PhysicComponent;
class CollisionData;
class PhysicsContainer;

  // Processes physic components, calculates movement and collisions
class PhysicsProcessor
{
	void CollideLayers( int layerID1, int layerID2, PhysicsContainer & physics, CollisionData & collisions );
	void CollideWithTarget( int layerID, PhysicsContainer & physics, CollisionData & collisions );
public:
	void Update( float dt, PhysicComponent & comp, CollisionData & collisions );
	void CalculateCollisions( PhysicsContainer & physics, CollisionData & collisions );
};
