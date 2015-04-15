#include "stdafx.h"
#include "ReflectionController.h"
#include "Common.h"
#include "Vars.h"

namespace
{
	RegisterVariable<bool> DebugReflectionLine( "DebugReflectionLine", false );
}

void ReflectionController::Setup( const SReflectionControllerParams & params )
{
	m_active = params.m_active;
	m_reflectionPercent = params.m_reflectionPercent;
	m_beforeAfterY = params.m_beforeAfterY;
}

void ReflectionController::Update( IRender & render, ImageHandle bgImg )
{
	if ( !m_active )
		return;

	if ( !m_image.IsValid() )
		m_image = render.CreateImage( SCREEN_W, SCREEN_H * m_reflectionPercent, true, "reflect" );

	render.CopyRegion( m_image, bgImg, 0, 0, 0, 0, SCREEN_W, SCREEN_H*(1.0f-m_reflectionPercent), SCREEN_W, SCREEN_H*m_reflectionPercent );
}

void ReflectionController::RenderBefore( IRender & render, GraphicComponent & comp )
{
	if ( !m_active )
		return;

	const int y = SCREEN_H * ( 1.0f - m_reflectionPercent );
	render.DrawTintedImage( m_image, Color(150,150,200), 0, y, 0, SCREEN_H*m_reflectionPercent - m_beforeAfterY, SCREEN_W, m_beforeAfterY, ALLEGRO_FLIP_VERTICAL );	
}

void ReflectionController::RenderAfter( IRender & render, GraphicComponent & comp )
{
	if ( !m_active )
		return;

	const int y = SCREEN_H * ( 1.0f - m_reflectionPercent ) + m_beforeAfterY;
	render.DrawTintedImage( m_image, Color(150,150,200), 0, y, 0, 0, SCREEN_W, SCREEN_H*m_reflectionPercent - m_beforeAfterY, ALLEGRO_FLIP_VERTICAL );

	if ( DebugReflectionLine )
		render.DrawLine( 0, y, SCREEN_W, y, Color(255,255,255) );
}
