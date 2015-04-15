#pragma once

/*
	Component identifier. Uses index + passport system to allow
	index reuse. Templated with Component type to allow writing
	more type-safe functions.
*/
template<typename TComp>
class ComponentID
{
	int m_index;
	int m_passport;
public:
	ComponentID()
		: m_index( -1 )
		, m_passport( 0 )
	{}

	ComponentID( int index, int passport = 0 )
		: m_index( index )
		, m_passport( passport )
	{}

	int GetIndex() const { return m_index; }
	int GetPassport() const { return m_passport; }

	bool operator == ( const ComponentID & other ) const
	{
		return m_index == other.m_index &&  m_passport == other.m_passport;
	}

	bool IsValid() const
	{
		return m_index >= 0 && m_passport >= 0;
	}
};

  // Concrete component IDs used in game
typedef ComponentID<class ControlComponent> ControlCompID;
typedef ComponentID<class GraphicComponent> GraphicCompID;
typedef ComponentID<class PhysicComponent> PhysicCompID;
typedef ComponentID<class MechanicComponent> MechanicCompID;
typedef ComponentID<class SoundComponent> SoundCompID;
typedef ComponentID<class GameObject> GameObjectID;


