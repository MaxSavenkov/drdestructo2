#pragma once
#include "ComponentID.h"
#include "BaseComponent.h"
#include "IRender.h"
#include "IImage.h"
#include "FastDelegate/FastDelegate.h"

/*
	Description of animation frame. It's coordinates and dimensions in texture
	and flags for flipping (to allow frame reuse)
*/
struct SAnimFrame
{
	SAnimFrame()
		: x(0)
		, y(0)
		, w(0)
		, h(0)
		, flipH( false )
		, flipV( false )
	{}

	SAnimFrame( int _x, int _y, int _w, int _h, bool _flipH = false, bool _flipV = false )
		: x(_x)
		, y(_y)
		, w(_w)
		, h(_h)
		, flipH( _flipH )
		, flipV( _flipV )
	{}

	int x,y,w,h;
	bool flipH, flipV;

	  // Functions to set width & height via providing right and bottom coordinates
	bool SetX2( const int & x2 ) { w = x2 - x + 1; return true; }
	bool SetY2( const int & y2 ) { h = y2 - y + 1; return true; }
};

/*
	Single animation
*/
class Animation
{
public:
	Animation()
		: m_currentFrame( 0 )
		, m_time( 0 )
	{}

	  // Vector of frames
	typedef std::vector<SAnimFrame> FrameVector;
	FrameVector m_frames;
	  // Current frame index
	int m_currentFrame;
	  // Current animation time
	float m_time;
	  // Frames per second
	float m_fps;
	  // If true, animation would play and ping-pong fashion instead of simply looping
	bool m_reverse;
	  // Current playing direction
	int m_dir;
	  // Flags for flipping animation (to allow whole animations reuse)
	bool m_flipH;
	bool m_flipV;
	  // If true, animation is not looped
	bool m_oneShot;
	  // Sound event to be dispatched when animation starts
	std::string m_soundEvent;
};

  // Animation identifier
typedef int AnimationID;
const AnimationID INVALID_ANIM = -1;

  // Render layer identifier (Z-Order of a kind)
typedef int RenderLayerID;
const RenderLayerID INVALID_LAYER = -1;

/*
	Graphic component which decided how object is drawn
	and holds current graphic state.
*/
class GraphicComponent : public SubComponent<GraphicComponent, GameObject>
{
public:
	GraphicComponent()
		: m_layerUpdated( true )
		, m_renderLayer( 0 )
		, m_currentAnim( INVALID_ANIM )
		, m_nextAnim( INVALID_ANIM )
		, m_caps( 0 )
	{}

	GraphicComponent( const GraphicCompID & id )
		: SubComponent<GraphicComponent, GameObject>( id )
		, m_layerUpdated( true )
		, m_renderLayer( 0 )
		, m_currentAnim( INVALID_ANIM )
		, m_nextAnim( INVALID_ANIM )
		, m_caps( 0 )
		, m_debugFlag( false )
		, m_invulnerabile( false )
		, m_blinkTimer( 0 )
	{}

	enum EType
	{
		TYPE_STATIC = 1,
		TYPE_ANIMATION = 2,
	};

	  // Type
	EType m_type;
	  // List of flags (see GraphicProcessor)
	int m_caps;
	  // Texture used for this Component
	ImageHandle m_image;
	  // Texture used for restoring image after displa was lost
	ImageHandle m_cache;
	  // Additional texture used for this Component. Isn't animated. Used for target (ships, castles etc.) background sprites
	ImageHandle m_staticBg;
      // Map of animations
	typedef std::map<AnimationID, Animation> AnimMap;
	AnimMap m_animations;
	  // Current animation ID
	AnimationID m_currentAnim;
	  // Next animation ID (see GraphicProcessor)
	AnimationID m_nextAnim;
	  // Positioning
	float m_x;
	float m_y;
	float m_angle;
	  // If true, object will blink
	bool m_invulnerabile;
	  // Timer used for blinking
	float m_blinkTimer;

	  // If true... Actually, you can use it in any way you want
	bool m_debugFlag;

	  // If set, instead of standard animation function this delegate would be used.
	  // Currently only used for drawing sky & reflection in levels
	typedef fastdelegate::FastDelegate<void ( IRender & render, GraphicComponent & comp )> RenderDelegate;
	RenderDelegate m_renderProc;

	  // Checks whether render layer of object has changed and clears that flag
	bool CheckLayerUpdated() { bool ret = m_layerUpdated; m_layerUpdated = false; return ret; }
	  // Returns current object's render layer
	RenderLayerID GetLayerID() const { return m_renderLayer; }

private:
	  // Render layer (indicates order in which objects are drawn)
	  // Multiple objects can share render layer, in which case their order of drawing
	  // is determined by time of creation (i.e. you just don't care about it)
	RenderLayerID m_renderLayer;
	bool m_layerUpdated;

public:
	void SetLayer( RenderLayerID id )
	{
		if ( id != m_renderLayer )
		{
			m_layerUpdated = true;
			m_renderLayer = id;
		}
	}

	int GetFrameW() const
	{
		if ( m_animations.empty() )
			return GetRender().GetImageByHandle( m_image )->GetWidth();

		AnimMap::const_iterator iter = m_animations.find( m_currentAnim );
		if ( iter == m_animations.end() )
			return 0;

		const Animation & anim = iter->second;

		if ( anim.m_currentFrame < 0 || anim.m_currentFrame >= (int)anim.m_frames.size() )
			return 0;

		return anim.m_frames[ anim.m_currentFrame ].w;
	}

	int GetFrameH() const
	{
		if ( m_animations.empty() )
			return GetRender().GetImageByHandle( m_image )->GetHeight();

		AnimMap::const_iterator iter = m_animations.find( m_currentAnim );
		if ( iter == m_animations.end() )
			return 0;

		const Animation & anim = iter->second;

		if ( anim.m_currentFrame < 0 || anim.m_currentFrame >= (int)anim.m_frames.size() )
			return 0;

		return anim.m_frames[ anim.m_currentFrame ].h;
	}
};
