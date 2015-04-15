#include "stdafx.h"
#include "SkyController.h"
#include "Common.h"
#include "Random.h"
#include <math.h>
#include "Engine/AllegroImage5.h"

void SkyController::Reset()
{
	m_dayTime = 0;
	m_skyTime = 0;
	m_astroTime = 0;
	m_starsTime = 0;
	m_currentSky = 0;
	m_currentAstro = 0;
	m_currentStars = 0;
}

void SkyController::Setup( const SSkyControllerParams & params )
{
	m_skyKeyPoints.clear();
	m_astroKeyPoints.clear();
	m_starsKeyPoints.clear();	

	if ( params.m_active )
	{
		m_skyKeyPoints = params.m_skyKeyPoints;
		m_astroKeyPoints = params.m_astroKeyPoints;
		m_starsKeyPoints = params.m_starsKeyPoints;
		m_skyPercent = params.m_skyPercent;
	}
	else
	{
		SetupDefault();
	}
}

void SkyController::SetupDefault()
{
	{
		SSkyKeyPoint midnight;
		midnight.m_time = 0.0f;
		midnight.m_top = Color( 10, 10, 50 );
		midnight.m_bottom = Color( 10, 15, 70 );

		SSkyKeyPoint sunrise;
		sunrise.m_time = 0.25f;
		sunrise.m_top = Color( 100, 160, 205 );
		sunrise.m_bottom = Color( 200, 150, 100 );

		SSkyKeyPoint midday;
		midday.m_time = 0.5f;
		midday.m_top = Color( 20, 100, 255 );
		midday.m_bottom = Color( 50, 170, 200 );

		SSkyKeyPoint sunset;
		sunset.m_time = 0.75f;
		sunset.m_top = Color( 60, 40, 155 );
		sunset.m_bottom = Color( 150, 120, 100 );

		m_skyPercent = 0.75f;

		m_skyKeyPoints.push_back( midnight );
		m_skyKeyPoints.push_back( sunrise );
		m_skyKeyPoints.push_back( midday );
		m_skyKeyPoints.push_back( sunset );
	}

	{
		SStarsKeyPoint midnight;
		midnight.m_time = 0.0f;
		midnight.m_visible = true;
		midnight.m_transparency = 1.0f;

		SStarsKeyPoint sunrise;
		sunrise.m_time = 0.25f;
		sunrise.m_visible = false;
		sunrise.m_transparency = 0.0f;

		SStarsKeyPoint midday;
		midday.m_time = 0.5f;
		midday.m_visible = false;
		midday.m_transparency = 0.0f;

		SStarsKeyPoint sunset;
		sunset.m_time = 0.75f;
		sunset.m_visible = true;
		sunset.m_transparency = 0.0f;

		m_starsKeyPoints.push_back( midnight );
		m_starsKeyPoints.push_back( sunrise );
		m_starsKeyPoints.push_back( midday );
		m_starsKeyPoints.push_back( sunset );
	}

	{
		SAstroKeyPoint midnight;
		midnight.m_time = 0.0f;
		midnight.m_moonColor = Color( 200, 200, 200 );
		midnight.m_sunColor = Color( 0, 0, 0 );

		SAstroKeyPoint sunrise;
		sunrise.m_time = 0.25f;
		sunrise.m_moonColor = Color( 150, 150, 150 );
		sunrise.m_sunColor = Color( 110, 110, 35 );

		SAstroKeyPoint midday;
		midday.m_time = 0.5f;
		midday.m_moonColor = Color( 0, 0, 0 );
		midday.m_sunColor = Color( 200, 200, 50 );

		SAstroKeyPoint sunset;
		sunset.m_time = 0.75f;
		sunset.m_moonColor = Color( 150, 150, 150 );
		sunset.m_sunColor = Color( 230, 120, 20 );

		m_astroKeyPoints.push_back( midnight );
		m_astroKeyPoints.push_back( sunrise );
		m_astroKeyPoints.push_back( midday );
		m_astroKeyPoints.push_back( sunset );
	}
}

template<typename TKey>
void SetTime( float dayTime, float & timeLocal, int & keyIndex, const std::vector<TKey> & keys )
{
	float keyLength = 0.0f;

	for ( size_t i = 0; i < keys.size(); ++i )
	{
		const TKey & curKey = keys[ i ];
		const TKey & nextKey = keys[ (i + 1) % keys.size() ];

		if ( i == keys.size() - 1 )
		{
			keyIndex = (int)i;
			keyLength = 1.0f - curKey.m_time;
			break;
		}
		
		if ( dayTime < nextKey.m_time )
		{
			keyIndex = (int)i;
			keyLength = nextKey.m_time - curKey.m_time;
			break;
		}
	}

	timeLocal = ( dayTime - keys[ keyIndex ].m_time ) / keyLength;
	if ( timeLocal < 0 )
		timeLocal = 0;
	if ( timeLocal >= 1.0f )
		timeLocal = 1.0f;
}

