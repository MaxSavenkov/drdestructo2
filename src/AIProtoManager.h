#pragma once

#include "PrototypeManager.h"
#include "Random.h"
#include "GameContext.h"

class IAITrigger;

  // Context used for Lua calls. Implemented by AIProcessor
class IScriptContext
{
public:
	virtual void Control( int control, IAITrigger *pTrigger ) = 0;
	virtual void DestroySelf() = 0;
	virtual SCreateObjectRequest::SData CreateObjectParams() = 0;
	virtual void CreateObject( const std::string & proto, const SCreateObjectRequest::SData & params, bool replace = false ) = 0;
	virtual float GetSelfX() const =  0;
	virtual float GetSelfY() const =  0;
};

  // State used ro run script. Wrapper around Lua state.
class IAIState
{
public:
	virtual ~IAIState(){}
	virtual void Run( const char *func, IScriptContext & context ) = 0;
};

class AILuaState : public IAIState
{
	  // Lua state
	lua_State *m_pState;
	  // Lua key used to distinguish one state create by lua_newthread from another
	int m_key;

public:	
	AILuaState( lua_State *pState, int key );
	~AILuaState();

	lua_State *GetLuaState() { return m_pState; }
	const lua_State *GetLuaState() const { return m_pState; }
	void Run( const char *func, IScriptContext & context );
};

class AIProtoManager
{
	AILuaState m_state;

public:
	AIProtoManager();
	~AIProtoManager();

	IAIState *GetBaseState() { return &m_state; }

	IAIState *CreateAIState();
	bool LoadResources();
};

AIProtoManager & GetAI();
void LoadAI();
