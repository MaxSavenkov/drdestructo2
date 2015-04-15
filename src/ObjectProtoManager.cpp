#include "stdafx.h"
#include "ObjectProtoManager.h"
#include "XMLDeserializer.h"

class ObjectsDeserializer : public RootXMLDeserializer
{
public:
	class ObjectDeserializer : public XMLNamedObjectDeserializer<SGameObjectProto, std::string>
	{
		void Bind( SGameObjectProto & object )
		{
			m_graphics.Attrib_Value( "Name", false, object.m_graphics );
			m_physics.Attrib_Value( "Name", false, object.m_physics );
			m_controls.Attrib_Value( "Name", false, object.m_controls );
			m_mechanics.Attrib_Value( "Name", false, object.m_mechanics );
			m_sounds.Attrib_Value( "Name", false, object.m_sounds );
		}

		XMLDataDeserializer m_graphics;
		XMLDataDeserializer m_physics;
		XMLDataDeserializer m_controls;
		XMLDataDeserializer m_mechanics;
		XMLDataDeserializer m_sounds;

	public:
		ObjectDeserializer()
			: XMLNamedObjectDeserializer<SGameObjectProto, std::string>( "Proto", true, "Name" )
			, m_graphics( "Graphics", true )
			, m_physics( "Physics", true )
			, m_controls( "Controls", true )
			, m_mechanics( "Mechanics", true )
			, m_sounds( "Sounds", true )
		{
			SubDeserializer( m_graphics );
			SubDeserializer( m_physics );
			SubDeserializer( m_controls );
			SubDeserializer( m_mechanics );
			SubDeserializer( m_sounds );
		}
	} m_objectDes;

	ObjectsDeserializer()
		: RootXMLDeserializer("Protos")
	{
		SubDeserializer( m_objectDes );
	}
};

void GameObjectProtoManager::LoadResources()
{
	ObjectsDeserializer root;
	root.m_objectDes.SetReceiver<GameObjectProtoManager>( *this, &GameObjectProtoManager::AddResource );
	root.m_objectDes.SetGetter<GameObjectProtoManager>( *this, &GameObjectProtoManager::GetResource );
	XMLDeserializer des( root );
	des.Deserialize( "Data/Protos/protos.xml" );
}

GameObjectProtoManager & ModifyGameObjects()
{
	static GameObjectProtoManager m;
	return m;
}

const GameObjectProtoManager & GetGameObjects()
{
	return ModifyGameObjects();
}

void LoadGameObjects()
{
	ModifyGameObjects().LoadResources();
}
