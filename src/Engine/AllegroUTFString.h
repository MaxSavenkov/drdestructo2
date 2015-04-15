#pragma once

#include "IUTFString.h"

class AllegroUTFString5 : public IUTFString
{
public:
	AllegroUTFString5()
		: m_pString( al_ustr_empty_string() )
		, m_default( true )
	{}

	~AllegroUTFString5()
	{
		if ( !m_default )
			al_ustr_free( (ALLEGRO_USTR*)m_pString );
	}

	void Assign( const char *buffer, int length )
	{
		if ( !m_default )
			al_ustr_free( (ALLEGRO_USTR*)m_pString );

		m_pString = al_ustr_new_from_utf16( (const uint16_t*)buffer );
		m_default = false;
	}

	void AssignCstr( const char *str )
	{
		if ( !m_default )
			al_ustr_free( (ALLEGRO_USTR*)m_pString );

		m_pString = al_ustr_new( str );
		m_default = false;
	}

	int GetChar( int index ) const
	{
		if ( index >= (int)al_ustr_length( m_pString ) )
			return 0;

		const int pos = al_ustr_offset( m_pString, index );

		return al_ustr_get( m_pString, pos );
	}

	bool Empty() const
	{
		return ( GetChar( 0 ) == 0 );
	}

	int Length() const
	{
		return al_ustr_length( m_pString );
	}

public:
	const ALLEGRO_USTR *GetString() const { return m_pString; }

private:
	const ALLEGRO_USTR *m_pString;
	bool m_default;
};

UTFString::UTFString()
{
	m_pString.reset( new AllegroUTFString5() );
}
