#pragma once
#include "BaseGameState.h"
#include "IRender.h"

/*
	This class describes what happens after level is won,
	but before the briefing for the next level (or end-game
	animation) is shown.
*/
class GameStateWin : public BaseGameState
{
	  // Describes a single Achievement to display
	struct SAchDisp
	{
		ImageHandle m_image;

		int m_x;
		int m_y;
		int m_alpha;
	};

	  // New achievements
	std::vector< SAchDisp > m_achievements;
	  // Currently appearing achievement
	int m_currentAch;

	enum EStage
	{
		STAGE_CLEAR,
		STAGE_RESULTS,
		STAGE_ACHIEVEMENTS,
		STAGE_FADE,
	};

	  // Y-speed of sinking target object
	float m_speedY;
	  // Timer for pause before starting the first stage
	float m_waitTimer;

	  // Fade timer
	float m_fadeTimer;
	  // This timer blocks Escape key handling to prevent player skipping achievements from skipping all thr screen
	float m_skipTimer;

	EStage m_stage;

	  // Image for drawing statistics
	ImageHandle m_results;
	  // Current Y-coordinate of statistics image
	float m_resultsY;
	  // Current speed of statistics image
	float m_resultsSpeed;

	  // If true, we need to create statistics image
	bool m_generateResults;

	void CheckAchievements( PlayerProfile & prof, const IGameContext & context );
public:
	GameStateWin()
		: BaseGameState( GAMESTATE_WIN )
	{
		m_stage = STAGE_CLEAR;
		m_speedY = 0;
		m_waitTimer = 0;
	}

	void RenderBefore( IRender & render, const IGameContext & context );
	void RenderAfter( IRender & render, const IGameContext & context );
	void Update( float dt, IGameContext & context );

	bool AllowPhysicsUpdate() const { return false; }
	bool AllowGraphicsUpdate() const { return false; }
	bool AllowGraphicsRender() const { return true; }
	bool AllowAIUpdate() const { return false; }

	void OnPush( IGameContext & context );
	void OnRemove(  IGameContext & context ) {}

	bool Skip( const InputEvent & e);
};
