#include "stdafx.h"
#include "AllegroRender5.h"
#include "AllegroImage5.h"
#include "AllegroUTFString.h"
#include "LogCommon.h"
#include "BitmapManager.h"

/*
	Too much code here to comment properly.
	Some things are commented, though...
*/

#ifndef min
    #define min( a, b ) (a) < (b) ? (a) : (b)
#endif
#ifndef max
    #define max( a, b ) (a) > (b) ? (a) : (b)
#endif

AllegroImage5 *AllegroRender5::GetImage( ImageHandle handle )
{
	return static_cast< AllegroImage5* >( GetBitmapManager().GetResource( handle ) );
}

bool AllegroRender5::Init( ERenderType type, int w, int h, int bpp, bool fullscreen, bool vsync, const char *defaultFont )
{
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_MIPMAP );
	al_set_new_bitmap_flags( ALLEGRO_VIDEO_BITMAP );
	al_set_new_display_option( ALLEGRO_VSYNC, vsync ? 1 : 2, ALLEGRO_SUGGEST );
#ifdef ALLEGRO_IPHONE
	al_set_new_display_option( ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_REQUIRE );
#endif
	#ifdef WIN32
	    al_set_new_display_flags( (fullscreen ? ALLEGRO_FULLSCREEN : ALLEGRO_WINDOWED) | ( type == RENDERTYPE_DIRECT3D ? ALLEGRO_DIRECT3D : ALLEGRO_OPENGL ) );
	#else
	    al_set_new_display_flags( fullscreen ? ALLEGRO_FULLSCREEN : ALLEGRO_WINDOWED | ALLEGRO_OPENGL );
	#endif

	  // Trying to create a window that's larget than desktop leads to a crash on older version of Windows, so try to correct that
	if ( !fullscreen )
	{
		ALLEGRO_MONITOR_INFO info;
		al_get_monitor_info(0, &info);

		const int deskW = info.x2 - info.x1;
		const int deskH = info.y2 - info.y1;
		if ( w > deskW || h > deskH )
		{
			w = deskW;
			h = deskH;
		}
	}

	m_display = al_create_display(w, h);

	if (!m_display)
	{		
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroRender5: Failed to initialize render ( %ix%i %s )", 
			w, h, fullscreen ? "fullscreen" : "windowed" );
		return false;
	}

	al_init_font_addon();
	al_init_ttf_addon();
	al_init_image_addon();
	al_init_primitives_addon();

	m_fontSize = 16;
	m_defaultFont = CreateFont( defaultFont );
	if ( !m_defaultFont || !((AllegroFont5*)m_defaultFont)->GetFont( m_fontSize ) )
	{
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroRender5: Failed to load default font ( %s )", 
			defaultFont );
		return false;
	}
	m_currentFont = m_defaultFont;

	GetLog().Log( CommonLog(), LL_INFO, "AllegroRender5: Initialized render ( %dx%d %s )", 
		w, h, fullscreen ? "fullscreen" : "windowed" );

	m_showingMouse = false;

	m_fullScreen = fullscreen;
	m_renderType = type;
	m_vsync = vsync;

	m_fontSize = 10;

	ReapplyScaling();

	m_eventsQueue = al_create_event_queue();
	al_register_event_source( m_eventsQueue, al_get_display_event_source( m_display ) );

	return true;
}

void AllegroRender5::SetGlobalScaling( EScalingMode mode, float baseW, float baseH )
{
	m_scalingMode = mode;
	m_scalingBaseW = baseW;
	m_scalingBaseH = baseH;
	ReapplyScaling();
}

void AllegroRender5::ReapplyScaling()
{
	switch( m_scalingMode )
	{
		case SCALING_KEEP_ASPECT:
		{
			const int w = GetWidth();
			const int h = GetHeight();
			  // sW and sH are ratios of current screen size to specified base screen size
			const float sW = w / m_scalingBaseW;
			const float sH = h / m_scalingBaseH;
			  // We are going to use the lesser of them and therefor keep aspect ration
			const float minS = min( sW, sH );
			ALLEGRO_TRANSFORM t;
			al_identity_transform(&t);
			  // Scale image
			al_scale_transform( &t, minS, minS );
			  // Place image at the center of the screen
			al_translate_transform(&t, (w - m_scalingBaseW*minS)/2.0f, (h - m_scalingBaseH*minS)/2.0f );
			  // Set global transform
			al_use_transform( &t );

			//------------ Adjust clipping (for wide-screen modes with Keep Aspect Ration on) --------------------

			const float kw = GetHeight() / (float)m_scalingBaseH;
			const int clipW = m_scalingBaseW*kw;
			const int mw = ( GetWidth() - clipW ) / 2;

			const float kh = GetWidth() / (float)m_scalingBaseW;
			const int clipH = m_scalingBaseH*kh;
			const int mh = ( GetHeight() - clipH ) / 2;

			al_set_clipping_rectangle( mw, mh, GetWidth() - 2*mw, GetHeight() - 2*mh );

			break;
		}
		case SCALING_FREE:
		{
			const int w = GetWidth();
			const int h = GetHeight();
			const float sW = w / m_scalingBaseW;
			const float sH = h / m_scalingBaseH;
			ALLEGRO_TRANSFORM t;
			al_identity_transform(&t);
			al_scale_transform( &t, sW, sH );

			al_use_transform( &t );
			break;
		}
		case SCALING_NONE:
		{
			ALLEGRO_TRANSFORM t;
			al_identity_transform(&t);
			al_use_transform( &t );	
			break;
		}
	}
}

int AllegroRender5::GetWidth() const
{
	return al_get_display_width( m_display );
}

int AllegroRender5::GetHeight() const
{
	return al_get_display_height( m_display );
}

void AllegroRender5::Flip()
{
	al_flip_display();
	al_clear_to_color( al_map_rgb( 0, 0, 0 ) );
}

ImageHandle AllegroRender5::LoadImage( const char *filename )
{
	return GetBitmapManager().LoadResource( filename );
}

