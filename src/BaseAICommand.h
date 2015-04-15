#pragma once
#include "GameContext.h"

class ControlComponent;
class PhysicComponent;

class IAITrigger
{
public:
	virtual ~IAITrigger(){}
	virtual void Reset( const ControlComponent & ctrl ) = 0;
	virtual float Update( float dt, const ControlComponent & ctrl ) = 0;
};

