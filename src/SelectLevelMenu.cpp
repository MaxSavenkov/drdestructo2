#include "stdafx.h"
#include "SelectLevelMenu.h"
#include "GameContext.h"

  // Class that passed to IGameContext to get
  // list of levels for displaying in Level Select menu
class LevelsEnumerator : public ILevelsEnumerator
{
	MenuSelectLevel *m_pMenu;
public:
	LevelsEnumerator( MenuSelectLevel *pMenu )
		: m_pMenu( pMenu )
	{}

	void operator()( const char *levelName, const char *levelObject, int index, bool locked )
	{
		if ( index <= 0 )
			return;

		if ( !locked )
			m_pMenu->AddLevel( levelName, levelObject );
	}
};

void LevelMenuEntry::Render( IRender *pRender, int x, int y )
{
	if ( !m_rightOn.IsValid() )
	{
		m_rightOff = pRender->LoadImage( "Data/Sprites2/ui/selectlevel_btn_normal.png" );
		m_rightOn = pRender->LoadImage( "Data/Sprites2/ui/selectlevel_btn_selected.png" );
		m_arrowLeft = pRender->LoadImage( "Data/Sprites2/ui/arrow_left_selected.png" );
		m_arrowRight = pRender->LoadImage( "Data/Sprites2/ui/arrow_right_selected.png" );
		m_arrowLeftPressed = pRender->LoadImage( "Data/Sprites2/ui/arrow_left_normal.png" );
		m_arrowRightPressed = pRender->LoadImage( "Data/Sprites2/ui/arrow_right_normal.png" );
	}

	m_screenX = x + m_x + m_w - 577 - 68;

	if ( !m_selected )
		pRender->DrawImage( m_rightOff, x + m_x + m_w - 577 - 68, y + m_y );
	else
	{
		pRender->DrawImage( m_rightOn, x + m_x + m_w - 577 - 68, y + m_y );
		if ( m_needArrows )
		{
			if ( m_pressedLeft )
				pRender->DrawImage( m_arrowLeftPressed, x + m_x + m_w - 577 - 68 + 20, y + m_y + 10 );
			else
				pRender->DrawImage( m_arrowLeft, x + m_x + m_w - 577 - 68 + 20, y + m_y + 10 );

			if ( m_pressedRight )
				pRender->DrawImage( m_arrowRightPressed, x + m_x + m_w - 68 - 125, y + m_y + 10 );				
			else
				pRender->DrawImage( m_arrowRight, x + m_x + m_w - 68 - 125, y + m_y + 10 );
		}
	}

	Color off( 73, 198, 135 );
	Color on( 136, 247, 27 );

	const char *value = CurrentValue();
	pRender->DrawAlignedText( x + m_x + m_w - 577/2 - 101, y + m_y + 13, m_selected ? on : off, 25, TEXT_ALIGN_CENTER, "%s", value );
}

void MenuSelectLevel::AddLevel( const char *levelName, const char *levelObject )
{
	m_level.AddValue( levelName, levelObject );
}

void MenuSelectLevel::ReInit( IGameContext & context )
{
	m_level.ClearValues();

	LevelsEnumerator e( this );
	context.EnumerateLevels( e );

	m_level.SetSelectedIndex( m_level.GetValuesCount() - 1 );

	m_level.SetY( 255 );

	m_wantLevelObject = *m_level.GetSelectedData();

	m_state = STATE_STAY;
	m_timer = 0;

	SetSelected( &m_start );
}

void MenuSelectLevel::operator()( int id )
{
	if ( id == EID_BACK )
		m_pCallback->BackFromSelectLevel();
	else if ( id == EID_START )
		m_pCallback->StartLevel( m_level.GetSelectedIndex()+1 );
}

void MenuSelectLevel::Render( IRender *pRender )
{
	GameMenu::Render( pRender );

	if ( m_levelObjectImage.IsValid() )
	{
		const IImage *pImage = pRender->GetImageByHandle( m_levelObjectImage );
		if ( !pImage )
			return;

		const int w = pImage->GetWidth()*0.5f;
		const int h = pImage->GetHeight()*0.5f;

		const int x = m_x + m_width/2 - w / 2;
		const int y = m_y + m_height/2 - h / 2 - 40;
		
		float opacity = 1.0f;
		switch( m_state )
		{
			case STATE_FADE_IN:
				opacity = 1.0f - m_timer / (float)FADE_TIME;
				break;

			case STATE_FADE_OUT:
				opacity = m_timer / (float)FADE_TIME;
				break;
		}

		int a,b,c;
		al_get_blender( &a, &b, &c );
		al_set_blender( ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA );
		//pRender->StretchImage( m_levelObjectImage, x, y, 0.5f, 0.5f, 0 );
		pRender->DrawTintedImage( m_levelObjectImage, Color(255,255,255,opacity*255), x, y, 0, 0, 0, 0, 0, w, h, 0 );
		al_set_blender( a, b, c );
	}
}

void MenuSelectLevel::UpdateLevel( IGameContext & context, float dt )
{
	bool updatePicture = false;

	if ( m_state != STATE_STAY )
	{
		m_timer -= dt * 1000;
		if ( m_timer <= 0 )
		{
			if ( m_state == STATE_FADE_OUT )
			{
				updatePicture = true;

				m_state = STATE_FADE_IN;
				m_timer = FADE_TIME;
			}
			else
				m_state = STATE_STAY;
		}
	}
	else
		updatePicture = true;

	if ( updatePicture && !m_wantLevelObject.empty() )
	{
		const char *pictureFile = context.GetObjectPictureForMenu( m_wantLevelObject.c_str() );
		m_levelObjectImage = GetRender().LoadImage( pictureFile );
		m_wantLevelObject = "";
	}
}

void MenuSelectLevel::OnLevelSelected( const char *levelObject )
{
	m_wantLevelObject = levelObject;

	if ( m_state == STATE_FADE_IN )
	{
		m_state = STATE_FADE_OUT;
		m_timer = FADE_TIME - m_timer;
	}
	else if ( m_state == STATE_STAY )
	{
		m_state = STATE_FADE_OUT;
		m_timer = FADE_TIME;
	}

}
