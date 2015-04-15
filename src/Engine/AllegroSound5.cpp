#include "stdafx.h"
#include "Engine/AllegroSound5.h"
#include "Logger.h"
#include "LogCommon.h"

/*
	The only thing to note here is difference between Sound and Music.
	Also, remember that SetPitch actually sets SPEED of sample.
*/

class SampleManager
{
#ifndef HAS_STD_SHARED_PTR
	typedef boost::unordered_map<std::string, SamplePtr > SampleMap;
#else
	typedef std::unordered_map<std::string, SamplePtr > SampleMap;
#endif

	SampleMap m_samples;

public:
	SamplePtr Load( const std::string & path )
	{
		SampleMap::iterator iter = m_samples.find( path );
		if ( iter != m_samples.end() )
			return iter->second;

		SamplePtr s( new SSample( path, al_load_sample( path.c_str() ) ), *this );
		if ( s )
			m_samples.insert( std::make_pair( path, s ) );

		return s;
	}

	void operator ()( SSample *sample )
	{
		m_samples.erase( sample->m_key );
		al_destroy_sample( sample->m_pSample );
		delete sample;
	}
};

SampleManager & GetSamples()
{
	static SampleManager manager;
	return manager;
}

AllegroSoundSample5::~AllegroSoundSample5()
{
	if ( m_pInstance )
	{
		al_detach_sample_instance( m_pInstance );
		al_destroy_sample_instance( m_pInstance );
	}
}

bool AllegroSoundSample5::Load( const std::string & path )
{
	if ( m_pInstance )
	{
		al_detach_sample_instance( m_pInstance );
		al_destroy_sample_instance( m_pInstance );
	}

	SamplePtr pSampleData = GetSamples().Load( path );
	if ( !pSampleData || !pSampleData->m_pSample )
		return false;

	m_pInstance = al_create_sample_instance( pSampleData->m_pSample );
	al_attach_sample_instance_to_mixer( m_pInstance,  ((AllegroSound5&)GetSound()).GetSoundMixer() );

	return ( m_pInstance != 0 );
}

bool AllegroSoundSample5::Play()
{
	if ( !m_pInstance )
		return false;

	return al_play_sample_instance( m_pInstance );
}

bool AllegroSoundSample5::Stop()
{
	if ( !m_pInstance )
		return false;

	return al_stop_sample_instance( m_pInstance );
}

void AllegroSoundSample5::SetVolume( float value )
{
	if ( !m_pInstance )
		return;

	al_set_sample_instance_gain( m_pInstance, value * GetSound().GetMasterVolume() );
	m_volume = value;
}

float AllegroSoundSample5::GetVolume() const
{
	return m_volume;
}

void AllegroSoundSample5::SetPan( float value )
{
	if ( !m_pInstance )
		return;

	al_set_sample_instance_pan( m_pInstance, value );
}

float AllegroSoundSample5::GetPan() const
{
	return m_pInstance ? al_get_sample_instance_pan( m_pInstance ) : 0.0f;
}

void AllegroSoundSample5::SetPitch( float value )
{
	if ( !m_pInstance )
		return;
	al_set_sample_instance_speed( m_pInstance, value );
}

float AllegroSoundSample5::GetPitch() const
{
	return m_pInstance ? al_get_sample_instance_speed( m_pInstance ) : false;
}

bool AllegroSoundSample5::IsPlaying() const
{
	return m_pInstance ? al_get_sample_instance_playing( m_pInstance ) : false;
}

void AllegroSoundSample5::SetLoopMode( ISample::ELoopMode mode )
{
	if ( !m_pInstance )
		return;
	ALLEGRO_PLAYMODE alMode = ALLEGRO_PLAYMODE_ONCE;
	switch( mode )
	{
		case ISample::LOOPMODE_ONCE: alMode = ALLEGRO_PLAYMODE_ONCE; break;
		case ISample::LOOPMODE_LOOP: alMode = ALLEGRO_PLAYMODE_LOOP; break;
	}
	al_set_sample_instance_playmode( m_pInstance, alMode );
}

