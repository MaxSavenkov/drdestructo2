#pragma once

class SoundComponent;

  // Processes sound events
class SoundProcessor
{
public:
	void Update( float dt, SoundComponent & comp );
};
