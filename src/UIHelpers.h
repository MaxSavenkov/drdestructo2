#pragma once
#include "IRender.h"
#include "IImage.h"
#include "Common.h"

/*
	Helper classes for UI rendering
*/

  // Render a portion of bitmap altas, possible stretching it
class BitmapRegion
{
public:
	void Init( ImageHandle atlas, int x, int y, int w, int h )
	{
		m_atlas = atlas;
		m_x = x;
		m_y = y;
		m_w = w;
		m_h = h;
	}

	void Render( IRender *pRender, int x, int y )
	{
		pRender->CopyRegion( m_atlas, x, y, m_x, m_y, m_w, m_h, m_w, m_h );
	}

	void StretchW( IRender *pRender, int x, int y, int w )
	{
		pRender->CopyRegion( m_atlas, x, y, m_x, m_y, m_w, m_h, w, m_h );
	}

	void StretchH( IRender *pRender, int x, int y, int h )
	{
		pRender->CopyRegion( m_atlas, x, y, m_x, m_y, m_w, m_h, m_w, h );
	}

	ImageHandle m_atlas;
	int m_x;
	int m_y;
	int m_w;
	int m_h;
};

  // Implementation of grid nine render
class Grid9
{
public:
	void Render( IRender *pRender, int x, int y, int w, int h )
	{
		m_topL.Render( pRender, x, y );
		m_topR.Render( pRender, x + w - m_topR.m_w, y );
		m_topM.StretchW( pRender, x + m_topL.m_w, y, w - m_topR.m_w - m_topL.m_w );

		m_botL.Render( pRender, x, y + h - m_botL.m_h );
		m_botR.Render( pRender, x + w - m_botR.m_w, y + h - m_botR.m_h );
		m_botM.StretchW( pRender, x + m_botL.m_w, y + h - m_botM.m_h, w - m_botR.m_w - m_botL.m_w );

		m_left.StretchH( pRender, x, y + m_topL.m_h, h - m_topL.m_h - m_botL.m_h );
		m_right.StretchH( pRender, x + w - m_right.m_w, y + m_topR.m_h, h - m_topR.m_h - m_botR.m_h );
	}

	BitmapRegion m_topL, m_topM, m_topR;
	BitmapRegion m_botL, m_botM, m_botR;
	BitmapRegion m_left, m_right;
};

  // Class for drawing window borders
class WindowBorder
{
public:
	void Init( IRender & render )
	{
		m_cornerTL	= render.LoadImage( "Data/Sprites2/ui/corner_left_top.png" );
		m_cornerTR	= render.LoadImage( "Data/Sprites2/ui/corner_right_top.png" );
		m_cornerBL	= render.LoadImage( "Data/Sprites2/ui/corner_left_bottom.png" );
		m_cornerBR	= render.LoadImage( "Data/Sprites2/ui/corner_right_bottom.png" );
		m_borderL	= render.LoadImage( "Data/Sprites2/ui/bordr_left.png" );
		m_borderR	= render.LoadImage( "Data/Sprites2/ui/bordr_rght.png" );
		m_borderT	= render.LoadImage( "Data/Sprites2/ui/bordr_top.png" );
		m_borderB	= render.LoadImage( "Data/Sprites2/ui/bordr_bottom.png" );
	}

	void Render( IRender & render )
	{
		render.DrawImage( m_cornerTL, 0, 0 );
		render.DrawImage( m_cornerTR, SCREEN_W - 128, 0 );
		render.DrawImage( m_cornerBL, 0, SCREEN_H - 128 );
		render.DrawImage( m_cornerBR, SCREEN_W - 128, SCREEN_H - 128 );
		render.StretchImage( m_borderT, 128, 0, (SCREEN_W - 128 * 2)/16.0f, 1.0f, 0 );
		render.StretchImage( m_borderB, 128, SCREEN_H - 16, (SCREEN_W - 128 * 2)/16.0f, 1.0f, 0 );
		render.StretchImage( m_borderL, 0, 128, 1.0f, (SCREEN_H - 128 * 2)/16.0f, 0 );
		render.StretchImage( m_borderR, SCREEN_W-16, 128, 1.0f, (SCREEN_H - 128 * 2)/16.0f, 0 );
	}

private:
	ImageHandle m_cornerTL;
	ImageHandle m_cornerTR;
	ImageHandle m_cornerBL;
	ImageHandle m_cornerBR;
	ImageHandle m_borderL;
	ImageHandle m_borderR;
	ImageHandle m_borderT;
	ImageHandle m_borderB;
};
