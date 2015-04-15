#pragma once
#include "Engine/ISound.h"
#include "Random.h"

/*
	There should be only one copy of this component.
	It's not event a BaseComponent, but just a Component.
	It's used to keep track of current music.
*/
class MusicComponent
{
	  // Constant fade-out/fade-in time between tracks
	static const int FADE_TIME = 1;

	  // Description of reaction to a music event
	struct SMusicEvent
	{
		SMusicEvent()
			: m_position( 0 )
		{}

		  // All music tracks for this event (could be more than one to provide a little variety)
		typedef std::vector<std::string> MusicList;
		MusicList m_list;
		  // Currently played track index
		size_t m_position;

		  // Changes currently playing track to next
		void Next()
		{
			m_position = ( m_position + 1 ) % ( m_list.size() );
		}

		  // Returns name of current track
		const char *Current() const
		{
			return ( m_position < 0 || m_position >= (int)m_list.size() ) ? 0 : m_list[ m_position ].c_str(); 
		}

		  // Adds new track to list at the end
		void Add( const std::string & song )
		{
			m_list.push_back( song );
		}

		  // Adds new track to list at random place
		void AddRandomly( const std::string & song )
		{
			m_list.insert( m_list.begin() + RndInt( 0, (int)m_list.size() ), song );
		}
	};

	  // Current fade action
	enum EFadeMode
	{
		FADE_NONE,
		FADE_IN,
		FADE_OUT,
	};

	  // Hash-map Event name -> Event description
#ifndef HAS_STD_SHARED_PTR
	typedef boost::unordered_map<std::string, SMusicEvent> EventMap;
#else
	typedef std::unordered_map<std::string, SMusicEvent> EventMap;
#endif
	EventMap m_events;

	  // Currently playing track
	MusicInstance m_song;
	  // Event to be processed
	std::string m_nextEvent;
	  // The next song after fade
	std::string m_nextSong;
	  // Current song name
	std::string m_currentSong;
	  // Current fade timer
	float m_fadeTimer;
	  // Current fade action
	EFadeMode m_fadeMode;

public:
	MusicComponent();
	  
	  // Loads music events descriptions from file
	bool Load( const std::string & path );
	  // Use this to pass events to this component. Overwrites current event: there is no queue!
	void ReceiveEvent( const std::string & e );
	  // Changes tracks if needed
	void Update( float dt );

	void AddMusicToEvent( const std::string & ev, const std::string & path )
	{
		m_events[ ev ].AddRandomly( path );
	}
};
