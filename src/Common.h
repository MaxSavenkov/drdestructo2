#pragma once
#include <math.h>

  // Rotates specified point around (0,0) for a specified angle. Changes passed variables!
inline void Rotate( float & x, float & y, float angle )
{
	const float cosa = cosf( angle );
	const float sina = sinf( angle );

	const float modx = cosa * x + sina * y;
	const float mody = -cosa * y + sina * x;

	x = modx;
	y = mody;
}

  // Global options version. Can be used to determine that options has changed in any part of program
extern int g_options_version;

  // Base screen dimensions from which image is scaled. Change this if want bigger or smaller base screen,
  // but beware: many things may break
static const int SCREEN_W = 1024; //1280;
static const int SCREEN_H = 768; //960;

static const int STAT_TYPES_COUNT = 13;