ISample::ELoopMode AllegroSoundSample5::GetLoopMode() const
{
	if ( !m_pInstance )
		return ISample::LOOPMODE_ONCE;
	ALLEGRO_PLAYMODE alMode = al_get_sample_instance_playmode( m_pInstance );
	switch( alMode )
	{
		case ALLEGRO_PLAYMODE_ONCE: return ISample::LOOPMODE_ONCE;
		case ALLEGRO_PLAYMODE_LOOP: return ISample::LOOPMODE_LOOP;
	}

	return ISample::LOOPMODE_ONCE;
}

// ------------------------------------------------------------

AllegroMusicSample5::~AllegroMusicSample5()
{
	if ( m_pInstance )
		al_destroy_audio_stream( m_pInstance );
}

bool AllegroMusicSample5::Load( const std::string & path )
{
	if ( m_pInstance )
	{
		al_detach_audio_stream( m_pInstance );
		al_destroy_audio_stream( m_pInstance );
	}

	m_pInstance = al_load_audio_stream( path.c_str(), 10, 1024 );
	if ( m_pInstance )
		al_attach_audio_stream_to_mixer( m_pInstance,  ((AllegroSound5&)GetSound()).GetMusicMixer() );

	return ( m_pInstance != 0 );
}

bool AllegroMusicSample5::Play()
{
	if ( !m_pInstance )
		return false;

	al_seek_audio_stream_secs( m_pInstance, 0 );

	return al_set_audio_stream_playing( m_pInstance, true );
}

bool AllegroMusicSample5::Stop()
{
	if ( !m_pInstance )
		return false;

	return al_set_audio_stream_playing( m_pInstance, false );
}

void AllegroMusicSample5::SetVolume( float value )
{
	if ( !m_pInstance )
		return;

	al_set_audio_stream_gain( m_pInstance, value );
	m_volume = value;
}

float AllegroMusicSample5::GetVolume() const
{
	return m_volume;
}

void AllegroMusicSample5::SetPan( float value )
{
	if ( !m_pInstance )
		return;

	al_set_audio_stream_pan( m_pInstance, value );
}

float AllegroMusicSample5::GetPan() const
{
	return m_pInstance ? al_get_audio_stream_pan( m_pInstance ) : 0.0f;
}

void AllegroMusicSample5::SetPitch( float value )
{
	if ( !m_pInstance )
	    return;
	al_set_audio_stream_speed( m_pInstance, value );
}

float AllegroMusicSample5::GetPitch() const
{
	if ( !m_pInstance )
	    return 0;
	return al_get_audio_stream_speed( m_pInstance );
}

bool AllegroMusicSample5::IsPlaying() const
{
	if ( !m_pInstance )
	    return false;
	return al_get_audio_stream_playing( m_pInstance );
}

void AllegroMusicSample5::SetLoopMode( ISample::ELoopMode mode )
{
	if ( !m_pInstance )
	    return;
	    
	ALLEGRO_PLAYMODE alMode = ALLEGRO_PLAYMODE_ONCE;
	switch( mode )
	{
		case ISample::LOOPMODE_ONCE: alMode = ALLEGRO_PLAYMODE_ONCE; break;
		case ISample::LOOPMODE_LOOP: alMode = ALLEGRO_PLAYMODE_LOOP; break;
	}
	al_set_audio_stream_playmode( m_pInstance, alMode );
}

ISample::ELoopMode AllegroMusicSample5::GetLoopMode() const
{
	if ( !m_pInstance )
	    return ISample::LOOPMODE_ONCE;
	ALLEGRO_PLAYMODE alMode = al_get_audio_stream_playmode( m_pInstance );
	switch( alMode )
	{
		case ALLEGRO_PLAYMODE_ONCE: return ISample::LOOPMODE_ONCE;
		case ALLEGRO_PLAYMODE_LOOP: return ISample::LOOPMODE_LOOP;
	}

	return ISample::LOOPMODE_ONCE;
}

