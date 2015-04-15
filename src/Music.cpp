#include "stdafx.h"
#include "Music.h"
#include "XMLDeserializer.h"

MusicComponent::MusicComponent()
{
	m_fadeTimer = 0.0f;
	m_fadeMode = FADE_NONE;
}

/*
	This class deserialized description of music events from file
*/
class MusicDeserializer : public RootXMLDeserializer
{
	class MusicEventDeserializer : public XMLCustomDeserializer
	{
		MusicComponent & m_comp;

	public:
		MusicEventDeserializer( MusicComponent & comp )
			: XMLCustomDeserializer( "Event", true )
			, m_comp( comp )
		{
		}

		bool CustomDeserialize( const TiXmlElement *pElement )
		{
			std::string id, path;
			if ( !GetAttributeValue( "ID", pElement, id ) )
				return false;

			if ( !GetAttributeValue( "Path", pElement, path ) )
				return false;

			m_comp.AddMusicToEvent( id, path );

			return true;
		}

	} m_eventDes;


public:
	MusicDeserializer( MusicComponent & comp )
		: RootXMLDeserializer( "Music" )
		, m_eventDes( comp )
	{
		SubDeserializer( m_eventDes );
	}
};

bool MusicComponent::Load( const std::string & path )
{
	m_events.clear();

	MusicDeserializer root( *this );
	XMLDeserializer des( root );
	des.Deserialize( path );
	
	return true;
}

void MusicComponent::ReceiveEvent( const std::string & e )
{
	m_nextEvent = e;
}

void MusicComponent::Update( float dt )
{
	  // Process fades
	if ( m_fadeTimer > 0.0f )
	{
		m_fadeTimer -= dt;
		if ( m_fadeTimer < 0 )
			m_fadeTimer = 0.0f;
	}

	  // If we have a new event...
	if ( !m_nextEvent.empty() )
	{
		  // Find event's description
		EventMap::iterator iter = m_events.find( m_nextEvent );
		if ( iter != m_events.end() )
		{
			SMusicEvent & e = iter->second;
			e.Next();
			const char *song = e.Current();

			  // If we really want to change tune...
			if ( song && song != m_currentSong )
			{
				m_nextSong = song ? song : "";

				  // If we have a song playing, fade it out
				if ( m_song.IsPlaying() )
				{
					m_fadeTimer = FADE_TIME;
					m_fadeMode = FADE_OUT;
				}
				else // if we have no song playing at the time, start fade in for the next song
				{
					m_fadeTimer = FADE_TIME;
					m_fadeMode = FADE_IN;
					m_song.Load( m_nextSong.c_str() );
					m_song.SetLoopMode( ISample::LOOPMODE_LOOP );
					m_song.SetVolume( 0.0f );
					m_song.Play();
					m_currentSong = song;
				}
			}
		}
		else // if not such event is found, fade out current music into silence
		{
			m_fadeTimer = FADE_TIME;
			m_fadeMode = FADE_OUT;
			m_nextSong = "";		
		}

		m_nextEvent = "";
	}

	  // Process fade out
	if ( m_fadeMode == FADE_OUT )
	{
		  // Decrease volume
		m_song.SetVolume( (m_fadeTimer / FADE_TIME)*1.0f );
		  
		  // If fade out is complete
		if ( m_fadeTimer <= 0.0f )
		{
			  // Stop song completely
			m_song.Stop();
			m_fadeMode = FADE_NONE;

			  // If we have next song - start fade in for it
			if ( !m_nextSong.empty() )
			{
				m_fadeTimer = FADE_TIME;
				m_fadeMode = FADE_IN;
				
				m_song.Load( m_nextSong.c_str() );
				m_song.SetLoopMode( ISample::LOOPMODE_LOOP );
				m_song.Play();
				m_currentSong = m_nextSong;
			}			
		}
	}

	  // Process fade in
	if ( m_fadeMode == FADE_IN )
	{
		  // Increase volume
		m_song.SetVolume( (1.0f - m_fadeTimer / FADE_TIME)*1.0f );
		if ( m_fadeTimer <= 0.0f )
		{
			m_fadeMode = FADE_NONE;
		}
	}
}
