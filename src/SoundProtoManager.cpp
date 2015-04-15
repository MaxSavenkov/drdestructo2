#include "stdafx.h"
#include "SoundProtoManager.h"

#include "XMLDeserializer.h"

class SoundDeserializer : public XMLNamedObjectDeserializer<SSoundProto, std::string>
{
	class EventDeserializer : public XMLNamedObjectDeserializer<SSoundEventProto, std::string>
	{		
		XMLDataDeserializer m_pitchDes;

	public:
		EventDeserializer()
			: XMLNamedObjectDeserializer<SSoundEventProto, std::string>( "Event", false, "ID" )
			, m_pitchDes( "PitchFollowsAngle", true )
		{
			SubDeserializer( m_pitchDes );
		}

		void Bind( SSoundEventProto & object )
		{
			Attrib_Value( "Path", false, object.m_sound );
			Attrib_Value( "Volume", true, object.m_volume );
			Attrib_Value( "Looped", true, object.m_looped );
			
			m_pitchDes.Attrib_Value( "MinPitch", false, object.m_minPitch );
			m_pitchDes.Attrib_Value( "MaxPitch", false, object.m_maxPitch );
			m_pitchDes.Finisher( XMLDataDeserializer::DataFinisherDelegate( &object, &SSoundEventProto::SetPitchFollowsAngle ) );
		}
	};

private:
	EventDeserializer m_eventDes;

	void Bind( SSoundProto & object )
	{
		m_eventDes.SetReceiver( object, &SSoundProto::SetEvent );
		m_eventDes.SetGetter<SSoundProto>( object, &SSoundProto::GetEvent );
	}

public:
	SoundDeserializer()
		: XMLNamedObjectDeserializer<SSoundProto, std::string>( "Sound", true, "Name")
	{
		SubDeserializer( m_eventDes ); 
	}
};

class SoundsDeserializer : public RootXMLDeserializer
{
public:
	SoundDeserializer m_soundDes;

	SoundsDeserializer()
		: RootXMLDeserializer( "Sounds" )
	{
		SubDeserializer( m_soundDes ); 
	}
};

void SoundProtoManager::LoadResources()
{
	SoundsDeserializer root;
	root.m_soundDes.SetReceiver<SoundProtoManager>( *this, &SoundProtoManager::AddResource );
	root.m_soundDes.SetGetter<SoundProtoManager>( *this, &SoundProtoManager::GetResource );
	XMLDeserializer des( root );
	des.Deserialize( "Data/Protos/sounds.xml" );	
}

SoundProtoManager & ModifySounds()
{
	static SoundProtoManager m;
	return m;
}

const SoundProtoManager & GetSounds()
{
	return ModifySounds();
}


void LoadSounds()
{
	ModifySounds().LoadResources();
}
