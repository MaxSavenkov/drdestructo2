#include "stdafx.h"
#include "Common.h"
#include "GameStateScoreTable.h"
#include "ObjectsStorage.h"

const float ScoreEntry::BLINK_TIME = 0.5f;

void GameStateScoreTable::ProcessInput( IInput & input )
{
	if ( !m_finish )
		m_input.ProcessInput( &input );
	
	BaseGameState::ProcessInput( input );
}

bool GameStateScoreTable::LoadScores()
{
	ISystem & sys = GetSystem();
	std::vector<char> buf;
	if ( !sys.LoadFile( false, "scores.xml", buf ) )
		return 0;

	TiXmlDocument doc;
	if ( !doc.Parse( &buf[0] ) )
		return false;
	
	const TiXmlElement *pRoot = doc.FirstChildElement("Scores");
	const TiXmlElement *pScore = 0;

	int scoreIndex = 0;

	while( pScore = pScore ? pScore->NextSiblingElement("Score") : pRoot->FirstChildElement( "Score" ) )
	{
		int score;
		std::string name;
		std::string date;

		if ( pScore->QueryValueAttribute( "Score", &score ) != TIXML_SUCCESS )
			continue;
		if ( pScore->QueryValueAttribute( "Name", &name ) != TIXML_SUCCESS )
			continue;
		if ( pScore->QueryValueAttribute( "Date", &date ) != TIXML_SUCCESS )
			continue;

		if ( name.length() > MAX_NAME_LEN )
			name = name.substr( 0, MAX_NAME_LEN );

		m_scoreEntry[ scoreIndex ].score = score;
		m_scoreEntry[ scoreIndex ].name = name;
		m_scoreEntry[ scoreIndex ].date = date;

		++scoreIndex;
		if ( scoreIndex >= MAX_SCORES )
			break;
	}

	std::sort( &m_scoreEntry[0], &m_scoreEntry[ MAX_SCORES ], std::greater<SScoreEntry>() );
	return true;
}

bool GameStateScoreTable::SaveScores()
{
	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8","yes");
	doc.LinkEndChild( decl );
	TiXmlElement *root = new TiXmlElement("Scores");
	doc.LinkEndChild( root );

	for ( int i = 0; i < MAX_SCORES; ++i )
	{
		const SScoreEntry & e = m_scoreEntry[ i ];
		TiXmlElement *newNode = new TiXmlElement("Score");
		newNode->SetAttribute( "Name", e.name );
		newNode->SetAttribute( "Score", e.score);
		newNode->SetAttribute( "Date", e.date );
		root->LinkEndChild( newNode );
	}

	std::string path = GetSystem().GetWriteablePath();
	if ( !path.empty() && *path.rbegin() != '/' && *path.rbegin() != '\\' )
		path += "/";
	path += "scores.xml";

	return doc.SaveFile( path.c_str() );
}

void GameStateScoreTable::DrawScoreEntry( const SScoreEntry & e, int x, int y, int pos, const Color & color )
{
	IRender & render = GetRender();

	const int x1 = x + 45;
	const int x2 = x1 + 20;
	const int x3 = x2 + 380;
	const int x4 = x3 + 300;
		
	render.DrawAlignedText( m_mask, x1, y, Color(255,255,255,255), 30, TEXT_ALIGN_RIGHT, "%2i.", pos );
	render.DrawText( m_mask, x2, y, Color(255,255,255,255), 30, "%-10.10s", e.name.c_str() );
	render.DrawAlignedText( m_mask, x3, y, Color(255,255,255,255), 30, TEXT_ALIGN_CENTER, "%i", e.score );
	render.DrawAlignedText( m_mask, x4, y, Color(255,255,255,255), 30, TEXT_ALIGN_CENTER, "%s", e.date.c_str() );
}