ImageHandle AllegroRender5::CloneImage( const char *filename, bool noPreserve )
{
	ImageHandle h = GetBitmapManager().LoadResource( filename );
	if ( h == ImageHandle::INVALID )
		return h;

	IImage *pImg = GetImageByHandle( h );
	if ( !pImg )
		return ImageHandle();

	if ( !noPreserve )
		noPreserve = ( al_get_bitmap_flags( ((AllegroImage5*)pImg)->GetBitmap() ) & ALLEGRO_NO_PRESERVE_TEXTURE ) != 0;
	ImageHandle clone = CreateImage( pImg->GetWidth(), pImg->GetHeight(), noPreserve, (std::string("clone_of_") + filename).c_str() );
	if ( clone == ImageHandle::INVALID )
		return clone;

	IImage *pClone = GetImageByHandle( clone );
	if ( !pClone )
		return ImageHandle();

	ALLEGRO_BITMAP *pOld = al_get_target_bitmap();
	al_set_target_bitmap( ((AllegroImage5*)pClone)->GetBitmap() );
	al_clear_to_color( al_map_rgba( 0, 0, 0, 0 ) );
	al_draw_bitmap( ((AllegroImage5*)pImg)->GetBitmap(), 0, 0, 0 );
	al_set_target_bitmap( pOld );
	return clone;
}

ImageHandle AllegroRender5::CloneImage( const ImageHandle & h, bool noPreserve )
{
	if ( h == ImageHandle::INVALID )
		return h;

	IImage *pImg = GetImageByHandle( h );
	if ( !pImg )
		return ImageHandle();

	if ( !noPreserve )
		noPreserve = ( al_get_bitmap_flags( ((AllegroImage5*)pImg)->GetBitmap() ) & ALLEGRO_NO_PRESERVE_TEXTURE ) != 0;
	ImageHandle clone = CreateImage( pImg->GetWidth(), pImg->GetHeight(), noPreserve, (std::string("clone")).c_str() );
	if ( clone == ImageHandle::INVALID )
		return clone;

	IImage *pClone = GetImageByHandle( clone );
	if ( !pClone )
		return ImageHandle();

	ALLEGRO_BITMAP *pOld = al_get_target_bitmap();
	al_set_target_bitmap( ((AllegroImage5*)pClone)->GetBitmap() );
	al_clear_to_color( al_map_rgba( 0, 0, 0, 0 ) );
	al_draw_bitmap( ((AllegroImage5*)pImg)->GetBitmap(), 0, 0, 0 );
	al_set_target_bitmap( pOld );
	return clone;
}

ImageHandle AllegroRender5::CreateImage( int w, int h, bool noPreserve, const char *context, bool memory )
{
	BitmapParams params;
	params.w = w;
	params.h = h;
	params.noPreserve = noPreserve;
	params.context = context ? context : "";
	params.memory = memory;
	return GetBitmapManager().CreateResource( params );
}

IImage *AllegroRender5::GetImageByHandle( const ImageHandle & h )
{
	return GetImage( h );
}

bool AllegroRender5::SaveImage( const char *filename, ImageHandle image )
{
	return al_save_bitmap( filename, GetImage(image)->GetBitmap() );
}

void AllegroRender5::DestroyImage( ImageHandle image )
{
	GetBitmapManager().UnloadResource( image );
}

IFontPtr AllegroRender5::CreateFont( const char *fontName )
{
#ifndef ALLEGRO_ANDROID
	if ( !al_filename_exists( fontName ) )
	{
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroRender5: Failed to load font ( %s )", 
			fontName );
		return 0;
	}
#else
	ALLEGRO_FILE *test = al_fopen( fontName, "r" );
	if ( !test )
	{
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroRender5: Failed to load font ( %s )", 
			fontName );
		return 0;
	}
	al_fclose( test );
#endif
	
	m_fonts.push_back( new AllegroFont5( fontName ) );
	return m_fonts.back();
}

void AllegroRender5::DestroyFont( IFontPtr font )
{
	for ( std::vector< AllegroFont5* >::iterator iter = m_fonts.begin();
		  iter != m_fonts.end();
		  ++iter)
	{
		if ( *iter == font )
		{
			m_fonts.erase( iter );
			break;
		}
	}

	delete font;
}

void AllegroRender5::SetFont( IFontPtr font )
{
	if ( font )
		m_currentFont = font;
	else
		m_currentFont = m_defaultFont;
}

void AllegroRender5::SetDefaultFont()
{
	SetFont( 0 );
}

Color AllegroRender5::MakeColor( int r, int g, int b, int a )
{
	Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;

	return c;
}

void AllegroRender5::DrawImage( ImageHandle image, int x, int y, int sx, int sy, int w, int h, float angle, int flags )
{
	AllegroImage5 *alImg = GetImage( image );

	ALLEGRO_COLOR tint = al_map_rgb(255,255,255);
	al_draw_tinted_scaled_rotated_bitmap_region( alImg->GetBitmap(), sx, sy, w, h, tint, w/2, h/2, x, y, 1.0f, 1.0f, angle, flags );
}

void AllegroRender5::DrawImage( ImageHandle image, int x, int y, int flags )
{
	AllegroImage5 *alImg = GetImage( image );
	if ( alImg )
		al_draw_bitmap( alImg->GetBitmap(), x, y, flags );
}

void AllegroRender5::CopyRegion( ImageHandle to, ImageHandle from, int toX, int toY, int fromX, int fromY, int fromW, int fromH, int toW, int toH )
{
	AllegroImage5 *alImgTo = GetImage( to );
	AllegroImage5 *alImgFrom = GetImage( from );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	if ( fromW < 0 )
		fromW = alImgFrom->GetWidth();
	if ( fromH < 0 )
		fromH = alImgFrom->GetHeight();

	al_set_target_bitmap( alImgTo->GetBitmap() );
	al_draw_scaled_bitmap( alImgFrom->GetBitmap(), fromX, fromY, fromW, fromH, toX, toY, toW, toH, 0 );
	al_set_target_bitmap( old );
}

void AllegroRender5::CopyRegion( ImageHandle from, int toX, int toY, int fromX, int fromY, int fromW, int fromH, int toW, int toH )
{
	AllegroImage5 *alImgFrom = GetImage( from );
	if ( fromW < 0 )
		fromW = alImgFrom->GetWidth();
	if ( fromH < 0 )
		fromH = alImgFrom->GetHeight();

	al_draw_scaled_bitmap( alImgFrom->GetBitmap(), fromX, fromY, fromW, fromH, toX, toY, toW, toH, 0 );
}

