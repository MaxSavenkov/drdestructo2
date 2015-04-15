#pragma once

/*
  A handle to a resource. Identifies resource in array.
  Uses passports to allow index reuse.
*/ 
template<typename TResource>
class Handle
{
	  // Resource's index in array
	unsigned int m_index;
	  // Resource's passport - increased each time this index is reused
	unsigned int m_passport;

public:
	Handle()
		: m_index( UINT_MAX )
		, m_passport( 0 )
	{}

	Handle( unsigned int index, unsigned int passport )
		: m_index( index )
		, m_passport( passport )
	{}

	static Handle<TResource> INVALID;

	bool operator == ( const Handle & other ) const
	{
		return m_index == other.m_index && m_passport == other.m_passport;
	}

	bool operator != ( const Handle & other ) const
	{
		return m_index != other.m_index || m_passport != other.m_passport;
	}

	unsigned int GetIndex() const { return m_index; }
	unsigned int GetPassport() const { return m_passport; }
	bool IsValid() const { return *this != INVALID; }
};

template<typename TResource>Handle<TResource> Handle<TResource>::INVALID;
