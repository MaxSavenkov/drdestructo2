#pragma once
#include "BaseAICommand.h"
#include "ControlComponent.h"

/*
	This file describes a set of commands and triggers used by AI
*/

class PhysicComponent;
class ControlComponent;

enum ECompareOp
{
	COMPARE_EQ,
	COMPARE_NE,
	COMPARE_LT,
	COMPARE_GT,
	COMPARE_LE,
	COMPARE_GE,
};

template<typename T>
float BoolCompare( ECompareOp op, T a, T b )
{
	switch( op )
	{
		case COMPARE_EQ: return ( a == b ) ? 1 : 0;
		case COMPARE_NE: return ( a != b ) ? 1 : 0;
		case COMPARE_LT: return ( a < b ) ? 1 : 0;
		case COMPARE_GT: return ( a > b ) ? 1 : 0;
		case COMPARE_LE: return ( a <= b ) ? 1 : 0;
		case COMPARE_GE: return ( a >= b ) ? 1 : 0;
	}

	return 0.0f;
}

  // Checks global X coordinate of object according to passed comparison type
class AITrigger_GlobalX : public IAITrigger
{
	ECompareOp m_compare;
	float m_x;

public:
	AITrigger_GlobalX( ECompareOp compare, float x )
		: m_compare( compare )
		, m_x( x )
	{}

	float Update( float dt, const ControlComponent & ctrl );
	void Reset( const ControlComponent & ctrl ){}
};

  // Checks global Y coordinate of object according to passed comparison type
class AITrigger_GlobalY : public IAITrigger
{
	ECompareOp m_compare;
	float m_y;

public:
	AITrigger_GlobalY( ECompareOp compare, float y )
		: m_compare( compare )
		, m_y( y )
	{}

	float Update( float dt, const ControlComponent & ctrl );
	void Reset( const ControlComponent & ctrl ){}
};

  // Checks that change of X coordinate since trigger initialization is greate or equals to given
class AITrigger_DeltaX : public IAITrigger
{
	float m_targetDx;
	float m_currentDx;
	float m_prevX;

public:
	AITrigger_DeltaX( float dx )
		: m_targetDx( dx )
		, m_currentDx( 0 )
		, m_prevX( 0 )
	{}

	float Update( float dt, const ControlComponent & ctrl );
	void Reset( const ControlComponent & ctrl );
};

  // Checks that change of Y coordinate since trigger initialization is greate or equals to given
class AITrigger_DeltaY : public IAITrigger
{
	float m_targetDy;
	float m_currentDy;
	float m_prevY;

public:
	AITrigger_DeltaY( float dy )
		: m_targetDy( dy )
		, m_currentDy( 0 )
		, m_prevY( 0 )
	{}

	float Update( float dt, const ControlComponent & ctrl );
	void Reset( const ControlComponent & ctrl );
};

  // Checks that give time has elapsed since initialization of trigger
class AITrigger_Timer : public IAITrigger
{
	float m_currentTime;
	float m_startTime;

public:
	AITrigger_Timer( float time )
		: m_currentTime( time )
		, m_startTime( time )
	{}

	float Update( float dt, const ControlComponent & ctrl );
	void Reset( const ControlComponent & ctrl );
};

  // Returns true one time
class AITrigger_OneTime : public IAITrigger
{
	int m_counter;

public:
	AITrigger_OneTime()
		: m_counter( 0 )
	{}

	float Update( float dt, const ControlComponent & ctrl );
	void Reset( const ControlComponent & ctrl );
};

  // Obsolete?
class AITrigger_OnCount : public IAITrigger
{
	int m_counter;
	int m_target;

public:
	AITrigger_OnCount( int target )
		: m_counter( 0 )
		, m_target( target )
	{}

	float Update( float dt, const ControlComponent & ctrl );
	void Reset( const ControlComponent & ctrl );
};

  // For commands that should never end
class AITrigger_Eternal : public IAITrigger
{
public:
	AITrigger_Eternal()
	{}

	float Update( float dt, const ControlComponent & ctrl ) { return 0.0f; }
	void Reset( const ControlComponent & ctrl ){}
};