void AllegroRender5::DrawImage( ImageHandle target, ImageHandle image, int x, int y )
{
	AllegroImage5 *alTrgImg = GetImage( target );
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alTrgImg->GetBitmap() );
	al_draw_bitmap( alImg->GetBitmap(), x, y, 0 );
	al_set_target_bitmap( old );
}

/*
	Note, that DrawTintedImage may be way less useful than you would expect it to be.
	It just adds tint color to image. Nothing fancy!
*/
void AllegroRender5::DrawTintedImage( ImageHandle target, ImageHandle image, const Color & tint, int x, int y, int flags )
{
	AllegroImage5 *alTrgImg = GetImage( target );
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alTrgImg->GetBitmap() );
	al_draw_tinted_bitmap( alImg->GetBitmap(),  al_map_rgba( tint.r, tint.g, tint.b, tint.a ), x, y, flags );
	al_set_target_bitmap( old );
}

void AllegroRender5::DrawTintedImage( ImageHandle target, ImageHandle image, const Color & tint, int x, int y, int sx, int sy, int w, int h, int flags )
{
	AllegroImage5 *alTrgImg = GetImage( target );
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alTrgImg->GetBitmap() );
	al_draw_tinted_bitmap_region( alImg->GetBitmap(),  al_map_rgba( tint.r, tint.g, tint.b, tint.a ), sx, sy, w, h, x, y, flags );
	al_set_target_bitmap( old );
}

void AllegroRender5::DrawTintedImage( ImageHandle image, const Color & tint, int x, int y, int sx, int sy, int w, int h, int flags )
{
	AllegroImage5 *alImg = GetImage( image );
	al_draw_tinted_bitmap_region( alImg->GetBitmap(),  al_map_rgba( tint.r, tint.g, tint.b, tint.a ), sx, sy, w, h, x, y, flags );
}

void AllegroRender5::DrawTintedImage( ImageHandle image, const Color & tint, int x, int y, int flags )
{
	AllegroImage5 *alImg = GetImage( image );
	al_draw_tinted_bitmap( alImg->GetBitmap(),  al_map_rgba( tint.r, tint.g, tint.b, tint.a ), x, y, flags );
}

void AllegroRender5::DrawTintedImage( ImageHandle image, const Color & tint, int x, int y, int cx, int cy, float angle, int sx, int sy, int w, int h, int flags )
{
	AllegroImage5 *alImg = GetImage( image );
	const float scaleX = w / (float)alImg->GetWidth();
	const float scaleY = h / (float)alImg->GetHeight();
	al_draw_tinted_scaled_rotated_bitmap( alImg->GetBitmap(),  al_map_rgba( tint.r, tint.g, tint.b, tint.a ), cx, cy, x, y, scaleX, scaleY, angle, flags );
}

void AllegroRender5::DrawImage( ImageHandle image, int x, int y, float angle )
{
	AllegroImage5 *alImg = GetImage( image );
	al_draw_rotated_bitmap( alImg->GetBitmap(), alImg->GetWidth()/2, alImg->GetHeight()/2, x, y, angle, 0 );
}

void AllegroRender5::DrawImage( ImageHandle image, int x, int y, int px, int py, float angle )
{
	AllegroImage5 *alImg = GetImage( image );
	al_draw_rotated_bitmap( alImg->GetBitmap(), px, py, x, y, angle, 0 );
}

void AllegroRender5::StretchImage( ImageHandle image, int x, int y, float scale )
{
	AllegroImage5 *alImg = GetImage( image );
	al_draw_scaled_bitmap( alImg->GetBitmap(), 0, 0, alImg->GetWidth(), alImg->GetHeight(), x, y, alImg->GetWidth()*scale, alImg->GetHeight()*scale, 0 );
}

void AllegroRender5::StretchImage( ImageHandle image, int x, int y, float scaleX, float scaleY, int flags )
{
	int alFlags = 0;
	if ( flags & DRAW_BITMAP_FLIP_V )
		alFlags |= ALLEGRO_FLIP_VERTICAL;
	if ( flags & DRAW_BITMAP_FLIP_H )
		alFlags |= ALLEGRO_FLIP_HORIZONTAL;

	AllegroImage5 *alImg = GetImage( image );
	al_draw_scaled_bitmap( alImg->GetBitmap(), 0, 0, alImg->GetWidth(), alImg->GetHeight(), x, y, alImg->GetWidth()*scaleX, alImg->GetHeight()*scaleY, alFlags );
}

void AllegroRender5::ClearToColor( const Color & color )
{
	al_clear_to_color( al_map_rgba( color.r, color.g, color.b, color.a ) );
}

void AllegroRender5::ClearImage( ImageHandle image, const Color & color )
{
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	al_clear_to_color( al_map_rgba( color.r, color.g, color.b, color.a ) );
	al_set_target_bitmap( old );
}

void AllegroRender5::DrawLine( int x1, int y1, int x2, int y2, const Color & color, float thikness )
{
	al_draw_line( x1, y1, x2, y2, al_map_rgba( color.r, color.g, color.b, color.a ), thikness );
}

void AllegroRender5::DrawLine( ImageHandle image, int x1, int y1, int x2, int y2, const Color & color, float thikness )
{
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	DrawLine( x1, y1, x2, y2, color, thikness );
	al_set_target_bitmap( old );
}

void AllegroRender5::DrawRect( int x1, int y1, int x2, int y2, const Color & color )
{
	al_draw_rectangle( x1, y1, x2, y2, al_map_rgba( color.r, color.g, color.b, color.a ), 1 );
}

void AllegroRender5::DrawRect( ImageHandle image, int x1, int y1, int x2, int y2, const Color & color )
{
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	DrawRect( x1, y1, x2, y2, color );
	al_set_target_bitmap( old );
}

void AllegroRender5::DrawRectFill( int x1, int y1, int x2, int y2, const Color & color )
{
	al_draw_filled_rectangle( x1, y1, x2, y2, al_map_rgba( color.r, color.g, color.b, color.a ) );
}

void AllegroRender5::DrawRectFill( ImageHandle image, int x1, int y1, int x2, int y2, const Color & color )
{
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	DrawRectFill( x1, y1, x2, y2, color );
	al_set_target_bitmap( old );
}

void AllegroRender5::DrawCircle( int x, int y, int radius, const Color & color )
{
	al_draw_circle( x, y, radius, al_map_rgba( color.r, color.g, color.b, color.a ), 1 );
}

