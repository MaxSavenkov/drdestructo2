#pragma once
#include "BaseGameState.h"
#include "MenuHelpers.h"

  // Subclass of TextInputMenuEntry for entering player's name into High Score table
class ScoreEntry : public TextInputMenuEntry
{
	static const float BLINK_TIME;
	float m_timer;
public:
	ScoreEntry( ICallback *pCallback, int maxLength )
		: TextInputMenuEntry( 0, 0, "", "", maxLength, Color(255,255,255), pCallback )
		, m_timer( 0 )
	{}

	void Render( IRender *pRender, int x, int y )
	{		
		pRender->DrawText( x, y, m_timer < BLINK_TIME/2 ? m_color : Color(0,0,0), 30, "%s_", m_value.c_str() );
		pRender->DrawText( x, y, m_color, 30, "%s", m_value.c_str() );
	}

	void Update( float dt )
	{
		m_timer += dt;
		if ( m_timer > BLINK_TIME )
			m_timer -= BLINK_TIME;
	}

	void Activate()
	{
	}

	int GetWidth()
	{
		return 0;
	}
};

  // Description of a single High Score entry
struct SScoreEntry
{
	enum EFlag
	{
		FLAG_FINISHED_GAME = 1 << 0,
	};

	SScoreEntry()
		: score( 0 )
		, name( "Nobody" )
		, date( "00.00.0000 00:00" )
		, flags( 0 )
	{}

	int score;
	std::string name;
	std::string date;
	  // Special flags (not used now)
	int flags;

	bool operator > ( const SScoreEntry & other ) const
	{
		return score > other.score;
	}
};

/*
	This class describes High Score screen.
*/
class GameStateScoreTable : public BaseGameState, public TextInputMenuEntry::ICallback
{
	  // Image used for drawing transparent text over shifting colors
	ImageHandle m_mask;
	  // Moving background image
	ImageHandle m_bg;
	  // Current background position
	int m_bgX;
	  // If true, text mask needs to be regenerated
	bool m_regenerateText;
	  // If true, screen could be closed
	bool m_finish;
	  // If true, screen SHOULD be closed
	bool m_exit;

	  // Position of input control
	int m_inputX;
	int m_inputY;

	  // Input control
	static const int MAX_NAME_LEN = 10;
	ScoreEntry m_input;

	  // Stored scroe table entries
	static const int MAX_SCORES = 20;
	SScoreEntry m_scoreEntry[ MAX_SCORES ];

	bool LoadScores();
	bool SaveScores();

	  // Index of new high score (or -1)
	int m_entryPos;

	bool Exit( const InputEvent & e );

	void DrawScoreEntry( const SScoreEntry & e, int x, int y, int pos, const Color & color );

public:
	GameStateScoreTable()
		: BaseGameState( GAMESTATE_SCORES )
		, m_input( this, MAX_NAME_LEN )
		, m_inputX( 0 )
		, m_inputY( 0 )
	{
		m_input.SetCanCancel( false );
		AddKeyboardHandler( this, &GameStateScoreTable::Exit,   KeyboardEvent::KBD_KEY_DOWN, ALLEGRO_KEY_ESCAPE );
		AddTouchHandler( this, &GameStateScoreTable::Exit,  TouchEvent::TOUCH_BEGIN );
		AddGamepadHandlerForControl( JoystickMapping::CONTROL_B, this, &GameStateScoreTable::Exit );
	}

	void ProcessInput( IInput & input );

	void RenderBefore( IRender & render, const IGameContext & context );
	void RenderAfter( IRender & render, const IGameContext & context );
	void Update( float dt, IGameContext & context );

	bool AllowPhysicsUpdate() const { return false; }
	bool AllowGraphicsUpdate() const { return false; }
	bool AllowGraphicsRender() const { return false; }
	bool AllowAIUpdate() const { return false; }

	void OnPush( IGameContext & context );
	void OnRemove( IGameContext & context ) {}

	void AcceptEdit( const std::string & value );
	void CancelEdit();
};
