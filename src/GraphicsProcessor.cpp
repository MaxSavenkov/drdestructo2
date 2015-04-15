#include "stdafx.h"
#include "GraphicsProcessor.h"
#include "GraphicComponent.h"
#include "IRender.h"
#include "IImage.h"
#include "Vars.h"

UseIntVar GRAPH_CAPS_TRAIL("GRAPH_CAPS_TRAIL");

  // Updates animation frames and other states
void GraphicsProcessor::Update( float dt, GraphicComponent & comp )
{
	if ( comp.m_invulnerabile )
	{
		if ( comp.m_blinkTimer > -0.1f )
			comp.m_blinkTimer -= dt;
		else
			comp.m_blinkTimer = 0.1f;
	}

	if ( comp.m_renderProc )
		return;

	if ( !comp.m_animations.empty() )
	{
		GraphicComponent::AnimMap::iterator iter = comp.m_animations.find( comp.m_currentAnim );
		if ( iter != comp.m_animations.end() )
		{
			  // Calculate next frame
			Animation & anim = iter->second;
			const float spf = 1.0f / anim.m_fps;
			anim.m_time += dt;
			while( anim.m_time >= spf )
			{
				anim.m_time -= spf;
				anim.m_currentFrame += anim.m_dir;
				if ( anim.m_oneShot )
				{
					if ( anim.m_currentFrame >= (int)anim.m_frames.size() )
						anim.m_currentFrame = (int)anim.m_frames.size() - 1;
					if ( anim.m_currentFrame < 0 )
						anim.m_currentFrame = 0;
				}
				else
				{
					if ( anim.m_reverse )
					{
						if ( anim.m_currentFrame >= (int)anim.m_frames.size() )
						{
							anim.m_dir = -anim.m_dir;
							anim.m_currentFrame = (int)anim.m_frames.size() - 2;
						}
						if ( anim.m_currentFrame < 0 )
						{
							anim.m_dir = -anim.m_dir;
							anim.m_currentFrame = 1;
						}
					}
					else
					{
						if ( anim.m_currentFrame >= (int)anim.m_frames.size() )
						{
							anim.m_currentFrame = 0;
						}
						if ( anim.m_currentFrame < 0 )
						{
							anim.m_currentFrame = (int)anim.m_frames.size() - 1;
						}
					}
				}
			}
		}
	}
}

  // Draws component on screen
void GraphicsProcessor::Render( IRender & render, GraphicComponent & comp )
{
	if ( comp.m_invulnerabile )
	{
		if ( comp.m_blinkTimer < 0 )
			return;
	}

	  // If component has render procedure specified, do not use standard logic
	if ( comp.m_renderProc )
	{
		comp.m_renderProc( render, comp );
	}
	else if ( comp.m_animations.empty() )
	{
		  // Draw static images
		IImage *pImg = render.GetImageByHandle( comp.m_image );
		const int w = pImg->GetWidth();
		const int h = pImg->GetHeight();
		const int x = comp.m_x;
		const int y = comp.m_y;

		if ( comp.m_staticBg != ImageHandle::INVALID )
			render.DrawImage( comp.m_staticBg, x, y, comp.m_angle );

		render.DrawImage( comp.m_image, x, y, comp.m_angle );

		if ( (comp.m_caps & GRAPH_CAPS_TRAIL) )
		{
			const float cosa = cosf( -comp.m_angle );
			const float sina = -sinf( -comp.m_angle );
			const int TRAIL_LENGTH = 3;
			//int xx = x+w/2;
			//int yy = y+h/2;
			//render.DrawLine( xx, yy, xx - cosa*10, yy-sina*10, Color(255,0,0));
			for ( int i = 0; i < TRAIL_LENGTH; ++i )
			{

				const int xx = x - i * cosa * 20;
				const int yy = y - i * sina * 20;

				const int t = (TRAIL_LENGTH-i)/(float)TRAIL_LENGTH * 255;

				render.DrawTintedImage( comp.m_image, Color( t, t, t, t ), xx, yy, w/2, h/2, comp.m_angle, 0, 0, w, h, 0 );
			}
		}
		//render.DrawCircle( comp.m_x, comp.m_y, 3, Color(255,255,25));
	}
	else
	{
		  // Draw animated images
		GraphicComponent::AnimMap::iterator iter = comp.m_animations.find( comp.m_currentAnim );
		if ( iter != comp.m_animations.end() )
		{
			Animation & anim = iter->second;

			if ( anim.m_currentFrame >= 0 && anim.m_currentFrame < (int)anim.m_frames.size() )
			{
				const SAnimFrame & frame = anim.m_frames[ anim.m_currentFrame ];
				int flags = 0;
				if ( anim.m_flipH || frame.flipH ) flags |= ALLEGRO_FLIP_HORIZONTAL;
				if ( anim.m_flipV || frame.flipV ) flags |= ALLEGRO_FLIP_VERTICAL;
				//if ( comp.m_staticBg != ImageHandle::INVALID )
				//	render.DrawImage( comp.m_staticBg, comp.m_x-w/2, comp.m_y-h/2, comp.m_angle );
				render.DrawImage( comp.m_image, comp.m_x, comp.m_y, frame.x, frame.y, frame.w, frame.h, comp.m_angle, flags );
				if ( comp.m_debugFlag )
					render.DrawCircle( comp.m_x, comp.m_y, 50, Color(255,0,0) );
				//int dx = 50 - 100 * ((comp.GetID().GetIndex() % 4)/2);
				//int dy = 50 - 100 * ((comp.GetID().GetIndex() % 4)%2);
				//render.DrawText( comp.m_x + dx, comp.m_y + dy, Color(255,255,255), 16, "%i %f", comp.m_currentAnim, comp.m_angle );
			}
		}
		else
		{
			//render.DrawCircle( comp.m_x, comp.m_y, 30, Color(255,255,25));
			//render.DrawText( comp.m_x - 5, comp.m_y - 5, Color(255,255,255), 40, "%i", comp.m_currentAnim );
		}
	}
}