void AllegroRender5::DrawCircle( ImageHandle image, int x, int y, int radius, const Color & color )
{
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	DrawCircle( x, y, radius, color );
	al_set_target_bitmap( old );
}

void AllegroRender5::DrawCircleFill( int x, int y, int radius, const Color & color )
{
	al_draw_filled_circle( x, y, radius, al_map_rgba( color.r, color.g, color.b, color.a ) );
}

void AllegroRender5::DrawCircleFill( ImageHandle image, int x, int y, int radius, const Color & color )
{
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	DrawCircleFill( x, y, radius, color );
	al_set_target_bitmap( old );
}

Color AllegroRender5::GetPixel( ImageHandle image, int x, int y )
{	
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_COLOR c = al_get_pixel( alImg->GetBitmap(), x, y );
	Color ret;
	unsigned char r, g, b, a;
	al_unmap_rgba( c, &r, &g, &b, &a );
	ret.r = r;
	ret.g = g;
	ret.b = b;
	ret.a = a;
	return ret;
}

void AllegroRender5::DrawPixel( int x, int y, const Color & color, bool blend )
{
	if ( blend )
		al_put_blended_pixel( x, y, al_map_rgba( color.r, color.g, color.b, color.a ) );
	else
		al_put_pixel( x, y, al_map_rgba( color.r, color.g, color.b, color.a ) );
}

void AllegroRender5::DrawPixel( ImageHandle image, int x, int y, const Color & color, bool blend )
{
	AllegroImage5 *alImg = GetImage( image );
	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	DrawPixel( x, y, color );
	al_set_target_bitmap( old );
}

/*
	OK, now. Text functions could be handled WAY better. I promise I'm going to refactor
	this for my next game.
*/

void AllegroRender5::DrawText( int x, int y, const Color & color, int size, const char *fmt, ... )
{
	va_list args;
	va_list argsCopy;
	va_start(args, fmt);	
#ifdef WIN32
	argsCopy = args;
	int bufLen = _vscprintf(fmt, argsCopy) + 1;
#else
	va_copy( argsCopy, args );
	int bufLen = vsnprintf(0,0,fmt,argsCopy)+1;
#endif	

	if ( bufLen <= 0 )
	    return;

	if ((int)m_buffer.size()	<= bufLen)
	    m_buffer.resize(bufLen);

	vsprintf(&m_buffer[0], fmt, args);
	
	ALLEGRO_COLOR c = al_map_rgba( color.r, color.g, color.b, color.a );
	al_draw_text( ((AllegroFont5*)m_currentFont)->GetFont( size ), c, x, y, 0, &m_buffer[0] );

	va_end( args );
}

void AllegroRender5::DrawText( int x, int y, const Color & color, int size, const UTFString & str, int offset, int length )
{
	const ALLEGRO_USTR *ustr = ((AllegroUTFString5*)(IUTFString*)str)->GetString();
	if ( length < 0 )
		length = al_ustr_length( ustr );

	const int start = al_ustr_offset( ustr, offset );
	const int end = al_ustr_offset( ustr, offset + length );

	ALLEGRO_USTR_INFO info;
	const ALLEGRO_USTR *subString = al_ref_ustr( &info, ustr, start, end );

	ALLEGRO_COLOR c = al_map_rgba( color.r, color.g, color.b, color.a );
	al_draw_ustr( ((AllegroFont5*)m_currentFont)->GetFont( size ), c, x, y, 0, subString );
}

void AllegroRender5::DrawText( ImageHandle image, int x, int y, const Color & color, int size, const char *fmt, ... )
{
	AllegroImage5 *alImg = GetImage( image );

	va_list args;
	va_list argsCopy;
	va_start(args, fmt);
#ifdef WIN32
	argsCopy = args;
	int bufLen = _vscprintf(fmt, argsCopy) + 1;
#else
	va_copy( argsCopy, args );
	int bufLen = vsnprintf(0,0,fmt,argsCopy)+1;
#endif	
	if ( bufLen <= 0 )
	    return;

	if ((int)m_buffer.size()	<= bufLen)
	    m_buffer.resize(bufLen);

	vsprintf(&m_buffer[0], fmt, args);

	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	ALLEGRO_COLOR c = al_map_rgba( color.r, color.g, color.b, color.a );
	al_draw_text( ((AllegroFont5*)m_currentFont)->GetFont( size ), c, x, y, 0, &m_buffer[0] );
	al_set_target_bitmap( old );

	va_end( args );
}

void AllegroRender5::DrawText( ImageHandle image, int x, int y, const Color & color, int size, const UTFString & str, int offset, int length )
{
	AllegroImage5 *alImg = GetImage( image );

	const ALLEGRO_USTR *ustr = ((AllegroUTFString5*)(IUTFString*)str)->GetString();
	if ( length < 0 )
		length = al_ustr_length( ustr );

	const int start = al_ustr_offset( ustr, offset );
	const int end = al_ustr_offset( ustr, offset + length );

	ALLEGRO_USTR_INFO info;
	const ALLEGRO_USTR *subString = al_ref_ustr( &info, ustr, start, end );

	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	ALLEGRO_COLOR c = al_map_rgba( color.r, color.g, color.b, color.a );
	al_draw_ustr( ((AllegroFont5*)m_currentFont)->GetFont( size ), c, x, y, 0, subString );
	al_set_target_bitmap( old );
}

void AllegroRender5::DrawAlignedText( int x, int y, const Color & color, int size, ETextAlign align, const char *fmt, ... )
{
	va_list args;
	va_list argsCopy;
	va_start(args, fmt);
#ifdef WIN32
	argsCopy = args;
	int bufLen = _vscprintf(fmt, argsCopy) + 1;
#else
	va_copy( argsCopy, args );
	int bufLen = vsnprintf(0,0,fmt,argsCopy)+1;
#endif	
	if ( bufLen <= 0 )
	    return;

	if ((int)m_buffer.size()	<= bufLen)
	    m_buffer.resize(bufLen);

	vsprintf(&m_buffer[0], fmt, args);

	ALLEGRO_FONT *f = ((AllegroFont5*)m_currentFont)->GetFont( size );

	ALLEGRO_COLOR c = al_map_rgba( color.r, color.g, color.b, color.a );

	if ( align == TEXT_ALIGN_LEFT )
		al_draw_text( f, c, x, y, ALLEGRO_ALIGN_LEFT, &m_buffer[0] );
	else if ( align == TEXT_ALIGN_RIGHT )
		al_draw_text( f, c, x, y, ALLEGRO_ALIGN_RIGHT, &m_buffer[0] );
	else if ( align == TEXT_ALIGN_CENTER )
		al_draw_text( f, c, x, y, ALLEGRO_ALIGN_CENTRE, &m_buffer[0] );

	va_end( args );
}