void GameStateScoreTable::RenderBefore( IRender & render, const IGameContext & context )
{
	render.ClearToColor(Color(0,0,0));
	if ( !m_mask.IsValid() )
	{
		m_mask = render.CreateImage( SCREEN_W, SCREEN_H, false, "mask" );
	}

	if ( !m_bg.IsValid() )
	{
		  // Generation of moving background image.
		  // It's just a lot of colored rectangles, actually :)
		m_bg = render.CreateImage( SCREEN_W, SCREEN_H, true, "bg" );
		const int STEPS_X = 127;
		const int STEPS_Y = 127;
		const int SIZE_X = SCREEN_W / STEPS_X + 1;
		const int SIZE_Y = SCREEN_H / STEPS_Y + 1;

		for ( int x = 0; x < STEPS_X; ++x)
		{
			for ( int y = 0; y < STEPS_Y; ++y )
			{
				const int r = (50+x*2) % 255;
				const int g = (50+y*2) % 255;
				const int b = 255 - (int)sqrtf( x*x + y*y ) % 255;				
				render.DrawRectFill( m_bg, SCREEN_W/2 - x*SIZE_X, y*SIZE_Y, SCREEN_W/2 - (x+1)*SIZE_X, (y+1)*SIZE_Y, Color(r,g,b));
				render.DrawRectFill( m_bg, SCREEN_W/2 + x*SIZE_X, y*SIZE_Y, SCREEN_W/2 + (x+1)*SIZE_X, (y+1)*SIZE_Y, Color(r,g,b));
			}
		}
	}

	if ( m_regenerateText )
	{
		  // Generation of transparent text

		  // Fill mask with opaque black
		render.ClearImage( m_mask, Color(0,0,0) );
		render.DrawAlignedText( m_mask, SCREEN_W / 2, 20, Color(255,255,255), 30, TEXT_ALIGN_CENTER, "HIGH SCORES:" );
		
		const int x = SCREEN_W / 2 - 460;
		const int x1 = x;
		const int x2 = x1 + 70;
		const int x3 = x2 + 330;
		const int x4 = x3 + 270;

		render.DrawText( m_mask, x1, 50, Color(200,200,200,0), 40, "Pos" );
		render.DrawText( m_mask, x2, 50, Color(200,200,200,0), 40, "Name" );
		render.DrawText( m_mask, x3, 50, Color(200,200,200,0), 40, "Score" );
		render.DrawText( m_mask, x4, 50, Color(200,200,200,0), 40, "Date/Time" );

		  // Substract each letter from mask
		int a,b,c,d,e,f;
		al_get_separate_blender( &a, &b, &c, &d, &e, &f );
		al_set_blender(ALLEGRO_DEST_MINUS_SRC , ALLEGRO_ONE , ALLEGRO_ONE);
		for ( int i = 0; i < MAX_SCORES; ++i )
		{
			if ( i == m_entryPos )
			{
				m_inputX = SCREEN_W / 2 - 380 - 17;
				m_inputY = 85 + i * 30;
				continue;
			}

			const SScoreEntry & ent = m_scoreEntry[ i ];
			DrawScoreEntry( ent, SCREEN_W / 2 - 460, 85 + i * 30,  i+1, Color(255,255,255,255) );
		}
		al_set_separate_blender( a,b,c,d,e,f );
		m_regenerateText = false;
	}

	render.DrawImage( m_bg, m_bgX, 0 );
	render.DrawImage( m_bg, -(SCREEN_W - m_bgX), 0 );
	render.DrawImage( m_mask, 0, 0 );

	if ( m_entryPos >= 0 )
	{
		render.DrawRectFill( m_mask, SCREEN_W / 2 - 460, 85 + m_entryPos * 30, SCREEN_W, 85 + (m_entryPos+1) * 30, Color(0,0,0) );
		const SScoreEntry & e = m_scoreEntry[ m_entryPos ];

		DrawScoreEntry( e, SCREEN_W / 2 - 460, 85 + m_entryPos * 30, m_entryPos+1, Color(255,255,255,255) );

		m_input.Render( &render, m_inputX, m_inputY );
	}

	if ( m_finish )
	{
		int c = 100 + 155 * (cosf(2*ALLEGRO_PI*(m_bgX/(float)SCREEN_W)) + 1.0f)/2.0f;
		if ( c > 255 )
			c = 255;
		render.DrawAlignedText( SCREEN_W/2, SCREEN_H - 40, Color(c,c,c), 20, TEXT_ALIGN_CENTER, "Press Esc to return to menu" );
	}
}

void GameStateScoreTable::RenderAfter( IRender & render, const IGameContext & context )
{

}

void GameStateScoreTable::Update( float dt, IGameContext & context )
{
	if ( !m_finish )
		m_input.Update( dt );
	else
		context.GetOjectsStorage().GetPlayer().m_score = 0;

	m_bgX += 500 * dt;
	if ( m_bgX >= SCREEN_W )
		m_bgX -= SCREEN_W;

	if ( m_exit )
	{
		context.RemoveState( GAMESTATE_SCORES );
		context.PushState( GAMESTATE_MENU );
	}

	m_input.Update( dt );
}

void GameStateScoreTable::OnPush( IGameContext & context )
{
	LoadScores();

	m_entryPos = -1;
	m_finish = true;

	const int curScore = context.GetOjectsStorage().GetPlayer().m_score;
	for ( int i = 0; i < MAX_SCORES; ++i )
	{
		if ( curScore > m_scoreEntry[ i ].score )
		{
			for ( int j = MAX_SCORES - 1; j > i; --j )
				m_scoreEntry[ j ] = m_scoreEntry[ j - 1 ];

			char dateStr[ 256 ];
			time_t t;
			time(&t);
			tm *date = localtime( &t );
			strftime( dateStr, 255, "%d.%m.%Y %H:%M", date );
			m_scoreEntry[ i ].flags = 0;
			m_scoreEntry[ i ].score = curScore;
			m_scoreEntry[ i ].name = "";
			m_scoreEntry[ i ].date = dateStr;
			m_entryPos = i;
			m_input.StartEdit();
			m_finish = false;
						
			break;
		}
	}

	m_input.SetValue( "" );
	m_bgX = 0;
	m_regenerateText = true;
	m_exit = false;
}

void GameStateScoreTable::AcceptEdit( const std::string & value )
{
	if ( m_entryPos >= 0 )
	{
		m_scoreEntry[ m_entryPos ].name = value;	
		m_entryPos = -1;

		  // Force regeneration of text mask
		m_regenerateText = true;
		SaveScores();
	}
	m_finish = true;
}

void GameStateScoreTable::CancelEdit()
{
	m_input.StartEdit();
}

bool GameStateScoreTable::Exit( const InputEvent & e )
{
	if ( m_finish )
	{
		m_exit = true;
	}

	return true;
}
