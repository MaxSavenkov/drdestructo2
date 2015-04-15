#include "stdafx.h"
#include "TextPrinter.h"

void TextPrinter::SetRegion( int x, int y, int w, int h )
{
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	m_cx = 0;
	m_cy = 0;
}

void TextPrinter::SetTextParams( int fontSize )
{
	m_fontSize = fontSize;
}

void TextPrinter::SetText( const UTFString & text )
{
	m_text = text;

	m_textLen = text->Length();
	m_absIndex = 0;
	m_wordIndex = 0;
	m_wordEnd = 0;
	m_cx = 0;
	m_cy = 0;
}

bool TextPrinter::Update( ImageHandle image, IRender & render )
{
	if ( m_text->Empty() )
		return false;

	if ( m_wordIndex >= m_wordEnd )
	{
		if ( m_absIndex >= m_textLen )
			return false;

		while( m_absIndex < m_textLen )
		{
			if ( m_text->GetChar( m_absIndex ) == ' ' || m_text->GetChar( m_absIndex ) == '\n' )
				break;
			++m_absIndex;
		}

		m_wordEnd = m_absIndex + 1;
		++m_absIndex;

		const int wordLen = m_wordEnd - m_wordIndex;
		if ( wordLen > 0 )
		{
			const int wordWidth = render.GetStringWidth( m_text, m_wordIndex, wordLen, m_fontSize );
			if ( m_cx + wordWidth >= m_w )
			{
				m_cx = 0;
				m_cy += m_fontSize + 2;
			}
		}
	}
	else
	{
		const int ch = m_text->GetChar( m_wordIndex );
		const int chWidth = render.GetCharWidth( (wchar_t)ch, m_fontSize );

		Color color( 200, 200, 200 );

		if ( iswprint( ch ) )
		{
			if ( image.IsValid() )
				render.DrawText( image, m_x + m_cx, m_y + m_cy, color, m_fontSize, m_text, m_wordIndex, 1 );
			else
				render.DrawText( m_x + m_cx, m_y + m_cy, color, m_fontSize, m_text, m_wordIndex, 1 );

			m_cx += chWidth;
		}
		else if ( m_wordIndex > 0 )
		{
			const bool isWspace = ( ch == '\n' ||  ch == '\r' || iswspace( ch ) );
			const int prevCh = m_text->GetChar( m_wordIndex - 1 );
			const bool isPrevWspace = ( prevCh == '\n' ||  prevCh == '\r' || iswspace( prevCh ) );
			if (  isWspace && !isPrevWspace )
				m_cx += chWidth;
		}
		
		++m_wordIndex;
	}

	return true;
}