void SkyController::Update( float dayTime )
{
	m_dayTime = dayTime;

	if ( !m_skyKeyPoints.empty() )
	{
		SetTime( m_dayTime, m_skyTime, m_currentSky, m_skyKeyPoints );
	}

	if ( !m_starsKeyPoints.empty() )
	{
		SetTime( m_dayTime, m_starsTime, m_currentStars, m_starsKeyPoints );
	}

	if ( !m_astroKeyPoints.empty() )
	{
		SetTime( m_dayTime, m_astroTime, m_currentAstro, m_astroKeyPoints );
	}
}

void SkyController::Render( IRender & render )
{
	const int skyW = SCREEN_W;
	const int skyH = SCREEN_H * m_skyPercent;

	if ( !m_skyImage.IsValid() || m_skyW != skyW || m_skyH != skyH )
	{
		render.DestroyImage( m_skyImage );
		m_skyImage = render.CreateImage( skyW, skyH, true, "sky" );
		if ( !m_skyImage.IsValid() )
			return;
		m_skyW = skyW;
		m_skyH = skyH;

		for( int i = 0; i < MAX_STARS; ++i )
		{
			SStar & s = m_stars[i];
			s.x = RndInt( 0, skyW );
			s.y = RndInt( 0, skyH - 20 );
			s.s = RndInt( 0, MAX_STARS_SIZES - 1 );
		}
	}

	if ( !m_starImages[0].IsValid() )
	{
		m_starImages[0] = render.CreateImage( 1, 1, false, "star0" );
		m_starImages[1] = render.CreateImage( 3, 3, false, "star1" );

		render.ClearImage( m_starImages[0], Color(0,0,0,255) );
		render.DrawPixel( m_starImages[0], 0, 0, Color(255,255,255) );

		render.ClearImage( m_starImages[1], Color(0,0,0,255) );
		render.DrawPixel( m_starImages[1], 1, 1, Color(255,255,255,255) );
		render.DrawPixel( m_starImages[1], 0, 0, Color(255,255,255,255) );
		render.DrawPixel( m_starImages[1], 2, 2, Color(255,255,255,255) );
		render.DrawPixel( m_starImages[1], 0, 2, Color(255,255,255,255) );
		render.DrawPixel( m_starImages[1], 2, 0, Color(255,255,255,255) );
	}

	if ( !m_skyKeyPoints.empty() )
	{
		RenderSky( render );
	}

	if ( !m_starsKeyPoints.empty() )
	{
		RenderStars( render );
	}

	if ( !m_astroKeyPoints.empty() )
	{
		RenderAstro( render );
	}

	//render.SetFontSize(20);
	//render.DrawText( m_skyImage, 0,0, Color(255,255,255), "%f", m_dayTime);
	//render.DrawText( m_skyImage, 0,20, Color(255,255,255), "%f %i", m_skyTime, m_currentSky);
}

void SkyController::RenderSky( IRender & render )
{
	ImageHandle back = render.LoadImage( "Data/Sprites2/backart.png" );
	IImage *pImg = render.GetImageByHandle( back );
	if ( pImg )
	{
		int backW = pImg->GetWidth();
		int backH = pImg->GetHeight();
		render.CopyRegion( m_skyImage, back, 0, 0, 0, 0, backW, backH, SCREEN_W, SCREEN_H );
	}

	const SSkyKeyPoint & key1 = m_skyKeyPoints[ m_currentSky ];
	const SSkyKeyPoint & key2 = m_skyKeyPoints[ (m_currentSky+1) % (int)m_skyKeyPoints.size() ];
	const float tk = m_skyTime;

	const int r1 = key1.m_top.r + tk*(key2.m_top.r - key1.m_top.r);
	const int r2 = key1.m_bottom.r + tk*(key2.m_bottom.r - key1.m_bottom.r);

	const int g1 = key1.m_top.g + tk*(key2.m_top.g - key1.m_top.g);
	const int g2 = key1.m_bottom.g + tk*(key2.m_bottom.g - key1.m_bottom.g);

	const int b1 = key1.m_top.b + tk*(key2.m_top.b - key1.m_top.b);
	const int b2 = key1.m_bottom.b + tk*(key2.m_bottom.b - key1.m_bottom.b);

	ColorPoint p1, p2, p3, p4;
	p1.x = 0, p1.y = 0, p1.c = Color( r1, g1, b1, 150 );
	p2.x = m_skyW, p2.y = 0, p2.c = Color( r1, g1, b1, 150 );
	p3.x = m_skyW, p3.y = m_skyH, p3.c = Color( r2, g2, b2, 150 );
	p4.x = 0, p4.y = m_skyH, p4.c = Color( r2, g2, b2, 150 );
	render.DrawColoredQuad( m_skyImage, p1, p2, p3, p4 );
	/*for ( int y = 0; y <= m_skyH; ++y )
	{
		const float k = y / (float)m_skyH;
		int r = r1 + (r2-r1) * k;
		int g = g1 + (g2-g1) * k;
		int b = b1 + (b2-b1) * k;

		render.DrawLine( m_skyImage, 0, y, m_skyW, y, Color(r,g,b) );
	}*/
}

