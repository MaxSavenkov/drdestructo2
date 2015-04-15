#pragma once
#include "GraphicsContainer.h"
#include "PhysicsContainer.h"
#include "PhysicComponent.h"
#include "ControlComponent.h"
#include "MechanicComponent.h"
#include "SoundComponent.h"
#include "GameObject.h"
#include "Player.h"
#include "Music.h"
#include "CollisionData.h"

/*
	Classed used to store all components.
	Nothing fancy here.
*/
class ObjectsStorage
{
public:
	typedef CompList<ControlComponent> ControlsContainer;
	typedef CompList<MechanicComponent> MechanicsContainer;
	typedef CompList<SoundComponent> SoundsContainer;
	typedef CompList<GameObject> GameObjectContainer;
	
private:
	PhysicsContainer m_physics;
	ControlsContainer m_controls;
	GraphicsContainer m_graphics;
	MechanicsContainer m_mechanics;
	SoundsContainer m_sounds;
	GameObjectContainer m_objects;
	MusicComponent m_music;

	Player m_player;

	CollisionData m_collisions;

public:
	GraphicComponent	*CreateGraphicComponent()	{ return m_graphics.Insert();	}
	PhysicComponent		*CreatePhysicComponent()	{ return m_physics.Insert();	}
	ControlComponent	*CreateControlComponent()	{ return m_controls.Insert();	}
	MechanicComponent   *CreateMechanicComponent()	{ return m_mechanics.Insert();	}
	SoundComponent		*CreateSoundComponent()		{ return m_sounds.Insert();     }
	GameObject			*CreateGameObject()			{ return m_objects.Insert();	}

	GraphicComponent	*GetGraphicComponent	( const GraphicCompID & id )	{ return m_graphics.Get( id );	}
	PhysicComponent		*GetPhysicComponent		( const PhysicCompID & id )		{ return m_physics.Get( id );	}
	ControlComponent	*GetControlComponent	( const ControlCompID & id )	{ return m_controls.Get( id );	}
	MechanicComponent   *GetMechanicComponent	( const MechanicCompID & id )	{ return m_mechanics.Get( id );	}
	SoundComponent		*GetSoundComponent  	( const SoundCompID & id )		{ return m_sounds.Get( id );	}
	GameObject			*GetGameObject			( const GameObjectID & id)		{ return m_objects.Get( id );	}

	Player & GetPlayer() { return m_player; }
	MusicComponent & GetMusic() { return m_music; }

	CollisionData & GetCollisions() { return m_collisions; }
	const CollisionData & GetCollisions() const { return m_collisions; }
	
	const GraphicComponent	*GetGraphicComponent	( const GraphicCompID & id )	const { return m_graphics.Get( id );	}
	const PhysicComponent	*GetPhysicComponent		( const PhysicCompID & id )		const { return m_physics.Get( id );		}
	const ControlComponent	*GetControlComponent	( const ControlCompID & id )	const { return m_controls.Get( id );	}
	const MechanicComponent *GetMechanicComponent	( const MechanicCompID & id )	const { return m_mechanics.Get( id );	}
	const SoundComponent    *GetSoundComponent		( const SoundCompID & id )		const { return m_sounds.Get( id );		}
	const GameObject		*GetGameObject			( const GameObjectID & id)		const { return m_objects.Get( id );		}

	const Player & GetPlayer() const { return m_player; }
	const MusicComponent & GetMusic() const { return m_music; }

	PhysicsContainer & GetPhysics() { return m_physics; }
	const PhysicsContainer & GetPhysics() const { return m_physics; }

	GraphicsContainer & GetGraphics() { return m_graphics; }
	const GraphicsContainer & GetGraphics() const { return m_graphics; }

	ControlsContainer & GetControls() { return m_controls; }
	const ControlsContainer & GetControls() const { return m_controls; }

	MechanicsContainer & GetMechanics() { return m_mechanics; }
	const MechanicsContainer & GetMechanics() const { return m_mechanics; }

	SoundsContainer & GetSounds() { return m_sounds; }
	const SoundsContainer & GetSounds() const { return m_sounds; }

	GameObjectContainer & GetGameObjects() { return m_objects; }
	const GameObjectContainer & GetGameObjects() const { return m_objects; }

	void Clear()
	{
		m_physics.Clear();
		m_controls.Clear();
		m_graphics.Clear();
		m_mechanics.Clear();
		m_sounds.Clear();
		m_objects.Clear();
		m_player = Player();
		m_controls.Clear();
	}

public:
	void Update()
	{
		m_graphics.Update();
		m_physics.Update();
	}
};
