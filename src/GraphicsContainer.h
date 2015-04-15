#pragma once
#include "ComponentID.h"
#include "ComponentContainer.h"
#include "GraphicComponent.h"

/*
	Special container for graphic components which additionaly
	stores a map of objects by layers to allow sequential drawing
	of layers
*/
class GraphicsContainer : public CompList<GraphicComponent>
{
public:
	typedef std::vector<GraphicCompID> GraphCompIDVector;
	typedef std::map<RenderLayerID, GraphCompIDVector> RenderLayerMap;

private:
	RenderLayerMap m_layers;

public:
	  // Moves objects between layers if have changed
	void Update()
	{
		for ( Iterator iter = GetIterator(); !iter.AtEnd(); ++iter )
		{
			GraphicComponent *pComp = *iter;
			if ( pComp->CheckLayerUpdated() )
			{
				if ( pComp->GetLayerID() != INVALID_LAYER )
				{
					GraphCompIDVector & vec = m_layers[ pComp->GetLayerID() ];
					GraphCompIDVector::iterator iterFind = std::find( vec.begin(), vec.end(), pComp->GetID() );
					if ( iterFind != vec.end() )
						vec.erase( iterFind );
				}
				m_layers[ pComp->GetLayerID() ].push_back( pComp->GetID() );
			}
		}
	}

	  // Removes object from render layer and then from container
	void Remove( const GraphicCompID & id )
	{
		const GraphicComponent *pComp = Get( id );
		if ( pComp )
		{
			const RenderLayerID layer = pComp->GetLayerID();
			
			GraphCompIDVector & vec = m_layers[ layer ];
			GraphCompIDVector::iterator iter = std::find( vec.begin(), vec.end(), pComp->GetID() );
			if ( iter != vec.end() )
				vec.erase( iter );
			
			CompList<GraphicComponent>::Remove( id );
		}
	}

	void Clear()
	{
		CompList<GraphicComponent>::Clear();
		m_layers.clear();
	}

	const RenderLayerMap & GetLayers() const { return m_layers; }
};