void AllegroRender5::DrawAlignedText( ImageHandle image, int x, int y, const Color & color, int size, ETextAlign align, const char *fmt, ... )
{
	AllegroImage5 *alImg = GetImage( image );
	if ( !alImg )
		return;

	va_list args;
	va_list argsCopy;
	va_start(args, fmt);
#ifdef WIN32
	argsCopy = args;
	int bufLen = _vscprintf(fmt, argsCopy) + 1;
#else
	va_copy( argsCopy, args );
	int bufLen = vsnprintf(0,0,fmt,argsCopy)+1;
#endif	
	if ( bufLen <= 0 )
	    return;

	if ((int)m_buffer.size()	<= bufLen)
	    m_buffer.resize(bufLen);

	vsprintf(&m_buffer[0], fmt, args);
	
	ALLEGRO_FONT *f = ((AllegroFont5*)m_currentFont)->GetFont( size );

	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	ALLEGRO_COLOR c = al_map_rgba( color.r, color.g, color.b, color.a );
	if ( align == TEXT_ALIGN_LEFT )
		al_draw_text( f, c, x, y, ALLEGRO_ALIGN_LEFT, &m_buffer[0] );
	else if ( align == TEXT_ALIGN_RIGHT )
		al_draw_text( f, c, x, y, ALLEGRO_ALIGN_RIGHT, &m_buffer[0] );
	else if ( align == TEXT_ALIGN_CENTER )
		al_draw_text( f, c, x, y, ALLEGRO_ALIGN_CENTRE, &m_buffer[0] );
	al_set_target_bitmap( old );

	va_end( args );
}

void AllegroRender5::DrawTextInArea( int x, int y, int w, int h, const Color & color, int size, const char *fmt, ... )
{
	va_list args;
	va_list argsCopy;
	va_start(args, fmt);
#ifdef WIN32
	argsCopy = args;
	int bufLen = _vscprintf(fmt, argsCopy) + 1;
#else
	va_copy( argsCopy, args );
	int bufLen = vsnprintf(0,0,fmt,argsCopy)+1;
#endif	
	if ( bufLen <= 0 )
	    return;

	if ((int)m_buffer.size()	<= bufLen)
	    m_buffer.resize(bufLen);

	vsprintf(&m_buffer[0], fmt, args);

	DrawTextInAreaInternal( x, y, w, h, color, size );

	va_end( args );
}

void AllegroRender5::DrawTextInArea( int x, int y, int w, int h, const Color & color, int size, const UTFString & text )
{
	const ALLEGRO_USTR *ustr = ((AllegroUTFString5*)(IUTFString*)text)->GetString();
	
	/*
		Here's the basic idea: read characters from string until we encounter space.
		This is word. If current string + new word still fits into allowed width - print it,
		otherwise break line.
	*/
	ALLEGRO_COLOR c = al_map_rgba( color.r, color.g, color.b, color.a );

	ALLEGRO_FONT *f = ((AllegroFont5*)m_currentFont)->GetFont( size );
	int wordStart = 0;
	int wordLength = 0;
	int pos = 0;
	int cx = 0;
	int cy = 0;

	const int lineH = al_get_font_line_height( f );

	const int textLen = al_ustr_size( ustr );

	while( true )
	{
		bool found0 = false;

		int prevPos = pos;
		char ch = al_ustr_get_next( ustr, &pos );
		if ( pos == prevPos )
			ch = 0;

		switch( ch )
		{
			case 0:
				found0 = true;
			case ' ':
				ALLEGRO_USTR_INFO info;
				const ALLEGRO_USTR *word = al_ref_ustr( &info, ustr, wordStart, pos );
				const int wordWidth = al_get_ustr_width( f, word );
				bool nextLine = false;

				if( cx + wordWidth >= w )
				{
					cy += lineH;
					cx = 0;
					nextLine = true;
				}
				if ( cy + lineH >= h )
					return;

				al_draw_ustr( f, c, x + cx, y + cy, 0, word );

				cx += wordWidth;

				wordStart = pos;
				break;
		}

		if ( found0 )
			break;
	}
}

void AllegroRender5::DrawTextInArea( ImageHandle image, int x, int y, int w, int h, const Color & color, int size, const char *fmt, ... )
{
	AllegroImage5 *alImg = GetImage( image );
	if ( !alImg )
		return;

	va_list args;
	va_list argsCopy;
	va_start(args, fmt);
#ifdef WIN32
	argsCopy = args;
	int bufLen = _vscprintf(fmt, argsCopy) + 1;
#else
	va_copy( argsCopy, args );
	int bufLen = vsnprintf(0,0,fmt,argsCopy)+1;
#endif		

	if ( bufLen <= 0 )
	    return;
	if ((int)m_buffer.size() <= bufLen)
	    m_buffer.resize(bufLen);

	vsprintf(&m_buffer[0], fmt, args);

	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	DrawTextInAreaInternal( x, y, w, h, color, size );
	al_set_target_bitmap( old );

	va_end( args );
}

void AllegroRender5::DrawTextInAreaInternal( int x, int y, int w, int h, const Color & color, int size )
{
	/*
		Here's the basic idea: read characters from string until we encounter space.
		This is word. If current string + new word still fits into allowed width - print it,
		otherwise break line.
	*/
	ALLEGRO_COLOR c = al_map_rgba( color.r, color.g, color.b, color.a );

	ALLEGRO_FONT *f = ((AllegroFont5*)m_currentFont)->GetFont( size );
	int wordStart = 0;
	int wordLength = 0;
	int pos = 0;
	int cx = 0;
	int cy = 0;

	const int lineH = al_get_font_line_height( f );

	while( pos < (int)m_buffer.size() )
	{
		const char ch = m_buffer[ pos ];
		
		bool found0 = false;

		switch( ch )
		{
			case 0:
				found0 = true;
			case ' ':
				  // I didn't want to allocate a new buffer for each word, so here's a stupid trick:
				  // We back up the next character and write \0 instead of it, so now C and Allegro
				  // would treat it as the end of string. Then we do our dirty business and replace
				  // the character. No harm done and WAY less memory allocations! Still not very
				  // clever.
				char backUp = 0;
				if ( !found0 )
				{
					backUp = m_buffer[ pos + 1 ];
					m_buffer[ pos + 1 ] = 0;
				}

				const char *word = &m_buffer[ wordStart ];
				const int wordWidth = al_get_text_width( f, word );
				bool nextLine = false;

				if( cx + wordWidth >= w )
				{
					cy += lineH;
					cx = 0;
					nextLine = true;
				}
				if ( cy + lineH >= h )
					return;

				al_draw_textf( f, c, x + cx, y + cy, 0, "%s", word );

				cx += wordWidth;

				if ( !found0 )
					m_buffer[ pos + 1 ] = backUp;

				wordStart = pos + 1;
				break;
		}

		if ( found0 )
			break;

		++pos;
	}
}

