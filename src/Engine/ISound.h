#pragma once

// See AllegroSound5 for some comments

class ISample
{
public:
	enum EType
	{
		TYPE_MUSIC = 0,
		TYPE_SOUND,

		TYPES_COUNT,
	};

	enum ELoopMode
	{
		LOOPMODE_ONCE,
		LOOPMODE_LOOP,
	};

	virtual ~ISample(){}
	ISample() {}
	ISample( const ISample & other ){}

	virtual bool Load( const std::string & path ) = 0;

	virtual bool Play() = 0;
	virtual bool Stop() = 0;

	virtual void SetVolume( float value ) = 0;
	virtual float GetVolume() const = 0;

	virtual void SetPan( float value ) = 0;
	virtual float GetPan() const = 0;

	virtual void SetPitch( float value ) = 0;
	virtual float GetPitch() const = 0;

	virtual bool IsPlaying() const = 0;

	virtual void SetLoopMode( ELoopMode mode ) = 0;
	virtual ELoopMode GetLoopMode() const = 0;
};

#ifndef HAS_STD_SHARED_PTR
typedef boost::shared_ptr<ISample> ISamplePtr;
#else
typedef std::shared_ptr<ISample> ISamplePtr;
#endif

class ISound
{
public:
	virtual ~ISound(){}

	virtual bool Init() = 0;
	virtual void SetMasterVolume( float volume ) = 0;
	virtual float GetMasterVolume() const = 0;

	virtual void SetVolumeByType( ISample::EType type, float volume ) = 0;
	virtual float GetVolumeByType( ISample::EType type ) const = 0;

private:
	template <ISample::EType> friend class SampleInstance;

	virtual ISamplePtr CreateSample( const std::string & path, ISample::EType type ) = 0;
};

ISound & GetSound();

template<ISample::EType Type>
class SampleInstance : public ISample
{
	ISamplePtr m_sample;

protected:
	void Create( const std::string & path )
	{
		if ( !m_sample )
			m_sample = GetSound().CreateSample( path, Type );
		else
			m_sample->Load( path );
	}

public:
	SampleInstance()
	{}

	SampleInstance( const SampleInstance & other )
	{
		if ( this != &other )
			m_sample.reset( other.m_sample );
	}

	~SampleInstance()
	{
		Stop();
	}

	virtual bool Load( const std::string & path ) { Create( path ); return true; }

	bool Play() { return m_sample ? m_sample->Play() : false; }
	bool Stop() { return m_sample ? m_sample->Stop() : false; }

	void SetVolume( float value ) { if ( m_sample ) m_sample->SetVolume( value ); };
	float GetVolume() const { return m_sample ? m_sample->GetVolume() : 0; }

	void SetPan( float value ) { if ( m_sample ) m_sample->SetPan( value ); };
	float GetPan() const { return m_sample ? m_sample->GetPan() : 0; }

	void SetPitch( float value ) { if ( m_sample ) m_sample->SetPitch( value ); };
	float GetPitch() const { return m_sample ? m_sample->GetPitch() : 0; }

	bool IsPlaying() const { return m_sample ? m_sample->IsPlaying() : false; }
	
	void SetLoopMode( ELoopMode mode ) { if ( m_sample ) m_sample->SetLoopMode( mode ); }
	ELoopMode GetLoopMode() const { return m_sample ? m_sample->GetLoopMode() : LOOPMODE_ONCE; }
};

typedef SampleInstance<ISample::TYPE_SOUND> SoundInstance;
typedef SampleInstance<ISample::TYPE_MUSIC> MusicInstance;
