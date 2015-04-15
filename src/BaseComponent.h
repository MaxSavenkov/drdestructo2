#pragma once
#include "ComponentID.h"

/*
	This is base class for all Component classed.
	It contains a typed ID and not much else.
*/
template<typename TChild>
class BaseComponent
{
protected:
	ComponentID<TChild> m_id;

public:
	BaseComponent(){}
	BaseComponent( const ComponentID<TChild> & id )
		: m_id( id )
	{}

	const ComponentID<TChild> & GetID() const { return m_id; }
};

/*
	Sub-component is a component that can has parent
	component. Most components in game actually inherit
	it rather than BaseComponent. It adds ID of parent
	component and a function to set it.
*/
template<typename TChild, typename TParent>
class SubComponent : public BaseComponent<TChild>
{
protected:
	ComponentID<TParent> m_parent;

public:
	SubComponent(){}
	SubComponent( const ComponentID<TChild> & id )
		: BaseComponent<TChild>( id )
	{}

	SubComponent( const ComponentID<TChild> & id, const ComponentID<TParent> & parentID )
		: BaseComponent<TChild>( id )
		, m_parent( parentID )
	{}

	void SetParent( const ComponentID<TParent> & parent ) { m_parent = parent; }
	const ComponentID<TParent> & GetParentID() const { return m_parent; }
};