void AllegroRender5::Close()
{
	al_destroy_event_queue( m_eventsQueue );
	al_destroy_display( m_display );
}

void AllegroRender5::ShowMouseCursor()
{
	al_show_mouse_cursor( m_display );
	m_showingMouse = true;
}

void AllegroRender5::HideMouseCursor()
{
	al_hide_mouse_cursor( m_display );
	m_showingMouse = false;
}

void AllegroRender5::ToggleFullscreen()
{
	const int w = al_get_display_width( m_display );
	const int h = al_get_display_height( m_display );
	Reinit( m_renderType, !m_fullScreen, w, h, m_vsync );
}

void AllegroRender5::Reinit( ERenderType type, bool fullscreen, int w, int h, bool vsync )
{
	  // If nothing changed, do nothing
	if ( m_fullScreen == fullscreen && m_renderType == type && m_vsync == vsync )
	{
		if ( w == GetWidth() && h == GetHeight() )
			return;	
	}

	  // If only resolution changed
	if ( !fullscreen && m_fullScreen == fullscreen && m_renderType == type && m_vsync == vsync )
	{
		if ( !al_resize_display( m_display, w, h ) )
		{
			GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroRender5: Failed to resize display to ( %ix%i %s )", 
				w, h, m_fullScreen ? "fullscreen" : "windowed" );

			  // If all else fails, try a fallback
			if ( !al_resize_display( m_display, 640, 480 ) )
				exit(-1);
		}
	}
	else
	{
		m_fullScreen = fullscreen;
		m_renderType = type;
		m_vsync = vsync;
		
		al_set_new_display_option( ALLEGRO_VSYNC, vsync ? 1 : 2, ALLEGRO_SUGGEST );
		#ifdef WINDOWS
		    al_set_new_display_flags( ( m_fullScreen ? ALLEGRO_FULLSCREEN : ALLEGRO_WINDOWED ) | ( type == RENDERTYPE_DIRECT3D ? ALLEGRO_DIRECT3D : ALLEGRO_OPENGL ) );
		#else
		    al_set_new_display_flags( ( m_fullScreen ? ALLEGRO_FULLSCREEN : ALLEGRO_WINDOWED ) | ALLEGRO_OPENGL );
		#endif

		al_unregister_event_source( m_eventsQueue, al_get_display_event_source( m_display ) );
		al_destroy_display( m_display );
		m_display = al_create_display( w, h );
		if ( !m_display )
		{
			GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroRender5: Failed to reinit display to ( %ix%i %s )", 
				w, h, m_fullScreen ? "fullscreen" : "windowed" );
			exit(-1);
		}
		al_register_event_source( m_eventsQueue, al_get_display_event_source( m_display ) );
	}

	  // OpenGL needs this, DirectX somehow doesn't...
	GetBitmapManager().ReloadAllResources();

	for ( std::vector<AllegroFont5*>::iterator iter = m_fonts.begin();
		iter != m_fonts.end();
		++iter)
	{
		(*iter)->ReloadFont();
	}

	if ( !m_display )
	{
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroRender5: Failed to set graphic mode ( %ix%i %s )", 
			w, h, m_fullScreen ? "fullscreen" : "windowed" );

		exit(-1);
	}

	ReapplyScaling();
}

void AllegroRender5::DrawFilledQuad( const TexPoint & p1, const TexPoint & p2, const TexPoint & p3, const TexPoint & p4, const Color & color )
{
	ALLEGRO_COLOR c = al_map_rgba( color.r, color.g, color.b, color.a );

	ALLEGRO_VERTEX v[4];
	v[0].x = p1.x; v[0].y = p1.y; v[0].z = 0;
	v[0].color = c;
	v[0].u = p1.u; v[0].v = p1.v;
	v[1].x = p2.x; v[1].y = p2.y; v[1].z = 0;
	v[1].u = p2.u; v[1].v = p2.v;
	v[1].color = c;
	v[2].x = p3.x; v[2].y = p3.y; v[2].z = 0;
	v[2].u = p3.u; v[2].v = p3.v;	
	v[2].color = c;
	v[3].x = p4.x; v[3].y = p4.y; v[3].z = 0;
	v[3].u = p4.u; v[3].v = p4.v;
	v[3].color = c;

	al_draw_prim( &v[0], 0, 0, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN );
}

void AllegroRender5::DrawColoredQuad( ImageHandle image, const ColorPoint & p1, const ColorPoint & p2, const ColorPoint & p3, const ColorPoint & p4 )
{
	AllegroImage5 *alImg = GetImage( image );

	ALLEGRO_VERTEX v[4];
	v[0].x = p1.x; v[0].y = p1.y; v[0].z = 0;
	v[0].color = al_map_rgba( p1.c.r, p1.c.g, p1.c.b, p1.c.a );
	//v[0].u = p1.u; v[0].v = p1.v;
	v[1].x = p2.x; v[1].y = p2.y; v[1].z = 0;
	v[1].color = al_map_rgba( p2.c.r, p2.c.g, p2.c.b, p2.c.a );
	//v[1].u = p2.u; v[1].v = p2.v;	
	v[2].x = p3.x; v[2].y = p3.y; v[2].z = 0;
	v[2].color = al_map_rgba( p3.c.r, p3.c.g, p3.c.b, p3.c.a );
	//v[2].u = p3.u; v[2].v = p3.v;		
	v[3].x = p4.x; v[3].y = p4.y; v[3].z = 0;
	v[3].color = al_map_rgba( p4.c.r, p4.c.g, p4.c.b, p4.c.a );
	//v[3].u = p4.u; v[3].v = p4.v;	

	ALLEGRO_BITMAP *old = al_get_target_bitmap();
	al_set_target_bitmap( alImg->GetBitmap() );
	al_draw_prim( &v[0], 0, 0, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN );
	al_set_target_bitmap( old );
}

