#include "stdafx.h"
#include "Random.h"

boost::mt19937 & GetGen()
{
	static boost::mt19937 gen(time(0));
	return gen;
}

int RndInt( int f, int t )
{
	boost::uniform_int<> dist(f, t);
	return dist( GetGen() );
}

float RndFloat( float f, float t )
{
	boost::uniform_real<> dist(f, t);
	return dist( GetGen() );
}
