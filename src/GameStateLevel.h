#pragma once
#include "BaseGameState.h"
#include "SkyController.h"
#include "ReflectionController.h"
#include "GroundController.h"
#include "GameObject.h"
#include "TextManager.h"
#include "TextPrinter.h"
#include "BitArray2D.h"
#include "Common.h"

class MechanicComponent;

  // Helper class for drawing a rectangle with decorations
class UIRect
{
public:
	UIRect()
		: m_x(0)
		, m_y(0)
		, m_w(0)
		, m_h(0)
	{}

	void Init( IRender & render );
	void Render( IRender & render );
	void Render( IRender & render, ImageHandle target );
	void SetRect( int x, int y, int w, int h );

private:
	ImageHandle m_tl;
	ImageHandle m_tr;
	ImageHandle m_bl;
	ImageHandle m_br;
	ImageHandle m_top;
	ImageHandle m_bottom;
	ImageHandle m_left;
	ImageHandle m_right;
	int m_x, m_y, m_w, m_h;
};

/*
	The name is bad, but I can't think of a better one.
	This class displays story-related messages at the
	bottom of the screen.
*/
class PlayByPlay
{
	static const int WIDTH  = SCREEN_W / 2;
	static const int HEIGHT = 128 + 6 + 10 + 10;

	  // Class for drawing text in area
	TextPrinter m_textPrinter;
	UIRect m_rect;

	  // Bitmap on which PBB message is composed before drawing it on screen
	ImageHandle m_image;
	  // Icon of current speaker
	ImageHandle m_currentActor;
	  // Timer used for various things
	float m_timer;

	bool  m_useOpacity;
	float m_opacityTimer;

	  // Index of current PBP message in level's array
	int m_pbpIndex;
	  // True when m_pbpIndex changes
	bool m_pbpUpdated;
	  // If true, m_timer is treated as a pause after all message is show and before it disappears
	bool m_pauseAfter;

public:
	PlayByPlay()
	{
		Reset();
	}

	void Init( IRender & render );

	void UpdatePBPIndex( int index );
	void Update( IGameContext & context, float dt );
	void Render( IRender & render );
	void Reset()
	{
		m_image = ImageHandle();
		m_currentActor = ImageHandle();
		m_textPrinter.SetText( UTFString() );

		m_timer = 0;
		m_pbpIndex = -1;
		m_pbpUpdated = false;
		m_pauseAfter = false;
	}
	bool IsActive();

	void SetDebugText( const UTFString & str )
	{
		m_textPrinter.SetTextParams( 25 );

		const int TEXT_X = 140 + 6 + 10;
		const int TEXT_Y = 10;
		const int TEXT_W = WIDTH - TEXT_X - 10;
		const int TEXT_H = HEIGHT - 20;

		m_textPrinter.SetRegion( TEXT_X, TEXT_Y, TEXT_W, TEXT_H );

		m_textPrinter.SetText( str );
	}

	void DebugRender()
	{
		IRender & render = GetRender();

		if ( !m_image.IsValid() )
			m_image = render.CreateImage( WIDTH, HEIGHT, true, "pbp" );

		render.ClearImage( m_image, Color( 0, 0,0,0 ) );
		m_rect.Render( render, m_image );

		while( m_textPrinter.Update( m_image, render ) );

		render.DrawImage( m_image, SCREEN_W / 2 - WIDTH / 2, SCREEN_H - HEIGHT - 50 );		
	}
};

  // Helper class for rendering UI where lives, score etc. are displayed
class CombatUI
{
public:
	void Init( IRender & render );
	void Render( const IGameContext & context, GameObjectID playerObjectID, IRender & render );

private:
	  // Combat UI consists of 4 elements and a bomb image
	ImageHandle m_e1, m_e2, m_e3, m_e4, m_bomb;
};

struct SAnimFrame;

  // Helper class for displaying a message about lost life and the type of enemy the player have crashed into
class LifeLossEffect
{
	enum EState
	{
		STATE_INACTIVE, // Invisible
		STATE_CRACK,    // Draw windshield cracks
		STATE_TEXT,     // Text appears
		STATE_WAIT,     // Wait until user presses fire
		STATE_FADE_OUT, // Fade out
	};

public:
	LifeLossEffect()
		: m_pauseTimer( 0.0f )
		, m_state( STATE_INACTIVE )
	{}

