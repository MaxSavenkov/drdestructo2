#pragma once
#include "PrototypeManager.h"
#include "ControlComponent.h"

struct SControlsProto
{
	ControlComponent::EType m_type;

	SControlsProto()
		: m_type( ControlComponent::TYPE_AI )
	{
	}
};

class ControlsProtoManager : public PrototypeManager<SControlsProto>
{
public:
	void LoadResources();
};

const ControlsProtoManager & GetControls();
void LoadControls();
