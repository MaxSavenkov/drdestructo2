#pragma once
#include "ISystem.h"

/*
	Player's component. Stores current information about player.
*/
class Player
{
public:
	Player()
		: m_score( 0 )
		, m_lives( 7 )
	{
		const int lives = GetSystem().GetConfigValue_Int( "DD_Gameplay", "Lives" );
		if ( lives > 0 && lives <= 21 )
			m_lives = lives;
	}

	int m_score;
	int m_lives;
};
