#pragma once
#include "InputHandler.h"
#include "ComponentID.h"
#include "GameContext.h"

/*
	This is base game state interface.
*/
class BaseGameState : protected InputHandler
{
	EGameState m_type;
	
public:
	BaseGameState( EGameState type ) : m_type( type ) {}
	virtual ~BaseGameState(){};
	  // This lets state process user input. Note that base behaviour is to pass
	  // all input to standard InputHandler, where you can register controls.
	virtual void ProcessInput( IInput & input ) { InputHandler::ProcessInput( input ); }
	  // This is called before game objects are rendered
	virtual void RenderBefore( IRender & render, const IGameContext & context ) = 0;
	  // This is called after game objects are rendered
	virtual void RenderAfter( IRender & render, const IGameContext & context ) = 0;
	  // This is the main update function of state. Note that Update will only be called
	  // for the top-most state on the stack (unlike render functions)
	virtual void Update( float dt, IGameContext & context ) =  0;
	  // Return true is state allows updating physic state of objects (call to PhysicProcessor)
	virtual bool AllowPhysicsUpdate() const { return true; }
	  // Return true is state allows updating graphic state of objects (call to GraphicProcessor)
	virtual bool AllowGraphicsUpdate() const { return true; }
	  // Return true is state allows drawing objects (call to GraphicProcessor)
	virtual bool AllowGraphicsRender() const { return true; }
	  // Return true is state allows updating AI state of objects (call to AIProcessor)
	virtual bool AllowAIUpdate() const { return true; }
	  // Called when state is pushed on stack
	virtual void OnPush( IGameContext & context ) = 0;
	  // Called when state is removed from stack
	virtual void OnRemove( IGameContext & context ) = 0;
	  // Called when device is restored (DirectX)
	virtual void OnDisplayFound( IGameContext & context ) {}

	EGameState GetType() const { return m_type; }
};
