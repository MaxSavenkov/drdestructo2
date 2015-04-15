#pragma once

#include "BaseGameState.h"
#include "IRender.h"
#include "IUTFString.h"

class JoystickMapping;

/*
	This state describes behaviour of tutorial
*/

class GameStateTutorial : public BaseGameState
{
	  // A tutorial message consists of a pulsing rectangle and some text
	struct SMessage
	{
		void Copy( const SMessage & other )
		{
			m_rects = other.m_rects;
			m_textX = other.m_textX;
			m_textY = other.m_textY;
			m_textW = other.m_textW;
			m_textH = other.m_textH;
		}

		struct SRect
		{
			SRect( int _x1, int _y1, int _x2, int _y2, Color c )
			{
				x1 = _x1;
				y1 = _y1;
				x2 = _x2;
				y2 = _y2;
				color = c;
			}
			int x1,y1,x2,y2;
			Color color;
		};

		std::vector<SRect> m_rects;
		UTFString m_text;
		int m_textX, m_textY;
		int m_textW, m_textH;
	};

	struct ControlPos
	{
		int x, y;
	};

	  // Helpe struct describing a control scheme element
	struct Control
	{
		  // Control image (button, key, stick etc.)
		ImageHandle m_image;
		  // Displayed control name (overlayed on image)
		std::string m_controlName;
		  // Displayed control label, like "Fire" or "Rotate clockwise"
		std::string m_controlLabel;
		  // Size & position
		int         m_height;
		int			m_width;

		ControlPos  m_pos;
	};

	enum EControl
	{
		CONTROL_CLOCK,
		CONTROL_COUNTER,
		CONTROL_ACCEL,
		CONTROL_FIRE,
		CONTROL_BOMB,

		CONTROLS_COUNT
	};

	Control m_kbdControls[CONTROLS_COUNT];
	Control m_padControls[CONTROLS_COUNT];

	  // Images for special keys, for which no text could be shown
	ImageHandle m_keyArrowLeft;
	ImageHandle m_keyArrowRight;
	ImageHandle m_keyArrowUp;
	ImageHandle m_keyArrowDown;
	  // Arrow indicating rotation
	ImageHandle m_rotateArrow;

	enum EStage
	{
		STAGE_TITLE,
		STAGE_PLAYER,
		STAGE_CONTROLS_KDB,
		STAGE_CONTROLS_PAD,
		STAGE_CONTROLS_TOUCH,
		STAGE_TARGET,
		STAGE_ENEMIES,
		STAGE_HUD,
		STAGE_LAST,
		STAGE_FADE_IN,

		STAGES_COUNT,
	};

	SMessage m_messages[ STAGES_COUNT ];

	EStage m_stage;
	float m_fadeTimer;
	float m_blinkTimer;

	bool Skip( const InputEvent & e);
	bool SkipPhase( const InputEvent & e);
	bool SkipPhaseAlt( const InputEvent & e);

	void NextPhase();

public:
	GameStateTutorial()
		: BaseGameState( GAMESTATE_TUTORIAL )
		, m_fadeTimer( 0 )
		, m_stage( STAGE_TITLE )
	{
	
	}

	void RenderBefore( IRender & render, const IGameContext & context );
	void RenderAfter( IRender & render, const IGameContext & context );
	void Update( float dt, IGameContext & context );

	bool AllowPhysicsUpdate() const { return false; }
	bool AllowGraphicsUpdate() const { return false; }
	bool AllowGraphicsRender() const { return true; }
	bool AllowAIUpdate() const { return false; }

	void OnPush( IGameContext & context );
	void OnRemove( IGameContext & context );

private:
	void PrepareControlsInfo();
	void SetupControlPositions( Control (&controls)[CONTROLS_COUNT] );
	void ConfigureKdbControl( EControl control, const char *keyName );
	void ConfigurePadControl( const JoystickMapping & mapping, EControl control, const char *baseName );
};


