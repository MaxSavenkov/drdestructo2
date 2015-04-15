#pragma once
#include "IRender.h"

  // Defines key point of sky
struct SSkyKeyPoint
{
	float m_time; // percent of day, with 0=00:00, 0.5=12:00
	Color m_top;
	Color m_bottom;
};

  // Defines key point of stars
struct SStarsKeyPoint
{
	float m_time;
	bool m_visible;
	float m_transparency;
};

  // Defines key point of soon or moon (or other celestial body)
  // Only color is defined here: trajectory is hard-coded for now
struct SAstroKeyPoint
{
	float m_time;
	Color m_sunColor;
	Color m_moonColor;
};

struct SSkyControllerParams
{
	SSkyControllerParams()
		: m_active( false )
	{}

	bool m_active;

	typedef std::vector<SSkyKeyPoint> SkyKeyPoints;
	SkyKeyPoints m_skyKeyPoints;

	typedef std::vector<SStarsKeyPoint> StarsKeyPoints;
	StarsKeyPoints m_starsKeyPoints;

	typedef std::vector<SAstroKeyPoint> AstroKeyPoints;
	AstroKeyPoints m_astroKeyPoints;

	float m_skyPercent;
};

  // Fills sky bitmap and returns it for drawing
class SkyController
{
	ImageHandle m_skyImage;

	static const int MAX_STARS_SIZES = 2;
	ImageHandle m_starImages[ MAX_STARS_SIZES ];

	typedef std::vector<SSkyKeyPoint> SkyKeyPoints;
	SkyKeyPoints m_skyKeyPoints;

	typedef std::vector<SStarsKeyPoint> StarsKeyPoints;
	StarsKeyPoints m_starsKeyPoints;

	typedef std::vector<SAstroKeyPoint> AstroKeyPoints;
	AstroKeyPoints m_astroKeyPoints;

	float m_dayTime;
	float m_skyTime;
	float m_starsTime;
	float m_astroTime;

	int m_currentSky;
	int m_currentStars;
	int m_currentAstro;

	float m_skyPercent;

	struct SStar
	{
		int x;
		int y;
		int s;
	};

	static const int MAX_STARS = 100;
	SStar m_stars[ MAX_STARS ];

	int m_skyW;
	int m_skyH;

private:
	void RenderSky( IRender & render );
	void RenderStars( IRender & render );
	void RenderAstro( IRender & render );

public:
	SkyController()
	{
		Reset();
		m_skyW = -1;
		m_skyH = -1;
	}

	void Reset();
	void Setup( const SSkyControllerParams & params );
	void SetupDefault();
	void Render( IRender & render );
	void Update( float dt );
	
	ImageHandle GetImage() const { return m_skyImage; }
};