void SkyController::RenderStars( IRender & render )
{
	const SStarsKeyPoint & key1 = m_starsKeyPoints[ m_currentStars ];
	const SStarsKeyPoint & key2 = m_starsKeyPoints[ (m_currentStars+1) % (int)m_starsKeyPoints.size() ];

	if ( !key1.m_visible )
		return;

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
	render.BeginBitmapBatch( m_skyImage );
	
	for ( int i = 0; i < MAX_STARS; ++i )
	{
		float kT = key1.m_transparency + (key2.m_transparency-key1.m_transparency)*m_starsTime;
		float kY = (m_skyH-m_stars[i].y)/(float)m_skyH;
		float alpha = kT*(kY/(1.0f*(1-kT) + kY*kT))*255;
		Color tint(255,255,255, alpha );
			
		render.DrawTintedImage( m_starImages[ 1 ], tint, m_stars[i].x, m_stars[i].y );
	}
	render.EndBitmapBatch();
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
}

void SkyController::RenderAstro( IRender & render )
{
	// Parabola coeficients for astronomical bodies (sun & moon)
	float a,b,c;

	{
		float x1 = -200;
		float y1 = 300;
		float x2 = 512;
		float y2 = 200;
		float x3 = 1224;
		float y3 = 300;

		a = ( y3 - (x3*(y2-y1)+x2*y1-x1*y2)/(x2-x1) ) / ( x3*(x3-x1-x2)+x1*x2 );
		b = ( y2 - y1 ) / ( x2 - x1 ) - a*(x1+x2);
		c = ( x2*y1 - x1*y2 )/(x2-x1) + a*x1*x2;
	}

	const SAstroKeyPoint & key1 = m_astroKeyPoints[ m_currentAstro ];
	const SAstroKeyPoint & key2 = m_astroKeyPoints[ (m_currentAstro+1) % (int)m_astroKeyPoints.size() ];

	const int mr = key1.m_moonColor.r + (key2.m_moonColor.r-key1.m_moonColor.r)*m_astroTime;
	const int mg = key1.m_moonColor.g + (key2.m_moonColor.g-key1.m_moonColor.g)*m_astroTime;
	const int mb = key1.m_moonColor.b + (key2.m_moonColor.b-key1.m_moonColor.b)*m_astroTime;

	const int sr = key1.m_sunColor.r + (key2.m_sunColor.r-key1.m_sunColor.r)*m_astroTime;
	const int sg = key1.m_sunColor.g + (key2.m_sunColor.g-key1.m_sunColor.g)*m_astroTime;
	const int sb = key1.m_sunColor.b + (key2.m_sunColor.b-key1.m_sunColor.b)*m_astroTime;

	if ( m_dayTime < 0.25f )
	{
		const float x = m_skyW/2 + (m_skyW+200 - m_skyW/2) * ( m_dayTime - 0.0f ) / 0.25f;
		const float y = a*x*x + b*x + c;
		render.DrawCircleFill( m_skyImage, x, y, 30, Color(mr,mg,mb) );
	}
	else if ( m_dayTime < 0.5f )
	{
		float x = -200 + (m_skyW/2 + 200) * ( m_dayTime - 0.25f ) / 0.25f;
		float y = a*x*x + b*x + c;
		render.DrawCircleFill( m_skyImage, x, y, 40, Color(sr,sg,sb) );
	}
	else if ( m_dayTime < 0.75f )
	{
		const float x = m_skyW/2 + (m_skyW+200 - m_skyW/2) * ( m_dayTime - 0.5f ) / 0.25f;
		float y = a*x*x + b*x + c;
		render.DrawCircleFill( m_skyImage, x, y, 40, Color(sr,sg,sb) );
	}
	else
	{
		float x = -200 + (m_skyW/2 + 200) * ( m_dayTime - 0.75f ) / 0.25f;
		float y = a*x*x + b*x + c;
		render.DrawCircleFill( m_skyImage, x, y, 30, Color(mr,mg,mb) );
	}
}
