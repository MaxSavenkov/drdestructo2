#pragma once

#include "BitmapManager.h"

class IRender;
class GraphicComponent;

struct SGroundControllerParams
{
	SGroundControllerParams()
		: m_active( false )
		, m_yFG( 0 )
		, m_yMG( 0 )
		, m_yBG( 0 )
	{}

	bool m_active;
	int m_yFG;
	int m_yMG;
	int m_yBG;
	std::string m_imageFG;
	std::string m_imageMG;
	std::string m_imageBG;
};

/*
	Actually, it's GroundRenderer. This is just a class that
	draws lower part of the screen. You can specify multiple
	images to it, to allow some nice levels.
*/
class GroundController
{
	  // If false, draw nothing
	bool m_active;
	  // Image for things that are on the foreground (player's plane go BEHIND them!), like cranes in dock levels
	ImageHandle m_imageFG;
	  // Image for things that are drawn after the target image, like dock quay
	ImageHandle m_imageMG;
	  // Images for things in background
	ImageHandle m_imageBG;
	std::string m_imageNameFG;
	std::string m_imageNameMG;
	std::string m_imageNameBG;
	  // Positions of images (maybe this should be relative?)
	int m_yFG;
	int m_yMG;
	int m_yBG;
	  // If true, images should be reloaded
	bool m_updatedImage;

public:
	GroundController()
		: m_active( false )
		, m_yFG( 0 )
		, m_yMG( 0 )
		, m_yBG( 0 )
		, m_updatedImage( false )
	{}

	void Setup( const SGroundControllerParams & params )
	{
		m_active = params.m_active;
		m_imageNameFG = params.m_imageFG;
		m_imageNameMG = params.m_imageMG;
		m_imageNameBG = params.m_imageBG;
		m_yFG = params.m_yFG;
		m_yMG = params.m_yMG;
		m_yBG = params.m_yBG;
		m_updatedImage = true;
	}

	void Update();
	void RenderBG( IRender & render, GraphicComponent & comp );
	void RenderMG( IRender & render, GraphicComponent & comp );
	void RenderFG( IRender & render, GraphicComponent & comp );
};
