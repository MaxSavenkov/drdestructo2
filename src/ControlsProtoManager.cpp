#include "stdafx.h"
#include "ControlsProtoManager.h"
#include "XMLDeserializer.h"

ControlComponent::EType ParseControlType( const std::string & str )
{
	if ( str == "PLAYER" )
		return ControlComponent::TYPE_PLAYER;

	return ControlComponent::TYPE_AI;
}

class ControlDeserializer : public XMLNamedObjectDeserializer<SControlsProto, std::string>
{
private:
	void Bind( SControlsProto & object )
	{
		Attrib_ConvertedValue( "Type", false, XMLSchemaConvertedValueAttrib<ControlComponent::EType>::ConversionDelegate( ParseControlType ), object.m_type );
	}

public:
	ControlDeserializer()
		: XMLNamedObjectDeserializer<SControlsProto, std::string>( "Control", true, "Name")
	{
	}
};

class ControlsDeserializer : public RootXMLDeserializer
{
public:
	ControlDeserializer m_ControlDes;

	ControlsDeserializer()
		: RootXMLDeserializer( "Controls" )
	{
		SubDeserializer( m_ControlDes ); 
	}
};

void ControlsProtoManager::LoadResources()
{
	ControlsDeserializer root;
	root.m_ControlDes.SetReceiver<ControlsProtoManager>( *this, &ControlsProtoManager::AddResource );
	root.m_ControlDes.SetGetter<ControlsProtoManager>( *this, &ControlsProtoManager::GetResource );
	XMLDeserializer des( root );
	des.Deserialize( "Data/Protos/controls.xml" );	
}

ControlsProtoManager & ModifyControls()
{
	static ControlsProtoManager m;
	return m;
}

const ControlsProtoManager & GetControls()
{
	return ModifyControls();
}


void LoadControls()
{
	ModifyControls().LoadResources();
}
