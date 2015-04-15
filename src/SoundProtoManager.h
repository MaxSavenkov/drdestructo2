#pragma once
#include "PrototypeManager.h"
#include "GraphicComponent.h"

struct SSoundEventProto
{
	SSoundEventProto()
		: m_volume( 1.0f )
		, m_looped( false )
		, m_pitchFollowsAngle( false )
		, m_minPitch( 1.0f )
		, m_maxPitch( 1.0f )
	{}

	std::string m_event;
	std::string m_sound;
	float m_volume;
	bool m_looped;

	bool m_pitchFollowsAngle;
	float m_minPitch;
	float m_maxPitch;

	bool SetPitchFollowsAngle()
	{
		m_pitchFollowsAngle = true;
		return true;
	}
};

struct SSoundProto
{
	typedef std::vector<SSoundEventProto> EventList;
	EventList m_events;

	void SetEvent( const std::string & id, const SSoundEventProto & ev )
	{
		m_events.push_back( ev );
		m_events.back().m_event = id;
	}

	const SSoundEventProto *GetEvent( const std::string & id ) const
	{
		for ( int i = 0; i < (int)m_events.size(); ++i )
		{
			if ( m_events[ i ].m_event == id )
				return &m_events[ i ];
		}

		return 0;
	}
};


class SoundProtoManager : public PrototypeManager<SSoundProto>
{
public:
	void LoadResources();
};

const SoundProtoManager & GetSounds();
void LoadSounds();
