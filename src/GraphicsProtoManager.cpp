#include "stdafx.h"
#include "GraphicsProtoManager.h"
#include "XMLDeserializer.h"

class GraphicDeserializer : public XMLNamedObjectDeserializer<SGraphicsProto, std::string>
{
	class AnimDeserializer : public XMLNamedObjectDeserializer<SAnimProto, AnimationID>
	{
		class FrameDeserializer : public XMLObjectDeserializer<SAnimFrame>
		{
		public:
			FrameDeserializer() : XMLObjectDeserializer<SAnimFrame>( "Frame", false )
			{}

			void Bind( SAnimFrame & object )
			{
				Attrib_Value( "X", false, object.x );
				Attrib_Value( "Y", false, object.y );
				Attrib_Value( "W", true, object.w );
				Attrib_Value( "H", true, object.h );
				Attrib_Value( "FlipH", true, object.flipH );
				Attrib_Value( "FlipV", true, object.flipV );
				Attrib_SetterValue<SAnimFrame, int>( "X2", true, object, &SAnimFrame::SetX2 );
				Attrib_SetterValue<SAnimFrame, int>( "Y2", true, object, &SAnimFrame::SetY2 );
			}
		}m_frameDes;

	public:
		AnimDeserializer()
			: XMLNamedObjectDeserializer<SAnimProto, AnimationID>( "Animation", false, "ID" )
		{
			SubDeserializer( m_frameDes );
		}

		void Bind( SAnimProto & object )
		{
			Attrib_Value( "FPS", false, object.m_fps );
			Attrib_Value( "Dir", true, object.m_dir );
			Attrib_Value( "Reverse", true, object.m_reverse );
			Attrib_Value( "FlipV", true, object.m_flipV );
			Attrib_Value( "FlipH", true, object.m_flipH );
			Attrib_Value( "OneShot", true, object.m_oneShot );
			Attrib_Value( "SoundEvent", true, object.m_soundEvent );
			m_frameDes.SetReceiver( object, &SAnimProto::AddFrame );
		}
	};

private:
	XMLDataDeserializer m_imgDes;
	XMLDataDeserializer m_bgDes;
	XMLDataDeserializer m_capsDes;
	AnimDeserializer m_animDes;

	void Bind( SGraphicsProto & object )
	{
		Attrib_Value( "Layer", false, object.m_layerID );
		m_animDes.SetReceiver( object, &SGraphicsProto::SetAnim );
		m_animDes.SetGetter<SGraphicsProto>( object, &SGraphicsProto::GetAnim );
		m_imgDes.Attrib_Value( "Path", false, object.m_image );
		m_bgDes.Attrib_Value( "Path", false, object.m_imageBg );
		m_capsDes.Attrib_SetterValue<SGraphicsProto, int>( "ID", false, object, &SGraphicsProto::SetCaps );
	}

public:
	GraphicDeserializer()
		: XMLNamedObjectDeserializer<SGraphicsProto, std::string>( "Graphic", true, "Name")
		, m_imgDes( "Image", false )
		, m_bgDes( "Bg", true )
		, m_capsDes( "Caps", false )
	{
		SubDeserializer( m_imgDes ); 
		SubDeserializer( m_bgDes ); 
		SubDeserializer( m_animDes ); 
		SubDeserializer( m_capsDes ); 
	}
};

class GraphicsDeserializer : public RootXMLDeserializer
{
public:
	GraphicDeserializer m_graphicDes;

	GraphicsDeserializer()
		: RootXMLDeserializer( "Graphics" )
	{
		SubDeserializer( m_graphicDes ); 
	}
};

void GraphicsProtoManager::LoadResources()
{
	GraphicsDeserializer root;
	root.m_graphicDes.SetReceiver<GraphicsProtoManager>( *this, &GraphicsProtoManager::AddResource );
	root.m_graphicDes.SetGetter<GraphicsProtoManager>( *this, &GraphicsProtoManager::GetResource );
	XMLDeserializer des( root );
	des.Deserialize( "Data/Protos/graphics.xml" );	
}

GraphicsProtoManager & ModifyGraphics()
{
	static GraphicsProtoManager m;
	return m;
}

const GraphicsProtoManager & GetGraphics()
{
	return ModifyGraphics();
}


void LoadGraphics()
{
	ModifyGraphics().LoadResources();
}
