#pragma once
#include "ComponentContainer.h"
#include "PhysicComponent.h"

/*
	Special container for physic Components with layers support.
	See GraphicsContainer.
*/
class PhysicsContainer : public CompList<PhysicComponent>
{
public:
	typedef std::vector<PhysicCompID> PhysCompIDVector;
	typedef std::map<CollisionLayerID, PhysCompIDVector> CollisionLayerMap;

private:
	CollisionLayerMap m_layers;

public:
	void Update()
	{
		for ( Iterator iter = GetIterator(); !iter.AtEnd(); ++iter )
		{
			PhysicComponent *pComp = *iter;
			if ( pComp->CheckUpdatedCollisionLayer() )
			{
				if ( pComp->GetCollisionLayerID() != INVALID_COLLISION_LAYER )
				{
					PhysCompIDVector & vec = m_layers[ pComp->GetCollisionLayerID() ];
					PhysCompIDVector::iterator iterFind = std::find( vec.begin(), vec.end(), pComp->GetID() );
					if ( iterFind != vec.end() )
						vec.erase( iterFind );
				}
				m_layers[ pComp->GetCollisionLayerID() ].push_back( pComp->GetID() );
			}
		}
	}

	void Remove( const PhysicCompID & id )
	{
		if ( !id.IsValid() )
			return;

		const PhysicComponent *pComp = Get( id );
		if ( pComp )
		{
			const CollisionLayerID layer = pComp->GetCollisionLayerID();

			PhysCompIDVector & vec = m_layers[ layer ];
			PhysCompIDVector::iterator iter = std::find( vec.begin(), vec.end(), pComp->GetID() );
			if ( iter != vec.end() )
				vec.erase( iter );
		}
		else
		{
			//__debugbreak();
		}

		CompList<PhysicComponent>::Remove( id );
	}

	void Clear()
	{
		CompList<PhysicComponent>::Clear();
		m_layers.clear();
	}

	const CollisionLayerMap & GetLayers() const { return m_layers; }
};