AllegroSound5::AllegroSound5()
{
	m_pVoice = 0;
	m_pMusicMixer = 0;
	m_pSoundMixer = 0;
	m_masterVolume = 100;
}

AllegroSound5::~AllegroSound5()
{
	for ( SampleList::iterator iter = m_sampleList.begin();
		iter != m_sampleList.end();
		++iter )
	{
		(*iter)->Stop();
	}

	al_destroy_mixer( m_pMusicMixer );
	al_destroy_mixer( m_pSoundMixer );
	al_destroy_voice( m_pVoice );
	m_sampleList.clear();
}

bool AllegroSound5::Init()
{
	if ( !al_install_audio() )
	{
		GetLog().Log( CommonLog(), LL_WARNING, "Failed to initialize sound (al_install_audio)" );
		return false;
	}

	if ( !al_init_acodec_addon() )
	{
		GetLog().Log( CommonLog(), LL_WARNING, "Failed to initialize sound (al_init_acodec_addon)" );
		return false;
	}

	m_pVoice = al_create_voice( 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2 );		

	if ( !m_pVoice )
	{
		GetLog().Log( CommonLog(), LL_WARNING, "Failed to initialize sound (al_create_voice)" );
		return false;
	}

	m_pMusicMixer = al_create_mixer( 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2 );
	m_pSoundMixer = al_create_mixer( 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2 );
	m_pMasterMixer = al_create_mixer( 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2 );

	if ( !m_pMusicMixer || !m_pSoundMixer || !m_pMasterMixer )
	{
		GetLog().Log( CommonLog(), LL_WARNING, "Failed to initialize sound (al_create_mixer)" );
		return false;
	}

	al_attach_mixer_to_mixer( m_pMusicMixer, m_pMasterMixer );
	al_attach_mixer_to_mixer( m_pSoundMixer, m_pMasterMixer );
	al_attach_mixer_to_voice( m_pMasterMixer, m_pVoice );

	return true;
}

void AllegroSound5::SetMasterVolume( float volume )
{
	if ( !m_pMasterMixer )
	    return;
	m_masterVolume = volume;
	al_set_mixer_gain( m_pMasterMixer, m_masterVolume );
}

float AllegroSound5::GetMasterVolume() const
{
	return m_masterVolume;
}

void AllegroSound5::SetVolumeByType( ISample::EType type, float volume )
{
	if ( !m_pVoice )
	    return;
	switch( type )
	{
		case ISample::TYPE_SOUND:
		{
			m_soundVolume = volume;
			al_set_mixer_gain( m_pSoundMixer, volume );
			break;
		}
		case ISample::TYPE_MUSIC:
		{
			m_musicVolume = volume;
			al_set_mixer_gain( m_pMusicMixer, volume );
			break;
		}
	}
}

float AllegroSound5::GetVolumeByType( ISample::EType type ) const
{
	switch( type )
	{
		case ISample::TYPE_SOUND:
			return m_soundVolume;
		case ISample::TYPE_MUSIC:
			return m_musicVolume;
	}

	return 0.0f;
}

ISamplePtr AllegroSound5::CreateSample( const std::string & path, ISample::EType type )
{
	if ( !m_pVoice )
	    return ISamplePtr();
	    
	switch( type )
	{
		case ISample::TYPE_SOUND:
		{
			AllegroSoundSample5 *pSample = new AllegroSoundSample5();
			pSample->Load( path );
			ISamplePtr s( pSample );
			m_sampleList.push_back( s );
			return s;
		}
		case ISample::TYPE_MUSIC:
		{
			AllegroMusicSample5 *pSample = new AllegroMusicSample5();
			pSample->Load( path );
			ISamplePtr s( pSample );
			m_sampleList.push_back( s );
			return s;
		}
	}
	return ISamplePtr();
}

ISound & GetSound()
{
	static AllegroSound5 sound;
	return sound;
}
