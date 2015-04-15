#pragma once
#include "BaseGameState.h"
#include "IRender.h"

/*
	This class describes what happens after the game is won
*/
class GameStateWinGame : public BaseGameState
{
	  // Ending sequence stages
	enum EStage
	{
		STAGE_INVALID,
		STAGE_SPACE,
		STAGE_EARTH,
		STAGE_TEXT,
		STAGE_LAST,
	};

	  // Current stage
	EStage m_stage;
	  // Stage timer
	float m_timer;

	  // Y-coordinate of sky rectangle
	float m_y;
	  // Y-coordinate of falling object
	float m_yShip;

	  // Fade parameters
	bool m_fadeOut;
	bool m_fadeIn;
	float m_fadeTimer;
	float m_fadeMax;
	Color m_fadeColor;

	  // Image of Dr. Destructo for text stage
	ImageHandle m_question;
	  // Image of stars
	ImageHandle m_space;
	  // Image of island
	ImageHandle m_island;
	  // Image of falling fortress
	ImageHandle m_ship;

	  // Explosion in space stage
	static const int MAX_EXPLOSIONS = 20;
	struct SExplosion
	{
		SExplosion()
			: m_lifeTimer( 0.25f )
		{}

		GameObjectID m_objID;
		float m_lifeTimer;
	};
	SExplosion m_explosions[ MAX_EXPLOSIONS ];
	RequestID m_explosionRequestID[ MAX_EXPLOSIONS ];

	void NextStage();
	bool Skip( const InputEvent & e );

public:
	GameStateWinGame()
		: BaseGameState( GAMESTATE_WIN )
		, m_stage( STAGE_INVALID )
		, m_timer( 0 )
		, m_fadeIn( false )
		, m_fadeOut( false )
		, m_fadeTimer( 0.0f )
	{
		AddKeyboardHandler( this, &GameStateWinGame::Skip, KeyboardEvent::KBD_KEY_DOWN, ALLEGRO_KEY_ESCAPE );	
		AddTouchHandler( this, &GameStateWinGame::Skip,  TouchEvent::TOUCH_BEGIN );
		AddGamepadHandlerFromConfig( "Pad_Bomb", this, &GameStateWinGame::Skip );
	}

	void RenderBefore( IRender & render, const IGameContext & context );
	void RenderAfter( IRender & render, const IGameContext & context );
	void Update( float dt, IGameContext & context );

	bool AllowPhysicsUpdate() const { return false; }
	bool AllowGraphicsUpdate() const { return true; }
	bool AllowGraphicsRender() const { return true; }
	bool AllowAIUpdate() const { return false; }

	void OnPush( IGameContext & context );
	void OnRemove( IGameContext & context ) {}
};
