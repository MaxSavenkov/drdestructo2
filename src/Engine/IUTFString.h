#pragma once

class IUTFString
{
public:
	virtual void Assign( const char *buffer, int length ) = 0;
	virtual void AssignCstr( const char *str ) = 0;
	virtual int GetChar( int index ) const = 0;
	virtual bool Empty() const = 0;
	virtual int Length() const = 0;
};

  // Class for working with UTF strings, because briefing files are written in UTF8 and Android doesn't support wchar_t
class UTFString
{
public:
	UTFString();

	UTFString( const UTFString & other ) { if ( this != &other ) m_pString = other.m_pString; }
	void operator = ( const UTFString & other ) { if ( this != &other ) m_pString = other.m_pString; }
	operator IUTFString *() const { return m_pString.get(); }
	IUTFString * operator -> () const { return m_pString.get(); }

private:
#ifndef HAS_STD_SHARED_PTR
	boost::shared_ptr<IUTFString> m_pString;
#else
	std::shared_ptr<IUTFString> m_pString;
#endif
};
