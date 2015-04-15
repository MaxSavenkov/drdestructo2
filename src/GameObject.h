#pragma once
#include "BaseComponent.h"

/*
	The main game object component. Only used to hold
	references to other components.
*/
class GameObject : public BaseComponent<GameObject>
{
	GraphicCompID m_graphicID;
	PhysicCompID m_physicID;
	ControlCompID m_controlID;
	MechanicCompID m_mechanicID;
	SoundCompID m_soundID;

public:
	GameObject()
	{}

	GameObject( const GameObjectID & id )
		: BaseComponent<GameObject>( id )
	{}

	void SetGraphicComponent( const GraphicCompID & id )
	{
		m_graphicID = id;
	}
	void SetPhysicComponent( const PhysicCompID & id )
	{
		m_physicID = id;
	}
	void SetControlComponent( const ControlCompID & id )
	{
		m_controlID = id;
	}
	void SetMechanicComponent( const MechanicCompID & id )
	{
		m_mechanicID = id;
	}
	void SetSoundComponent( const SoundCompID & id )
	{
		m_soundID = id;
	}

	const GraphicCompID & GetGraphicComponent() const { return m_graphicID; }
	const PhysicCompID & GetPhysicComponent() const { return m_physicID; }
	const ControlCompID & GetControlComponent() const { return m_controlID; }
	const MechanicCompID & GetMechanicComponent() const { return m_mechanicID; }
	const SoundCompID & GetSoundComponent() const { return m_soundID; }
};
