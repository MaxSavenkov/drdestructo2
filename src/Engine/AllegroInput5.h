#pragma once
#include "IInput.h"

// This struct contains state of a gamepad axis. The goal is to report any
// axis movement only once, like it's a key press, so once an event is sent,
// m_unlocked is set to false until axis returns to a neutral position or
// changes direction
struct GamepadAxis
{
	GamepadAxis()
		: m_unlocked( true )
	{}

	bool m_unlocked;
};

// Allegro groups gamepad axes into a stick, so we do too
struct GamepadStick
{
	std::vector<GamepadAxis> m_axis;
};

/*
	Implementation of IInput interface for Allegro5
*/
class AllegroInput5 : public IInput
{
	  // Our events queue is actually circular buffer, and this is its size
	static const int MAX_EVENTS = 256;

	  // Initialized input methods (but see comment in cpp)
	bool m_hasKeyboard;
	bool m_hasMouse;
	bool m_hasGamepad;
	bool m_hasTouch;

	  // Saved input events
	std::vector<InputEvent> m_events;
	  // Saved keyboard states
	std::bitset<ALLEGRO_KEY_MAX> m_keyStates;
	  // Keyboard repeat timer
	int m_repeatTimer;
	  // For catching double-click events
	int m_doubleClickTimer;
	  // Gamepad sticks states for transforming analog input into digital-like
	std::vector<GamepadStick> m_gamepadSticks;

	ALLEGRO_EVENT_QUEUE *m_queue;

	  // Event index in queue, used for reading
	size_t m_eventIndex;

	void AddEvent( InputEvent ev );
public:
	AllegroInput5()
		: m_eventIndex( 0 )
		, m_queue( 0 )
		, m_doubleClickTimer( 0 )
	{}

	~AllegroInput5();

	  // Initializes Input with support for given devices
	bool Init( bool keyboard, bool mouse, bool gamepad, bool touch );
	
	  // Helper/debug function, will pause the game until a key is pressed
	void WaitAnyKey();
	
	  // Restarts reading event queue
	void BeginRead()
	{
		m_eventIndex = 0;
	}
	  // Get next event from queue. If there are no more events, return InputEvent with type EVENT_INVALID
	InputEvent GetEvent();
	  // Moves index to the next event without removing current one from queue
	void NextEvent();
	  // Removes current event from queue and move index to the next one
	void PopEvent();
	
	  // Polls states, processes events
	void Update( int dt );
	
	  // Removes all pending events from queue
	void Clear()
	{
		m_eventIndex = 0;
		m_events.clear();
	}

	  // Should return true if we have am usable keyboard (only relevant for mobile devies, for now returns true always)
	bool HasHardwareKeyboardInstalled();
	  // Returns true if gamepad was initialized
	bool HasGamepadInstalled();
	  // Returns true if touch controls were initialized
	bool HasTouchInstalled();
};

  // Singleton for accessing Input
IInput & GetInput()
{
	static AllegroInput5 input;
	return input;
}
