#include "stdafx.h"
#include "AllegroImage5.h"

int AllegroImage5::GetWidth() const
{
	return al_get_bitmap_width( m_bitmap );
}

int AllegroImage5::GetHeight() const
{
	return al_get_bitmap_height( m_bitmap );
}
