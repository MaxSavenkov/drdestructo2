#include "stdafx.h"
#include "AIProcessor.h"
#include "ControlComponent.h"
#include "PhysicComponent.h"
#include "BaseAICommand.h"
#include "GameContext.h"
#include "ObjectsStorage.h"
#include "Vars.h"

namespace
{
	UseIntVar CONTROL_TURN_UP("CONTROL_TURN_UP" );
	UseIntVar CONTROL_TURN_DOWN("CONTROL_TURN_DOWN" );
	UseIntVar CONTROL_SPEED_UP("CONTROL_SPEED_UP" );
	UseIntVar CONTROL_VERT_UP("CONTROL_VERT_UP" );
	UseIntVar CONTROL_VERT_DOWN("CONTROL_VERT_DOWN" );
	UseIntVar CONTROL_FIRE1("CONTROL_FIRE1" );
	UseIntVar CONTROL_FIRE2("CONTROL_FIRE2" );
	UseIntVar CONTROL_STOP("CONTROL_STOP" );
	UseIntVar CONTROL_GO("CONTROL_GO" );
	UseIntVar CONTROL_FLIP("CONTROL_FLIP" );
	UseIntVar CONTROL_RANDOM_TELEPORT("CONTROL_RANDOM_TELEPORT" );
}

using namespace luabind;

IAITrigger *CreateTrigger_Timer( float time )
{
	return new AITrigger_Timer( time );
}

IAITrigger *CreateTrigger_DY( float dy )
{
	return new AITrigger_DeltaY( dy );
}

IAITrigger *CreateTrigger_DX( float dx )
{
	return new AITrigger_DeltaX( dx );
}

IAITrigger *CreateTrigger_Eternal()
{
	return new AITrigger_Eternal();
}

IAITrigger *CreateTrigger_OneTime()
{
	return new AITrigger_OneTime();
}

  // Bind C++ functions to Lua calls
void AIProcessor::RegisterFunctions( IAIState *pAIState )
{
	AILuaState *pLuaAIState = (AILuaState *)pAIState;
	lua_State *pState = pLuaAIState->GetLuaState();
	if ( !pState )
		return;

	module( pState )
	[
		def( "RandomInt", &RndInt ),
		def( "RandomFloat", &RndFloat ),
		
		def( "Trigger_Timer", &CreateTrigger_Timer ),
		def( "Trigger_DY", &CreateTrigger_DY ),
		def( "Trigger_DX", &CreateTrigger_DX ),
		def( "Trigger_Eternal", &CreateTrigger_Eternal ),
		def( "Trigger_OneTime", &CreateTrigger_OneTime ),

		class_<IScriptContext>( "Context" )			
			.def( "Control", &IScriptContext::Control, yield )
			.def( "CreateObjectParams", &IScriptContext::CreateObjectParams )
			.def( "CreateObject", &IScriptContext::CreateObject, yield )
			.def( "DestroySelf", &IScriptContext::DestroySelf, yield )
			.def( "GetSelfY", &IScriptContext::GetSelfY )
			.def( "GetSelfX", &IScriptContext::GetSelfX )

			.enum_( "ControlConstants" )
			[
				value( "IDLE", 0 ),
				value( "CTRL_VERT_UP", CONTROL_VERT_UP ),
				value( "CTRL_VERT_DOWN", CONTROL_VERT_DOWN ),
				value( "CTRL_SPEED_UP", CONTROL_SPEED_UP ),
				value( "CTRL_STOP", CONTROL_STOP ),
				value( "CTRL_GO", CONTROL_GO ),
				value( "CTRL_FLIP", CONTROL_FLIP ),
				value( "CTRL_FIRE1", CONTROL_FIRE1 ),
				value( "CTRL_FIRE2", CONTROL_FIRE2 ),
				value( "CTRL_RANDOM_TELEPORT", CONTROL_RANDOM_TELEPORT )
			]
		,

		class_<SCreateObjectRequest::SData>( "CreateParams" )
			.def_readwrite( "x", &SCreateObjectRequest::SData::x )
			.def_readwrite( "y", &SCreateObjectRequest::SData::y )
			.def_readwrite( "dir", &SCreateObjectRequest::SData::dir )
			.def_readwrite( "angle", &SCreateObjectRequest::SData::angle )
			.def_readwrite( "speedX", &SCreateObjectRequest::SData::speedX )
			.def_readwrite( "speedY", &SCreateObjectRequest::SData::speedY )
			.def_readwrite( "hasSpeed", &SCreateObjectRequest::SData::hasSpeed )
			.def_readwrite( "ai", &SCreateObjectRequest::SData::ai )
		,

		class_<IAITrigger>( "IAITrigger" )
	];
}

void AIProcessor::Update( float dt, ControlComponent & comp, IGameContext & context )
{
	if ( !comp.m_pState )
		return;

	bool triggerOK = true;
 
	  // Check current trigger's condition
	if ( comp.m_pCurrentTrigger )
	{
		triggerOK = ( comp.m_pCurrentTrigger->Update( dt, comp ) >= 1.0f );
		if ( triggerOK )
		{
			delete comp.m_pCurrentTrigger;
			comp.m_pCurrentTrigger = 0;
		}
	}

	  // Only resume execution if trigger is satisfied
	if ( triggerOK )
	{
		m_pContext = &context;
		m_pCurrentComp = &comp;
		comp.m_pState->Run( comp.m_function.c_str(), *this );
		m_pCurrentComp = 0;
		m_pContext = 0;
	}
}

void AIProcessor::Control( int ctrl, IAITrigger *pTrigger )
{
	if ( !m_pCurrentComp )
		return;

	if ( pTrigger )
		pTrigger->Reset( *m_pCurrentComp );

	m_pCurrentComp->m_controls = ctrl;
	m_pCurrentComp->SetTrigger( pTrigger );	
}

void AIProcessor::DestroySelf()
{
	m_pContext->DestroyGameObject( m_pCurrentComp->GetParentID() );
}

SCreateObjectRequest::SData AIProcessor::CreateObjectParams()
{
	SCreateObjectRequest::SData data;
	data.x = m_pCurrentComp->m_aiData.m_x;
	data.y = m_pCurrentComp->m_aiData.m_y;
	data.angle = m_pCurrentComp->m_aiData.m_angle;
	data.dir = (EDirection)m_pCurrentComp->m_direction;
	
	return data;
}

void AIProcessor::CreateObject( const std::string & proto, const SCreateObjectRequest::SData & params, bool replace )
{
	GameObjectID replaceID = replace ? m_pCurrentComp->GetParentID() : GameObjectID();
	m_pContext->CreateGameObject( proto, params, replaceID );
}

float AIProcessor::GetSelfX() const
{
	return m_pCurrentComp ? m_pCurrentComp->m_aiData.m_x : 0;
}

float AIProcessor::GetSelfY() const
{
	return m_pCurrentComp ? m_pCurrentComp->m_aiData.m_y : 0;
}
