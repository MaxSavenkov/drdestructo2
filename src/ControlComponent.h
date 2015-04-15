#pragma once
#include "BaseComponent.h"
#include "AIProtoManager.h"
#include "AICommands.h"

class BaseAICommand;

/*
	Some data copied from Physic component for use by AI.
*/
struct SAIData
{
	float m_x;
	float m_y;
	float m_angle;
};

/*
	Component that is used to control object's behaviour, either
	for Player or AI
*/
class ControlComponent : public SubComponent<ControlComponent, GameObject>
{
public:
	enum EType
	{
		TYPE_PLAYER,
		TYPE_AI,
	};

	ControlComponent()
	{}

	ControlComponent( const ControlCompID & id )
		: SubComponent<ControlComponent, GameObject>( id )
		, m_pCurrentTrigger( 0 )
		, m_direction( 1 )
		, m_pState( 0 )
	{}

	  // Type
	EType m_type;
	  // Mask of control flags
	int m_controls;
	  // See EDirection
	int m_direction;

	  // AI Lua function to be run at the each frame
	std::string m_function;

	  // State that is used to run m_function. Actually, just a Lua state hidden begind interface
	IAIState *m_pState;
	  // Current trigger. Until it fires, m_function would not be run
	IAITrigger *m_pCurrentTrigger;
      // Data from other components for AI to use
	SAIData m_aiData;

	void SetTrigger( IAITrigger *pTrigger )
	{
		if ( m_pCurrentTrigger )
			delete m_pCurrentTrigger;

		m_pCurrentTrigger = pTrigger;
	}
};
