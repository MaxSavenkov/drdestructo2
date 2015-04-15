#pragma once

/*
	This class stores a 2d array of bits.
	It is used for per-pixel collision mask of targets (ships, castles etc.)
*/
class BitArray2D
{
	int *m_data;
	int m_w, m_h;
	size_t m_len;
public:
	BitArray2D()
		: m_data( 0 )
		, m_w( 0 )
		, m_h( 0 )
		, m_len( 0 )
	{}

	~BitArray2D()
	{
		delete [] m_data;
		m_data = 0;
	}

	BitArray2D( const BitArray2D & other )
	{
		if ( other.m_len > 0 )
		{
			m_data = new int[ other.m_len ];
			m_len = other.m_len;
			m_w = other.m_w;
			m_h = other.m_h;
			memcpy( m_data, other.m_data, m_len*sizeof(int) );
		}
		else
		{
			m_data = 0;
			m_len = 0;
			m_w = 0;
			m_h = 0;
		}
	}

	void operator = ( const BitArray2D & other )
	{
		if ( this == &other )
			return;

		if ( other.m_len > 0 )
		{
			delete [] m_data;
			m_data = new int[ other.m_len ];
			m_len = other.m_len;
			m_w = other.m_w;
			m_h = other.m_h;
			memcpy( m_data, other.m_data, m_len*sizeof(int) );
		}
		else
		{
			delete [] m_data;
			m_data = 0;
			m_len = 0;
			m_w = 0;
			m_h = 0;		
		}
	}

	void Create( int w, int h )
	{
		delete [] m_data;

		m_len = ( w * h ) / sizeof(int) + 1;
		m_w = w;
		m_h = h;

		m_data = new int[ m_len ];
		memset( &m_data[0], 0, sizeof(int)*m_len );
	}

	void Set( int x, int y, bool value )
	{
		const size_t indexInt = ( y * m_w + x ) / sizeof(int);
		const size_t indexBit = ( y * m_w + x ) % sizeof(int);

		if ( indexInt >= m_len )
			return;

		if ( value )
			m_data[ indexInt ] |= ( 1 << indexBit );
		else
			m_data[ indexInt ] &= ~( 1 << indexBit );
	}

	bool Get( int x, int y ) const
	{
		const size_t indexInt = ( y * m_w + x ) / sizeof(int);
		const size_t indexBit = ( y * m_w + x ) % sizeof(int);

		if ( indexInt >= m_len )
			return false;
		return ( m_data[ indexInt ] & ( 1 << indexBit ) ) != 0;
	}

	bool IsEmpty() const
	{
		return m_data == 0;
	}

	int GetWidth() const
	{
		return m_w;
	}

	int GetHeight() const
	{
		return m_h;
	}
};
