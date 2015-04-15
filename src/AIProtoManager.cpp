#include "stdafx.h"
#include "AIProtoManager.h"
#include "XMLDeserializer.h"

/*
	A a brief description of how AI works.

	I use Lua co-routines to write simple, readable AI scripts.
	That means that script can be suspended at any line and
	resumed later, with saved variables etc.

	Usually, in this game co-routines are suspended until
	Trigger conditions are specified (for example, if some
	time passed, or object moved by a specified distance).

	If script function ends without being suspended, it will
	be re-run from start next time.

	To allow multiple co-routines to be run at the same time,
	we need multiple Lua states. This is solved by using Lua
	threads, which are not threads at all, but rather just
	lightweight copies of main Lua state.
*/

AILuaState::AILuaState( lua_State *pState, int key )
	: m_pState( pState )
	, m_key( key )
{}

AILuaState::~AILuaState()
{
	lua_State *pMainState = ((AILuaState*)GetAI().GetBaseState())->GetLuaState();
	if ( pMainState != m_pState )
		luaL_unref( pMainState, LUA_REGISTRYINDEX, m_key );
}

void AILuaState::Run( const char *func, IScriptContext & context )
{
	try
	{
		luabind::resume_function<void>( m_pState, func, &context );
	}
	catch( std::exception e )
	{
		const char *error = lua_tostring(m_pState, -1);
		lua_pop(m_pState, 1); // remove error message
		//printf( "%s\n", e.what() );
	}
	catch( ... )
	{
		//printf("Achtung\n");
	}
}

AIProtoManager::AIProtoManager()
	: m_state( lua_open(), -1 )
{
	lua_State *pState = m_state.GetLuaState();
	luabind::open( pState );

	luaL_openlibs( pState );
}

AIProtoManager::~AIProtoManager()
{
}

IAIState *AIProtoManager::CreateAIState()
{
	if ( !m_state.GetLuaState() )
		return 0;

	  // Create new Lua thread to be used by this state
	lua_State *pThread = lua_newthread( m_state.GetLuaState() );
	  // Remember its key for deletion
	int threadKey = luaL_ref( m_state.GetLuaState(), LUA_REGISTRYINDEX );

	return new AILuaState( pThread, threadKey );
}

bool AIProtoManager::LoadResources()
{
	ISystem & sys = GetSystem();
	std::vector<char> buf;
	if ( !sys.LoadFile( true, "Data/Protos/ai.lua", buf ) )
		return false;

	const int error = luaL_dostring( m_state.GetLuaState(), &buf[0] );
	if ( error != 0 )
	{
		const char *errorText = lua_tostring(m_state.GetLuaState(), -1);
		GetLog().Log( 0, LL_CRITICAL, "Error loading lua scripts: %s\n", errorText );
		lua_pop(m_state.GetLuaState(), 1); // remove error message
		return false;
	}

	return true;
}


AIProtoManager & ModifyAI()
{
	static AIProtoManager m;
	return m;
}

AIProtoManager & GetAI()
{
	return ModifyAI();
}

void LoadAI()
{
	AIProtoManager & m = ModifyAI();
	m.LoadResources();
}