void AllegroRender5::DrawColoredQuad( const ColorPoint & p1, const ColorPoint & p2, const ColorPoint & p3, const ColorPoint & p4 )
{
	ALLEGRO_VERTEX v[4];
	v[0].x = p1.x; v[0].y = p1.y; v[0].z = 0;
	v[0].color = al_map_rgba( p1.c.r, p1.c.g, p1.c.b, p1.c.a );
	
	v[1].x = p2.x; v[1].y = p2.y; v[1].z = 0;
	v[1].color = al_map_rgba( p2.c.r, p2.c.g, p2.c.b, p2.c.a );
	
	v[2].x = p3.x; v[2].y = p3.y; v[2].z = 0;
	v[2].color = al_map_rgba( p3.c.r, p3.c.g, p3.c.b, p3.c.a );
	
	v[3].x = p4.x; v[3].y = p4.y; v[3].z = 0;
	v[3].color = al_map_rgba( p4.c.r, p4.c.g, p4.c.b, p4.c.a );	

	al_draw_prim( &v[0], 0, 0, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN );
}

void AllegroRender5::DrawTexturedQuad( ImageHandle texture, const TexPoint & p1, const TexPoint & p2, const TexPoint & p3, const TexPoint & p4 )
{
	AllegroImage5 *alImg = GetImage( texture );
	if ( !alImg )
		return;

	ALLEGRO_COLOR c = al_map_rgba( 255,255,255,255 );

	ALLEGRO_VERTEX v[4];
	v[0].x = p1.x; v[0].y = p1.y; v[0].z = 0;
	v[0].u = p1.u; v[0].v = p1.v;
	v[0].color = c;
	v[1].x = p2.x; v[1].y = p2.y; v[1].z = 0;
	v[1].u = p2.u; v[1].v = p2.v;
	v[1].color = c;
	v[2].x = p3.x; v[2].y = p3.y; v[2].z = 0;
	v[2].u = p3.u; v[2].v = p3.v;	
	v[2].color = c;
	v[3].x = p4.x; v[3].y = p4.y; v[3].z = 0;
	v[3].u = p4.u; v[3].v = p4.v;
	v[3].color = c;
	
	al_draw_prim( &v[0], 0, alImg->GetBitmap(), 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN );
}

void AllegroRender5::SetFontSize( int pixels )
{
	m_fontSize = pixels;
}

void AllegroRender5::BeginBitmapBatch( ImageHandle targetImage )
{
	AllegroImage5 *pImg = GetImage( targetImage );
	if ( !pImg )
		return;

	if ( targetImage.IsValid() )
	{
		m_pBatchPrevTarget = al_get_target_bitmap();
		al_set_target_bitmap( pImg->GetBitmap() );
	}
	al_hold_bitmap_drawing( true );
}

void AllegroRender5::EndBitmapBatch()
{
	al_hold_bitmap_drawing( false );

	if ( m_pBatchPrevTarget )
		al_set_target_bitmap( m_pBatchPrevTarget );

	m_pBatchPrevTarget = 0;
}

int AllegroRender5::GetFontSize() const
{
	return m_fontSize;
}

int AllegroRender5::GetCharWidth( const char c, int fontSize ) const
{
	char str[2] = {c,0};
	return GetStringWidth( str, fontSize );
}

int AllegroRender5::GetCharWidth( const wchar_t c, int fontSize ) const
{
	wchar_t str[2] = {c,L'\0'};
	return GetStringWidth( &str[0], 2, fontSize );
}

int AllegroRender5::GetStringWidth( const char *str, int fontSize ) const
{
	if ( fontSize < 0 )
		fontSize = m_fontSize;

	return al_get_text_width( ((AllegroFont5*)m_currentFont)->GetFont( fontSize ), str );
}

int AllegroRender5::GetStringWidth( const wchar_t *str, int length, int fontSize ) const
{
	if ( fontSize < 0 )
		fontSize = m_fontSize;

	ALLEGRO_FONT *f = ((AllegroFont5*)m_currentFont)->GetFont( fontSize );
	if ( !f )
		return 0;

	static char fmt[16];
	if ( length > 0 )
		sprintf( fmt, "%%.%iS", length );
	else
		sprintf( fmt, "%%S" );

	  // Very bad idea. Still don't know how to do it better.
	ALLEGRO_USTR *ustr = al_ustr_newf( fmt, str );
	const int wordWidth = al_get_ustr_width( f, ustr );
	
	al_ustr_free( ustr );

	return wordWidth;
}

int AllegroRender5::GetStringWidth( const UTFString & str, int offset, int length, int fontSize ) const
{
	if ( fontSize < 0 )
		fontSize = m_fontSize;

	ALLEGRO_FONT *f = ((AllegroFont5*)m_currentFont)->GetFont( fontSize );
	if ( !f )
		return 0;

	const ALLEGRO_USTR *ustr = ((AllegroUTFString5*)(IUTFString*)str)->GetString();

	const int start = al_ustr_offset( ustr, offset );
	const int end = al_ustr_offset( ustr, offset+length );

	ALLEGRO_USTR_INFO info;
	const ALLEGRO_USTR *tmp = al_ref_ustr( &info, ustr, start, end );

	const int wordWidth = al_get_ustr_width( f, tmp );
	
	return wordWidth;
}

void AllegroRender5::MeasureText( const char *str, int &bbx, int &bby, int &bbw, int &bbh ) const
{
	return al_get_text_dimensions( ((AllegroFont5*)m_currentFont)->GetFont( m_fontSize ), str, &bbx, &bby, &bbw, &bbh );
}

AllegroImage5::~AllegroImage5()
{
	if ( !m_system )
		al_destroy_bitmap( m_bitmap );
}

ALLEGRO_FONT* AllegroFont5::GetFont(int size)
{
	std::map< int, ALLEGRO_FONT * >::iterator iter = m_fonts.find( size );
	if ( iter != m_fonts.end() )
		return iter->second;

	ALLEGRO_FONT *font = al_load_font( m_filename.c_str(), size, 0 );
	if ( !font )
		return 0;

	m_fonts.insert( std::make_pair( size, font ) );
	return font;
}

