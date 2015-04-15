#pragma once
#include "IRender.h"
#include "IUTFString.h"

/*
	Class for printing text in limited are with
	automatic word breaks. IGNORES line breaks!
*/
class TextPrinter
{
	UTFString m_text;
	int m_textLen;
	int m_wordEnd;
	int m_absIndex;
	int m_wordIndex;
	int m_cx;
	int m_cy;
	int m_x;
	int m_y;
	int m_w;
	int m_h;
	int m_fontSize;
public:
	TextPrinter()
		: m_textLen( 0 )
		, m_fontSize( 20 )
	{}

	void SetRegion( int x, int y, int w, int h );
	void SetTextParams( int fontSize );
	void SetText( const UTFString & text );
	bool Update( ImageHandle image, IRender & render );
	bool HasText() { return !m_text->Empty() != 0; }
};
