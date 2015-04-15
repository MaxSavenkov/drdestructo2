#pragma once
#include "BaseGameState.h"
#include "BitmapManager.h"
#include "TextManager.h"
#include "TextPrinter.h"
#include "UIHelpers.h"

/*
	This state represents briefing screen between levels
*/
class GameStateNextLevel : public BaseGameState
{
	TextPrinter m_textPrinter;

	  // Various timers
	float m_timer;
	  // Text flash timer
	float m_flash;

	  // Currently drawing piece of briefing
	int m_briefingIndex;

	  // Images for character portraits and associated texts
	struct Actor
	{
		ImageHandle m_image;
		int m_y;
	};

	std::vector<Actor> m_actors;

	enum EState
	{
		STATE_INIT,
		STATE_ACTOR_INIT,
		STATE_ACTOR_APPEAR,
		STATE_DRAW_TEXT,
		STATE_PAUSE,
		STATE_FINISH,
		STATE_FADE,
	};

	EState m_state;

	  // Icon of next speaker
	ImageHandle m_nextActor;
	  // If true, m_briefingIndex needs to be incremented
	bool m_next;
	  // If true, player wishes to skip a piece of briefing
	bool m_skipPhrase;
	  // If true, player wishes to skip all briefing
	bool m_skipAll;

	  // Y of current piece of briefing
	int m_areaY;
	  // Current text coordinates
	int m_textY;
	int m_textX;

	  // UI Images
	//ImageHandle m_bg;
	ImageHandle m_slot;
	ImageHandle m_shaft;
	WindowBorder m_border;

	bool SkipPhrase( const InputEvent & e);
	bool SkipPhraseAlt( const InputEvent & e);
	bool SkipAll( const InputEvent & e);

public:
	GameStateNextLevel()
		: BaseGameState( GAMESTATE_NEXT_LEVEL )
	{
	}

	void RenderBefore( IRender & render, const IGameContext & context );
	void RenderAfter( IRender & render, const IGameContext & context );
	void Update( float dt, IGameContext & context );

	bool AllowPhysicsUpdate() const { return false; }
	bool AllowGraphicsUpdate() const { return false; }
	bool AllowGraphicsRender() const { return false; }
	bool AllowAIUpdate() const { return false; }

	void OnPush( IGameContext & context );
	void OnRemove( IGameContext & context );
};