void AllegroFont5::ReloadFont()
{
	for ( std::map< int, ALLEGRO_FONT* >::iterator iter = m_fonts.begin();
		  iter != m_fonts.end();
		  ++iter)
	{
		al_destroy_font( iter->second );
		iter->second = al_load_font( m_filename.c_str(), iter->first, 0 );
	}
}

AllegroFont5::~AllegroFont5()
{
	for ( std::map< int, ALLEGRO_FONT * >::iterator iter = m_fonts.begin();
		  iter != m_fonts.end();
		  ++iter)
		  al_destroy_font( iter->second );
}

void AllegroRender5::LockImage( ImageHandle image )
{
	AllegroImage5 *alImg = GetImage( image );
	if ( !alImg )
		return;

	al_lock_bitmap( alImg->GetBitmap(), ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE );
}

void AllegroRender5::LockImage( ImageHandle image, int x1, int y1, int x2, int y2 )
{
	AllegroImage5 *alImg = GetImage( image );
	if ( !alImg )
		return;
    
    int w = x2 - x1;
    int h = y2 - y1;
    if ( x1 < 0 ) x1 = 0;
    if ( y1 < 0 ) y1 = 0;
    if ( w < 0 ) w = 0;
    if ( x1 + w > alImg->GetWidth() ) w = alImg->GetWidth() - x1;
    if ( y1 + h > alImg->GetHeight() ) h = alImg->GetHeight() - y1;
	al_lock_bitmap_region( alImg->GetBitmap(), x1, y1, w, h, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE );
}

void AllegroRender5::UnlockImage( ImageHandle image )
{
	AllegroImage5 *alImg = GetImage( image );
	if ( !alImg )
		return;

	al_unlock_bitmap( alImg->GetBitmap() );
}

void AllegroRender5::EnumerateDisplayModes( IEnumerateDisplayModesCallback & callback ) const
{
#if defined(ALLEGRO_ANDROID) || defined(ALLEGRO_IPHONE)
	return;
#endif
	const int modesCount = al_get_num_display_modes();
	for ( int i = 0; i < modesCount; ++i )
	{
		ALLEGRO_DISPLAY_MODE mode;
		al_get_display_mode( i, &mode );

		int bits = 0;

		  // Oh, come on, WHY doesn't DISPLAY_MODE contain bit depth?!
		switch( mode.format )
		{
			case ALLEGRO_PIXEL_FORMAT_ARGB_8888:
			case ALLEGRO_PIXEL_FORMAT_RGBA_8888:
			case ALLEGRO_PIXEL_FORMAT_ABGR_8888:
			case ALLEGRO_PIXEL_FORMAT_XBGR_8888:
			case ALLEGRO_PIXEL_FORMAT_RGBX_8888:
			case ALLEGRO_PIXEL_FORMAT_XRGB_8888:
			case ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE:
				bits = 32;
				break;
			case ALLEGRO_PIXEL_FORMAT_ARGB_4444:
			case ALLEGRO_PIXEL_FORMAT_RGBA_5551:
			case ALLEGRO_PIXEL_FORMAT_ARGB_1555:
			case ALLEGRO_PIXEL_FORMAT_BGR_565:
			case ALLEGRO_PIXEL_FORMAT_RGB_565:
			case ALLEGRO_PIXEL_FORMAT_RGBA_4444:
				bits = 16;
				break;
			case ALLEGRO_PIXEL_FORMAT_RGB_555:
			case ALLEGRO_PIXEL_FORMAT_BGR_555:
				bits = 15;
				break;
			case ALLEGRO_PIXEL_FORMAT_RGB_888:
			case ALLEGRO_PIXEL_FORMAT_BGR_888:
				bits = 24;
				break;
			case ALLEGRO_PIXEL_FORMAT_ABGR_F32:
				bits = 32;
				break;
		}	

		callback.EnumerateDisplayMode( bits, mode.width, mode.height, mode.refresh_rate );
	}
}

void AllegroRender5::Clear()
{
	GetBitmapManager().Clear();
	for ( size_t i = 0; i < m_fonts.size(); ++i )
		delete m_fonts[i];
}

void AllegroRender5::ProcessEvents()
{
	m_events.clear();

	ALLEGRO_EVENT event;
	while( al_get_next_event( m_eventsQueue, &event ) )
	{
		DisplayEvent e;

		switch( event.type )
		{
			case ALLEGRO_EVENT_DISPLAY_EXPOSE:
				e.m_type = DisplayEvent::DISPLAY_EXPOSE;
				e.m_x = event.display.x;
				e.m_y = event.display.y;
				e.m_w = event.display.width;
				e.m_h = event.display.height;
				break;

			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				e.m_type = DisplayEvent::DISPLAY_RESIZE;
				e.m_x = event.display.x;
				e.m_y = event.display.y;
				e.m_w = event.display.width;
				e.m_h = event.display.height;
				break;

			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				e.m_type = DisplayEvent::DISPLAY_CLOSE;
				break;

			case ALLEGRO_EVENT_DISPLAY_LOST:
				e.m_type = DisplayEvent::DISPLAY_LOST;
				break;

			case ALLEGRO_EVENT_DISPLAY_FOUND:
				e.m_type = DisplayEvent::DISPLAY_FOUND;
				break;

			case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
				e.m_type = DisplayEvent::DISPLAY_SWITCH_IN;
				break;

			case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
				e.m_type = DisplayEvent::DISPLAY_SWITCH_OUT;
				break;

			case ALLEGRO_EVENT_DISPLAY_ORIENTATION:
				e.m_type = DisplayEvent::DISPLAY_ORIENTATION;
				break;

			case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
				e.m_type = DisplayEvent::DISPLAY_HALT_DRAWING;
				break;

			case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
				e.m_type = DisplayEvent::DISPLAY_RESUME_DRAWING;
				break;

			default:
				continue;
		}

		m_events.push_back( e );
	}
}

DisplayEvent AllegroRender5::GetEvent()
{
	if ( m_events.empty() )
		return DisplayEvent();

	DisplayEvent e = m_events.front();
	m_events.erase( m_events.begin() );

	return e;
}

IRender & GetRender()
{
	static AllegroRender5 render;
	return render;
}
