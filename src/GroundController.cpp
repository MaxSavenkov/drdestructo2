#include "stdafx.h"
#include "GroundController.h"
#include "Engine/IRender.h"
#include "Common.h"

void GroundController::Update()
{

}

void GroundController::RenderFG( IRender & render, GraphicComponent & comp )
{
	if ( m_updatedImage )
	{
		render.DestroyImage( m_imageFG );
		m_imageFG = render.LoadImage( m_imageNameFG.c_str() );
		render.DestroyImage( m_imageMG );
		m_imageMG = render.LoadImage( m_imageNameMG.c_str() );
		render.DestroyImage( m_imageBG );
		m_imageBG = render.LoadImage( m_imageNameBG.c_str() );
		m_updatedImage = false;
	}

	if ( !m_active || !m_imageFG.IsValid() )
		return;

	const int fgW = render.GetImageByHandle( m_imageFG )->GetWidth();
	render.DrawImage( m_imageFG, SCREEN_W/2 - fgW/2, m_yFG );
}

void GroundController::RenderBG( IRender & render, GraphicComponent & comp )
{
	if ( m_updatedImage )
	{
		render.DestroyImage( m_imageFG );
		m_imageFG = render.LoadImage( m_imageNameFG.c_str() );
		render.DestroyImage( m_imageMG );
		m_imageMG = render.LoadImage( m_imageNameMG.c_str() );
		render.DestroyImage( m_imageBG );
		m_imageBG = render.LoadImage( m_imageNameBG.c_str() );
		m_updatedImage = false;
	}

	if ( !m_active || !m_imageBG.IsValid() )
		return;

	const int bgW = render.GetImageByHandle( m_imageBG )->GetWidth();
	render.DrawImage( m_imageBG, SCREEN_W/2 - bgW/2, m_yBG );
}

void GroundController::RenderMG( IRender & render, GraphicComponent & comp )
{
	if ( m_updatedImage )
	{
		render.DestroyImage( m_imageFG );
		m_imageFG = render.LoadImage( m_imageNameFG.c_str() );
		render.DestroyImage( m_imageMG );
		m_imageMG = render.LoadImage( m_imageNameMG.c_str() );
		render.DestroyImage( m_imageBG );
		m_imageBG = render.LoadImage( m_imageNameBG.c_str() );
		m_updatedImage = false;
	}

	if ( !m_active || !m_imageMG.IsValid() )
		return;

	const int mgW = render.GetImageByHandle( m_imageMG )->GetWidth();
	render.DrawImage( m_imageMG, SCREEN_W/2 - mgW/2, m_yMG );
}
