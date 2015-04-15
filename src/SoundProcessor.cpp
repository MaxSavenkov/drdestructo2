#include "stdafx.h"
#include "SoundProcessor.h"
#include "SoundComponent.h"

void SoundProcessor::Update( float dt, SoundComponent & comp )
{
	while( !comp.m_events.empty() )
	{
		const std::string & ev = comp.m_events.front();

		SoundComponent::SoundMap::iterator iter = comp.m_sounds.find( ev );
		if ( iter != comp.m_sounds.end() )
		{
			iter->second.m_sound.Play();
		}

		comp.m_events.erase( comp.m_events.begin() );
	}

	for ( SoundComponent::SoundMap::iterator iter = comp.m_sounds.begin();
		  iter != comp.m_sounds.end();
		  ++iter )
	{
		SSound & s = iter->second;
		if ( s.m_pitchFollowsAngle && s.m_sound.IsPlaying() )
		{
			float newPitch = 1.0f + sinf( -comp.m_angle ) * (s.m_maxPitch - s.m_minPitch);
			s.m_sound.SetPitch( newPitch );
		}
	}
}
