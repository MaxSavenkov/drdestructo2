#pragma once
#include "PrototypeManager.h"
#include "GraphicComponent.h"

struct SAnimProto
{
	SAnimProto()
		: m_fps( 0 )
		, m_reverse( false )
		, m_dir( 1 )
		, m_flipH( false )
		, m_flipV( false )
		, m_oneShot( false )
	{

	}

	typedef std::vector<SAnimFrame> FrameVector;
	FrameVector m_frames;
	float m_fps;
	bool m_reverse;
	int m_dir;
	bool m_flipH;
	bool m_flipV;
	bool m_oneShot;
	std::string m_soundEvent;

	void AddFrame( const SAnimFrame & frame )
	{
		m_frames.push_back( frame );
	}
};

struct SGraphicsProto
{
	SGraphicsProto()
		: m_type( GraphicComponent::TYPE_ANIMATION )
		, m_startAnim( 0 )
		, m_caps( 0 )
		, m_layerID( 0 )
	{}

	GraphicComponent::EType m_type;
	std::string m_image;
	std::string m_imageBg;
	typedef std::map<AnimationID, SAnimProto> AnimProtoMap;
	AnimProtoMap m_animations;
	AnimationID m_startAnim;
	int m_caps;
	RenderLayerID m_layerID;

	void SetAnim( const AnimationID & id, const SAnimProto & anim )
	{
		m_animations[ id ] = anim;
	}

	bool SetCaps( const int & caps )
	{
		m_caps |= caps;
		return true;
	}

	const SAnimProto *GetAnim( const AnimationID & id ) const
	{
		AnimProtoMap::const_iterator iter = m_animations.find( id );
		if ( iter != m_animations.end() )
			return &iter->second;
		return 0;
	}
};


class GraphicsProtoManager : public PrototypeManager<SGraphicsProto>
{
public:
	void LoadResources();
};

const GraphicsProtoManager & GetGraphics();
void LoadGraphics();