	  // Returns true if this effect is currently shown
	bool IsActive() const;
	  // Called when user presses fire (on keyboard, gamepad, or touch)
	void OnFire();
	  // Restarts effect with specified enemy image
	void Restart( ImageHandle enemyImage,  const SAnimFrame *pFrame );
	  // Makes effect inactive
	void Stop();

	void Update( float dt );

private:
	void RenderCracks( IRender & render );
	void RenderText( IRender & render );

public:
	void Render( IRender & render );

private:
	EState m_state;
	  // Current state of a crack
	struct SLine
	{
		int m_x, m_y;   // Last crack position (starts from center)
		float m_angle;  // Angle at which crack last expanded
		int m_lifetime; // Remaining lifetime (in number of expansions) of the crack
	};	
	  // List of currently expanding cracks
	std::vector<SLine> m_lines;
	  // Image for rendering effect
	ImageHandle m_image;
	  // Image for rendering text over effect
	ImageHandle m_textImage;
	  // Current state timer
	float m_pauseTimer;
	  // Initial value of state timer
	float m_pauseTimerInit;
	  // Separate blink timer for prompt
	float m_blinkTimer;
	  // Text & enemy image background
	UIRect m_textRect;
};

/*
	This state describes main game screen, where you play the game.

	Screen logic:
	* Fade in from black
	* Draw objects, process controls and messages
	* Show PBP messages when necessary
	* If player loses all lives or is out of time - go to game over screen
	* If enough of target is destroyed - go to win screen
*/
class GameStateLevel : public BaseGameState
{
	  // Class responsible for drawing sky
	SkyController m_skyController;
	  // Class responsible for drawing reflection in water
	ReflectionController m_refController;
	  // Class responsible for drawing ground in levels where there is no water
	GroundController m_groundController;
	  // Graphic component IDs for render procedures, the only objects other than GameObjects to use Component system
	GraphicCompID m_beforeComp;   // Upper part of the reflection (the ship will be drawn over it)
	GraphicCompID m_afterComp;    // Lower part of the reflection (the ship will sinky beyond it)
	GraphicCompID m_groundCompFg; // Foreground for levels where we have ground (castles, ports etc.). Cranes in port level, for example. Planes will be drawn BEYOND it!
	GraphicCompID m_groundCompMg; // "Middleground". Planes will be drawn over it, the ship will sink beyond it
	GraphicCompID m_groundCompBg; // Background. Ship will sink over it (used very rarely, see level 10), i.e. ground for castle
	  // Class for displaying and tracking story messages
	PlayByPlay m_pbp;

	  // Mask of damage done to target. It will be lost if DirectX device is lost, and is need to be recreated from m_damageCahce
	ImageHandle m_damageImage;
	  // Bit map of damage done to target. Used for determining hits below waterline
	BitArray2D  m_damageMap;

	  // User input flags
	bool m_exitGame;
	bool m_pauseGame;
	  // Flags used to pass control to mechanics component
	int m_controls;
	  // Special case for touch (might be obsolete)
	int m_touchControls;
	float m_touchControlsValue;
	int m_touchControlsNum;
	  // Special case for gamepad
	int m_padControls;
	  // If true, weapon cooldown must be reset, because user released fire button
	bool m_resetWeaponCooldown;

	bool m_gamePaused;

	struct STouch
	{
		STouch()
			: id( -1 )
			, time( 0 )
		{}

		int id;
		int time;
		int sx, sy;
	};

	STouch m_touch1;
	STouch m_touch2;

	  // Specially remembered ID of player's object (plane)
	GameObjectID m_playerObjectID;
	  // ID of request to create player's object
	RequestID m_playerObjectRequestID;

	  // Used to notice when options version change (see g_options_version)
	int m_optionsVersion;

	  // Gameplay flags
	bool m_aimAidEnabled;

	  // Timer before start of tutorial
	float m_tutorialTimer;
	  // True if tutorial is enabled (on first start, of from options)
	bool  m_tutorialEnabled;

	  // The reason game is over
	enum EGameOverType
	{
		GAMEOVER_TYPE_NONE	= 0,
		GAMEOVER_TYPE_LIVES = 1,
		GAMEOVER_TYPE_TIME	= 2,
	};

