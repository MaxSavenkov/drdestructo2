#pragma once
  // Global functions for generating random numbers instead of rand(). Probab;y not thread-safe, but I have no need for this.
int RndInt( int f, int t );
float RndFloat( float f, float t );