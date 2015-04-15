#pragma once
#include "GameContext.h"
#include "AIProtoManager.h"

class ControlComponent;
class IGameContext;
class IAIState;
class IAITrigger;

  // This Processor processes AI objects
class AIProcessor : public IScriptContext
{
	  // Currently processed AI component to provide context for Lua calls
	ControlComponent *m_pCurrentComp;
	  // Saved game context for Lua calls
	IGameContext *m_pContext;

	  // Will repeatedly set given control flag until trigger is satisfied
	void Control( int ctrl, IAITrigger *pTrigger );
	  // Self-descriptionary :)
	void DestroySelf();
	  // Returns a structure which can be used to specify params of newly created object
	SCreateObjectRequest::SData CreateObjectParams();
	  // Creates object using specified params
	void CreateObject( const std::string & proto, const SCreateObjectRequest::SData & params, bool replace );
	  // Return current coordinates
	float GetSelfX() const;
	float GetSelfY() const;

public:
	AIProcessor()
		: m_pCurrentComp( 0 )
	{}

	void RegisterFunctions( IAIState *pState );
	
	void Update( float dt, ControlComponent & comp, IGameContext & context );
};
