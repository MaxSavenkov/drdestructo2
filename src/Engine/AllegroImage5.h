#pragma once
#include "IImage.h"

struct ALLEGRO_BITMAP;

/*
	Implementation of IImage interface for Allegro5
*/
class AllegroImage5 : public IImage
{
	ALLEGRO_BITMAP *m_bitmap;
	  // This flag is true for bitmaps that should not
	  // be automatically deleted
	bool m_system;
public:
	AllegroImage5( ALLEGRO_BITMAP *bmp, bool system = false )
		: m_bitmap( bmp )
		, m_system( system )
	{}

	~AllegroImage5();

	int GetWidth() const;
	int GetHeight() const;
	ALLEGRO_BITMAP *GetBitmap() const { return m_bitmap; }
	void SetBitmap( ALLEGRO_BITMAP *bmp ) { m_bitmap = bmp; }
};