	  // Timer for pause before going to game over screen
	float m_gameOverTimer;
	EGameOverType m_gameOverType;

	  // If true, draw fade effect
	bool m_fade;
	float m_fadeTimer;

	  // How long this level played in seconds
	float m_levelTimer;

	CombatUI m_ui;

	int m_debugBulletType;

	struct SDamageCache
	{
		int x, y;
	};

	  // Stored damage, from which damage image is recreated if device was lost
	std::vector<SDamageCache> m_damageCache;

	LifeLossEffect m_lifeLossEffect;

	bool ExitGame( const InputEvent & e );
	bool PauseGame( const InputEvent & e );	

	bool TurnUp( const InputEvent & e );
	bool TurnDown( const InputEvent & e );
	bool SpeedUp( const InputEvent & e );
	bool FireGun( const InputEvent & e );
	bool FireGunRelease( const InputEvent & e );
	bool FireBomb( const InputEvent & e );

	bool TouchBegin( const InputEvent & e );
	bool TouchMove( const InputEvent & e );
	bool TouchEnd( const InputEvent & e );
	bool TouchCancel( const InputEvent & e );

	bool PadTurnUp( const InputEvent & e );
	bool PadTurnDown( const InputEvent & e );
	bool PadAccel( const InputEvent & e );
	bool PadFireGun( const InputEvent & e );
	bool PadFireBomb( const InputEvent & e );

	bool DebugSetBulletType0( const InputEvent & e ) { m_debugBulletType = 0; return true; }
	bool DebugSetBulletType1( const InputEvent & e ) { m_debugBulletType = 2; return true; }
	bool DebugSetBulletType2( const InputEvent & e ) { m_debugBulletType = 3; return true; }
	bool DebugSetBulletType3( const InputEvent & e ) { m_debugBulletType = 4; return true; }
	bool DebugSetBulletType4( const InputEvent & e ) { m_debugBulletType = 5; return true; }
	bool DebugSetBulletType5( const InputEvent & e ) { m_debugBulletType = 6; return true; }
	bool DebugSetBulletType6( const InputEvent & e ) { m_debugBulletType = 7; return true; }
	bool DebugSetBulletType7( const InputEvent & e ) { m_debugBulletType = 8; return true; }
	bool DebugSetBulletType8( const InputEvent & e ) { m_debugBulletType = 9; return true; }
	bool DebugSetBulletType9( const InputEvent & e ) { m_debugBulletType = 10; return true; }

	  // Applies player and AI input to physic (movement)
	void ControlsToPhysics( GameObject *pObject,IGameContext & context, float dt );
	  // Applies player and AI input to mechanic (weapons etc.)
	void ControlsToMechanic( GameObject *pObject,IGameContext & context, float dt );
	  // Applies changes in Physic components to Graphic components (copies position etc.)
	void PhysicsToGraphics( GameObject *pObject,IGameContext & context, float dt );
	  // Applies changes in Physic components to Sound components (copies angle to calculate engine pitch etc.)
	void PhysicsToSound( GameObject *pObject,IGameContext & context, float dt );
	  // Copies necessary data from Physic components to AI
	void PhysicsToAI( GameObject *pObject,IGameContext & context, float dt );
	  // Provides sound event from animation changes
	void GraphicsToSound( GameObject *pObject,IGameContext & context, float dt );

	  // Processes all collisions supplied by PhysicProcessor
	void ProcessCollisions( IGameContext & context );
	  // Processes single collision between two objects
	void ProcessSingleCollision( const MechanicComponent *pMe, const MechanicComponent *pOther, IGameContext & context );
	  
	  // Fires a weapon (for player or AI)
	void FireWeapon( int weaponIndex, const PhysicComponent *pPhComp, const ControlComponent *pCnComp, const GraphicComponent *pGrComp, MechanicComponent *pMechComp, SoundComponent *pSnComp, IGameContext & context, const std::string & soundEvent );

public:
	GameStateLevel();

	void OnPush( IGameContext & context );
	void OnRemove( IGameContext & context );
	void OnDisplayFound( IGameContext & context );

	void RenderBefore( IRender & render, const IGameContext & context );
	void RenderAfter( IRender & render, const IGameContext & context );
	void Update( float dt, IGameContext & context );

	void CheckWinConditions( IGameContext & context );
};
