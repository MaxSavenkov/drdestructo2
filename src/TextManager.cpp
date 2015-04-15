#include "stdafx.h"
#include "TextManager.h"
#include "Engine/ISystem.h"

Text *TextManager::Load( const std::string & path )
{
	ISystem & sys = GetSystem();
	std::vector<char> buf;
	if ( !sys.LoadFile( true, path.c_str(), buf ) )
		return 0;

	buf.push_back( 0 );
	buf.push_back( 0 );

	Text *text = new Text();
	text->m_text->Assign( &buf[0], buf.size() );
	return text;
}

void TextManager::Delete( Text *res )
{
	delete res;
}

TextManager & GetTextManager()
{
	static TextManager m;
	return m;
}