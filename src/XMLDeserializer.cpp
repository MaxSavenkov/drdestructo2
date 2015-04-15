#include "stdafx.h"
#include "XMLDeserializer.h"

//#include "GraphicComponent.h"
//
//struct SAnimProto
//{
//	SAnimProto()
//		: m_fps( 0 )
//		, m_reverse( false )
//		, m_dir( 1 )
//		, m_flipH( false )
//		, m_flipV( false )
//	{
//		
//	}
//
//	typedef std::vector<SAnimFrame> FrameVector;
//	FrameVector m_frames;
//	float m_fps;
//	bool m_reverse;
//	int m_dir;
//	bool m_flipH;
//	bool m_flipV;
//
//	void AddFrame( const SAnimFrame & frame )
//	{
//		m_frames.push_back( frame );
//	}
//};
//
//struct SGraphProto
//{
//	GraphicComponent::EType m_type;
//	std::string m_image;
//	typedef std::map<AnimationID, SAnimProto> AnimProtoMap;
//	AnimProtoMap m_animations;
//	AnimationID m_startAnim;
//
//	void SetAnim( const AnimationID & id, const SAnimProto & anim )
//	{
//		m_animations[ id ] = anim;
//	}
//
//	const SAnimProto *GetAnim( const AnimationID & id )
//	{
//		AnimProtoMap::const_iterator iter = m_animations.find( id );
//		if ( iter != m_animations.end() )
//			return &iter->second;
//		return 0;
//	}
//};
//
//struct Test
//{
//	struct GraphProtoContext
//	{
//		std::map<std::string, SGraphProto> m_protos;
//
//		void Receive( const std::string & protoName, const SGraphProto & proto )
//		{
//			m_protos[ protoName ] = proto;
//		}
//
//		const SGraphProto *Get( const std::string & id ) const
//		{
//			std::map<std::string, SGraphProto>::const_iterator iter = m_protos.find( id );
//			if ( iter != m_protos.end() )
//				return &iter->second;
//			return 0;
//		}
//	};
//
//	class GraphicDeserializer : public XMLNamedObjectDeserializer<SGraphProto, std::string>
//	{
//		class AnimDeserializer : public XMLNamedObjectDeserializer<SAnimProto, AnimationID>
//		{
//			class FrameDeserializer : public XMLObjectDeserializer<SAnimFrame>
//			{
//			public:
//				FrameDeserializer() : XMLObjectDeserializer( "Frame", false )
//				{}
//
//				void Bind( SAnimFrame & object )
//				{
//					Attrib_Value( "X", false, object.x );
//					Attrib_Value( "Y", false, object.y );
//					Attrib_Value( "W", true, object.w );
//					Attrib_Value( "H", true, object.h );
//					Attrib_SetterValue<SAnimFrame, int>( "X2", true, object, &SAnimFrame::SetX2 );
//					Attrib_SetterValue<SAnimFrame, int>( "Y2", true, object, &SAnimFrame::SetY2 );
//				}
//			}m_frameDes;
//
//		public:
//			AnimDeserializer()
//				: XMLNamedObjectDeserializer( "Animation", false, "ID" )
//			{
//				SubDeserializer( m_frameDes );
//			}
//
//			void Bind( SAnimProto & object )
//			{
//				Attrib_Value( "FPS", false, object.m_fps );
//				Attrib_Value( "Dir", true, object.m_dir );
//				Attrib_Value( "Reverse", true, object.m_reverse );
//				Attrib_Value( "FlipV", true, object.m_flipV );
//				Attrib_Value( "FlipH", true, object.m_flipH );
//				m_frameDes.SetReceiver( object, &SAnimProto::AddFrame );
//			}
//		};
//
//	private:
//		XMLDataDeserializer m_imgDes;
//		AnimDeserializer m_animDes;
//
//		void Bind( SGraphProto & object )
//		{
//			m_animDes.SetReceiver( object, &SGraphProto::SetAnim );
//			m_animDes.SetGetter<SGraphProto>( object, &SGraphProto::GetAnim );
//			m_imgDes.Attrib_Value( "Path", false, object.m_image );
//		}
//
//	public:
//		GraphicDeserializer( GraphProtoContext & ctx )
//			: XMLNamedObjectDeserializer( "Graphic", true, "Name", 
//					XMLNamedObjectDeserializer<SGraphProto, std::string>::TReceiveDelegate( &ctx, &GraphProtoContext::Receive ),
//					XMLNamedObjectDeserializer<SGraphProto, std::string>::TGetDelegate( &ctx, &GraphProtoContext::Get ) )
//			, m_imgDes( "Image", false )
//		{
//			SubDeserializer( m_imgDes ); 
//			SubDeserializer( m_animDes ); 
//		}
//	};
//
//	class GraphicsDeserializer : public RootXMLDeserializer
//	{
//		GraphicDeserializer m_graphicDes;
//	public:
//		GraphicsDeserializer( GraphProtoContext & ctx )
//			: RootXMLDeserializer( "Graphics" )
//			, m_graphicDes( ctx )
//		{
//			SubDeserializer( m_graphicDes ); 
//		}
//	};
//
//	Test()
//	{
//		GraphProtoContext ctx;
//		GetVars().Load("Data/protos/vars.xml");
//		GraphicsDeserializer root( ctx );
//		XMLDeserializer des( root );
//		des.Deserialize( "Data/Protos/graphics.xml" );	
//	}
//}t;
