#include "stdafx.h"
#include "AICommands.h"
#include "ControlComponent.h"
#include "PhysicComponent.h"
#include "Common.h"

float AITrigger_GlobalX::Update( float dt, const ControlComponent & ctrl )
{
	return BoolCompare( m_compare, ctrl.m_aiData.m_x, m_x );
}

float AITrigger_GlobalY::Update( float dt, const ControlComponent & ctrl )
{
	return BoolCompare( m_compare, ctrl.m_aiData.m_y, m_y );
}

float AITrigger_DeltaX::Update( float dt, const ControlComponent & ctrl )
{
	float dx = fabs( ctrl.m_aiData.m_x - m_prevX );
	if ( dx >= SCREEN_W )
		dx -= SCREEN_W;

	m_prevX = ctrl.m_aiData.m_x;

	m_currentDx += dx;
	if ( m_currentDx >= m_targetDx )
		return 1.0f;

	return 0.0f;
}

void AITrigger_DeltaX::Reset( const ControlComponent & ctrl )
{
	m_prevX = ctrl.m_aiData.m_x;
	m_currentDx = 0;
}

float AITrigger_DeltaY::Update( float dt, const ControlComponent & ctrl )
{
	float dy = fabs( ctrl.m_aiData.m_y - m_prevY );
	if ( dy >= SCREEN_H )
		dy -= SCREEN_H;

	m_prevY = ctrl.m_aiData.m_y;

	m_currentDy += dy;
	if ( m_currentDy >= m_targetDy )
		return 1.0f;

	return 0.0f;
}

void AITrigger_DeltaY::Reset( const ControlComponent & ctrl )
{
	m_prevY = ctrl.m_aiData.m_y;
	m_currentDy = 0;
}

float AITrigger_Timer::Update( float dt, const ControlComponent & ctrl )
{
	m_currentTime -= dt;
	if ( m_currentTime <= 0 )
		return 1.0f;

	return 0.0f;
}

void AITrigger_Timer::Reset( const ControlComponent & ctrl )
{
	m_currentTime = m_startTime;
}

float AITrigger_OneTime::Update( float dt, const ControlComponent & ctrl )
{
	return 1.0f;
}

void AITrigger_OneTime::Reset( const ControlComponent & ctrl )
{
}

float AITrigger_OnCount::Update( float dt, const ControlComponent & ctrl )
{
	++m_counter;

	if ( m_counter != m_target )
		return 1.0f;

	return 0.0f;
}

void AITrigger_OnCount::Reset( const ControlComponent & ctrl )
{
	m_counter = 0;
}
