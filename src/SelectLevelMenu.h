#pragma once

#include "BaseMenu.h"
#include "MenuHelpers.h"
#include "Common.h"

/*
	Menu for level selection in casual mode
*/

class IGameContext;

class ISelectLevelCallback
{
public:
	virtual void BackFromSelectLevel() = 0;
	virtual void StartLevel( int levelIndex ) = 0;
};

class ILevelMenuEntryCallback
{
public:
	virtual void OnLevelSelected( const char *levelObject ) = 0;
};

class LevelMenuEntry : public ValueListMenuEntry<std::string>
{
	ILevelMenuEntryCallback *m_pCallback;
public:
	LevelMenuEntry( BaseMenu *menu, int order, ILevelMenuEntryCallback *pCallback ) 
		: ValueListMenuEntry<std::string>( menu, order, "", 700, 64 )
		, m_pCallback( pCallback )
	{}

	void Render( IRender *pRender, int x, int y );

	virtual void OnValueChanged( int index )
	{
		m_pCallback->OnLevelSelected( m_values[ index ].m_data.c_str() );
	}
};

  // This menu allows player to select level to start from (for Casual mode)
class MenuSelectLevel : public GameMenu, public ButtonMenuEntry::ICallback, public ILevelMenuEntryCallback
{
	static const int EID_BACK			= 1;
	static const int EID_START			= 2;

	enum EState
	{
		STATE_FADE_IN,
		STATE_FADE_OUT,
		STATE_STAY,
	} m_state;

	int m_timer;

	static const int FADE_TIME = 250;

	ISelectLevelCallback *m_pCallback;

	LevelMenuEntry		     m_level;
	FooterAcceptButtonEntry  m_start;
	FooterDeclineButtonEntry m_back;

	friend class LevelsEnumerator;

	  // Name of level's target to load and show
	std::string m_wantLevelObject;

	ImageHandle m_levelObjectImage;

public:
	MenuSelectLevel( int x, int y, ISelectLevelCallback *pCallback )
		: GameMenu( "Select starting level", SCREEN_W/2 - 350, SCREEN_H/2-200, 700, 400 )
		, m_pCallback( pCallback )
		, m_level( this, 0, this )
		, m_back( this, 255, "Back", EID_BACK, this )
		, m_start( this, 255, "Start", EID_START, this )
		, m_state( STATE_STAY )
		, m_timer( 0 )
	{
	}

	void AddLevel( const char *levelName, const char *levelObject );

	void operator()( int id );
	void Back() { this->operator ()( EID_BACK ); }

	void ReInit( IGameContext & context );

	void Render( IRender *pRender );

	void UpdateLevel( IGameContext & context, float dt );

	void OnLevelSelected( const char *levelObject );
};
