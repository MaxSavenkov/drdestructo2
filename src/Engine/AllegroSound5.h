#pragma once
#include "Engine/ISound.h"
#include "Engine/ResourceManager.h"

/*
	Class for storing raw sample data. It should only ever be deleted if not
	needed by anyone - if no instance of this sample is played.
*/
struct SSample
{
	SSample()
		: m_pSample( 0 )
	{}

	SSample( const std::string & key, ALLEGRO_SAMPLE *pSample )
		: m_key( key )
		, m_pSample( pSample )
	{}

	std::string m_key;
	ALLEGRO_SAMPLE *m_pSample;

	operator ALLEGRO_SAMPLE*() { return m_pSample; }
	operator const ALLEGRO_SAMPLE*() const { return m_pSample; }
};

/*
	Stores data about streamed sample (music).
*/
struct SSampleStream
{
	SSampleStream()
		: m_pStream( 0 )
	{}

	SSampleStream( const std::string & key, ALLEGRO_AUDIO_STREAM *pStream )
		: m_key( key )
		, m_pStream( pStream )
	{}

	std::string m_key;
	ALLEGRO_AUDIO_STREAM *m_pStream;

	operator ALLEGRO_AUDIO_STREAM*() { return m_pStream; }
	operator const ALLEGRO_AUDIO_STREAM*() const { return m_pStream; }
};

#ifndef HAS_STD_SHARED_PTR
typedef boost::shared_ptr<SSample> SamplePtr;
typedef boost::shared_ptr<SSampleStream> SampleStreamPtr;
#else
typedef std::shared_ptr<SSample> SamplePtr;
typedef std::shared_ptr<SSampleStream> SampleStreamPtr;
#endif
/*
	Implementation for sound effects samples.
	Uses ALLEGRO_SAMPLE_INSTANCE for playback
*/
class AllegroSoundSample5 : public ISample
{
	ALLEGRO_SAMPLE_INSTANCE *m_pInstance;
	float m_volume;

public:
	AllegroSoundSample5()
		: m_pInstance( 0 )
		, m_volume( 1.0f )
	{}

	~AllegroSoundSample5();

	bool Load( const std::string & path );

	bool Play();
	bool Stop();

	void SetVolume( float value );
	float GetVolume() const;

	void SetPan( float value );
	float GetPan() const;

	void SetPitch( float value );
	float GetPitch() const;

	bool IsPlaying() const;

	void SetLoopMode( ELoopMode mode );
	ELoopMode GetLoopMode() const;

	ALLEGRO_SAMPLE_INSTANCE *GetInstance() { return m_pInstance; }
};

/*
	Implementation for sound effects samples.
	Uses ALLEGRO_AUDIO_STREAM for playback,
	because tracker music only works in this mode.
*/
class AllegroMusicSample5 : public ISample
{
	ALLEGRO_AUDIO_STREAM *m_pInstance;
	float m_volume;

public:
	AllegroMusicSample5()
		: m_pInstance( 0 )
		, m_volume( 1.0f )
	{}

	~AllegroMusicSample5();

	bool Load( const std::string & path );

	bool Play();
	bool Stop();

	void SetVolume( float value );
	float GetVolume() const;

	void SetPan( float value );
	float GetPan() const;

	void SetPitch( float value );
	float GetPitch() const;

	bool IsPlaying() const;

	void SetLoopMode( ELoopMode mode );
	ELoopMode GetLoopMode() const;

	ALLEGRO_AUDIO_STREAM *GetInstance() { return m_pInstance; }
};

class AllegroSound5 : public ISound
{
	float m_masterVolume;
	float m_musicVolume;
	float m_soundVolume;

	  // I only use the single voice into which everything else is mixed
	ALLEGRO_VOICE *m_pVoice;
	  // Master mixer for setting Master Sound Volume
	ALLEGRO_MIXER *m_pMasterMixer;
	ALLEGRO_MIXER *m_pMusicMixer;
	ALLEGRO_MIXER *m_pSoundMixer;

	  // Currently active samples (both Music and Sounds)
	typedef std::vector<ISamplePtr> SampleList;
	SampleList m_sampleList;

public:
	AllegroSound5();
	~AllegroSound5();

	bool Init();

	ALLEGRO_MIXER *GetSoundMixer() { return m_pSoundMixer; }
	ALLEGRO_MIXER *GetMusicMixer() { return m_pMusicMixer; }

	void SetMasterVolume( float volume );
	float GetMasterVolume() const;

	void SetVolumeByType( ISample::EType type, float volume );
	float GetVolumeByType( ISample::EType type ) const;


private:
	ISamplePtr CreateSample( const std::string & path, ISample::EType type );
};
