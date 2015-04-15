#pragma once
#include "BaseComponent.h"
#include "ISound.h"

/*
	Description of sound effect of object
*/
struct SSound
{
	SSound()
		: m_pitchFollowsAngle( false )
		, m_minPitch( 0.0f )
		, m_maxPitch( 0.0f )
	{}

	SSound( const SSound & other )
	{
		if ( this != &other )
			*this = other;
	}

	  // For playing sound
	SoundInstance m_sound;

	  // If true, pitch will change along with angle (not used for anything, but player's engine, and it's very quitet because it's annoying)
	bool m_pitchFollowsAngle;
	  // Min and max values for sound's pitch
	float m_minPitch;
	float m_maxPitch;
};

/*
	This component describes all posible sound effects for object
	and stores sound events to be processed
*/
class SoundComponent : public SubComponent<SoundComponent, GameObject> 
{
public:
	  // Sound events queue
	typedef std::vector<std::string> EventQueue;
	EventQueue m_events;

	  // Hash-Map Sound ID -> Sound description
#ifndef HAS_STD_SHARED_PTR
	typedef boost::unordered_map<std::string, SSound> SoundMap;
#else
	typedef std::unordered_map<std::string, SSound> SoundMap;
#endif
	SoundMap m_sounds;

	  // Current object's angle (copied from Physic component)
	float m_angle;

	SoundComponent()
		: m_angle( 0.0f )
	{}

	SoundComponent( const SoundCompID & id )
		: SubComponent<SoundComponent, GameObject>( id )
		, m_angle( 0.0f )
	{}

	void PushEvent( const std::string & event )
	{
		m_events.push_back( event );
	}
};
