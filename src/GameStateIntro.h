#pragma once
#include "BaseGameState.h"
#include "BitmapManager.h"

/*
	This state describes intro screen.

	Screen logic:
	* Show static intro picture
	* Show flashing "Press Enter" text
	* Wait until user presses Enter
	* Fade image out
*/
class GameStateIntro : public BaseGameState
{
	  // Fade timer
	float m_timer;
	  // Text flash timer
	float m_flash;
      // Static intro image
	ImageHandle m_image;
	  // True if we're fading now
	bool m_fade;

	bool GoToMenu( const InputEvent & e);
	bool GoToMenuWithStart( const InputEvent & e);	

public:
	GameStateIntro()
		: BaseGameState( GAMESTATE_INTRO )
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
	void OnRemove( IGameContext & context ) {}
};
