#pragma once
#include "IRender.h"

/*
	WARNING! These classes are far from being good code or even easy to use code.
	They are mostly just a (bad?) wrapper for Allegro functions, because at some
	point in my coder's life I decided that calling library functions directly isn't
	a very good idea.

	Beside wrapping Allegro, there is some resources management here. Don't expect
	wonders from it. Expect bugs. Especially when switching resolutions/fullscreen
	modes.
*/

struct ALLEGRO_BITMAP;
struct ALLEGRO_DISPLAY;
struct ALLEGRO_FONT;

class AllegroImage5;

  /*
    Since Allegro treats each size of TTF font as
	a different bitmap font, this class takes care
	of stroing them all together.
  */
class AllegroFont5 : public IFont
{
	  // Font size -> ALLEGRO_FONT map
	std::map< int, ALLEGRO_FONT* > m_fonts;
	  // Base TTF font name
	std::string m_filename;
public:
	AllegroFont5( const std::string & filename )
		: m_filename( filename )
	{}	

	~AllegroFont5();

	ALLEGRO_FONT *GetFont( int size );
	void ReloadFont();
};

/*
	IRender implementation for Allegro5
*/
class AllegroRender5 : public IRender
{
	  // Current display
	ALLEGRO_DISPLAY* m_display;	  
	ALLEGRO_EVENT_QUEUE* m_eventsQueue;
	  // Queue of display events (resize, close button clicks etc.)
	std::vector<DisplayEvent> m_events;
	  // This is supposed to be used to hide/show mouse. I don't know if it works anymore
	bool m_showingMouse;
	bool m_fullScreen;
	  // Current render type (OpenGl/DirectX)
	ERenderType m_renderType;
	bool m_vsync;
	  // Current font size for operations which do not take font size as parameter (maybe obsolete?)
	int m_fontSize;
	IFontPtr m_currentFont;
	IFontPtr m_defaultFont;
	  // Buffer for text formatting. Here to avoid more allocations.
	std::vector< char > m_buffer;
      // All fonts loaded by game
	std::vector< AllegroFont5* > m_fonts;

	  // Scaling mode and parameters
	EScalingMode m_scalingMode;
	  // Base Width & Height of game screen
	float m_scalingBaseW;
	float m_scalingBaseH;

	  // Saved target bitmap for cases when we want to draw the same bitmap many times onto different bitmap
	ALLEGRO_BITMAP *m_pBatchPrevTarget;

	  // Returns image by provided handle
	AllegroImage5 *GetImage( ImageHandle handle );

public:
	AllegroRender5()
	{
		m_pBatchPrevTarget = 0;
		m_scalingMode = SCALING_NONE;
		m_scalingBaseW = 1.0f;
		m_scalingBaseH = 1.0f;
	}

	  // Initializes render
	bool Init( ERenderType type, int w, int h, int bpp, bool fullscreen, bool vsync, const char *defaultFont );
	  // Set scaling mode and parameters
	void SetGlobalScaling( EScalingMode mode, float baseW, float baseH );

private:
	  // Called internally after resolution/fullscreen mode changes
	void ReapplyScaling();

public:
	  // Returns width of current display
	int GetWidth() const;
	  // Returns height of current display
	int GetHeight() const;
	  // Draws backbuffer onto screen
	void Flip();
	  // Creates temporary image with specified parameters
	  // Specifiy noPreserve if you are going to update it
	  // manually every frame or so.
	  // Specify context for debugging purposes
	ImageHandle CreateImage( int w, int h, bool noPreserve, const char *context = 0, bool memory = false );
	  // Loads image from file
	ImageHandle LoadImage( const char *filename );
	  // Loads image from file into a new copy even if it is already loaded
	ImageHandle CloneImage( const char *filename, bool noPreserve = false );
	  // Creates a copy of provided image
	ImageHandle CloneImage( const ImageHandle & h, bool noPreserve = false );
	  // Returns an intefrace for image, from which you can query its dimensions
	IImage *GetImageByHandle( const ImageHandle & h );
	  // Saves image to disk
	bool SaveImage( const char *filename, ImageHandle image );
	void DestroyImage( ImageHandle image );
	  // Destroys all images. Needs to be called before Allegro de-initializes
	void Clear();

	  // Obsolete. Use Color( r,g,b,a )
	Color MakeColor( int r, int g, int b, int a = 255 );
	  
	  // Returns pixel color from specified image. Slow!
	Color GetPixel( ImageHandle, int x, int y );

	  // Copies region from one bitmap to another while stretching it
	void CopyRegion( ImageHandle to, ImageHandle from, int toX, int toY, int fromX, int fromY, int fromW, int fromH, int toW, int toH );
	  // Copies region from bitmap to backbuffer while stretching it
	void CopyRegion( ImageHandle from, int toX, int toY, int fromX, int fromY, int fromW, int fromH, int toW, int toH );

	  // Draws bitmap onto backbuffer
	void DrawImage( ImageHandle image, int x, int y, int flags = 0 );
	  // Draws bitmap onto another bitmap
	void DrawImage( ImageHandle target, ImageHandle image, int x, int y );
	  // Draws bitmap onto backbuffer with rotation
	void DrawImage( ImageHandle image, int x, int y, float angle );
	  // Draws bitmap onto backbuffer with rotation around specified point
	void DrawImage( ImageHandle image, int x, int y, int px, int py, float angle );
	  // Draws part of bitmap onto backbuffer with rotation
	void DrawImage( ImageHandle image, int x, int y, int sx, int sy, int w, int h, float angle = 0, int flags = 0 );

	  // Same things, but also adds tint color according to currently set blend parameters
	void DrawTintedImage( ImageHandle image, const Color & tint, int x, int y, int flags = 0 );
	void DrawTintedImage( ImageHandle target, ImageHandle image, const Color & tint, int x, int y, int flags = 0 );
	void DrawTintedImage( ImageHandle target, ImageHandle image, const Color & tint, int x, int y, int sx, int sy, int w, int h, int flags = 0 );
	void DrawTintedImage( ImageHandle image, const Color & tint, int x, int y, int sx, int sy, int w, int h, int flags = 0 );
	void DrawTintedImage( ImageHandle image, const Color & tint, int x, int y, int cx, int cy, float angle, int sx, int sy, int w, int h, int flags = 0 );

	  // Draws scaled image
	void StretchImage( ImageHandle image, int x, int y, float scale );
	  // Draws scaled image with different scales for X and Y
	void StretchImage( ImageHandle image, int x, int y, float scaleX, float scaleY, int flags );
	  // Clears backbuffer to specified color
	void ClearToColor( const Color & color );
	  // Clears specified image to color
	void ClearImage( ImageHandle image, const Color & color );
	  // Primitives operations
	void DrawLine( int x1, int y1, int x2, int y2, const Color & color, float thikness = 0.0f );
	void DrawLine( ImageHandle image, int x1, int y1, int x2, int y2, const Color & color, float thikness = 0.0f );
	void DrawRect( int x1, int y1, int x2, int y2, const Color & color );
	void DrawRect( ImageHandle image, int x1, int y1, int x2, int y2, const Color & color );
	void DrawRectFill( int x1, int y1, int x2, int y2, const Color & color );
	void DrawRectFill( ImageHandle image, int x1, int y1, int x2, int y2, const Color & color );
	void DrawCircle( int x, int y, int radius, const Color & color );
	void DrawCircle( ImageHandle image, int x, int y, int radius, const Color & color );
	void DrawCircleFill( int x, int y, int radius, const Color & color );
	void DrawCircleFill( ImageHandle image, int x, int y, int radius, const Color & color );

	  // These two are slow!
	void DrawPixel( int x, int y, const Color & color, bool blend = true );	
	void DrawPixel( ImageHandle image, int x, int y, const Color & color, bool blend = false );

	  // Draws text with printf-like syntax
	void DrawText( int x, int y, const Color & color, int size, const char *fmt, ... );
	void DrawText( int x, int y, const Color & color, int size, const UTFString & str, int offset, int length = -1 );
	void DrawText( ImageHandle image, int x, int y, const Color & color, int size, const char *fmt, ... );
	void DrawText( ImageHandle image, int x, int y, const Color & color, int size, const UTFString & str, int offset, int length = -1 );
	  // Draws aligned text with printf-like syntax
	void DrawAlignedText( int x, int y, const Color & color, int size, ETextAlign align, const char *fmt, ... );
	void DrawAlignedText( ImageHandle image, int x, int y, const Color & color, int size, ETextAlign align, const char *fmt, ... );
	  // Draws text with printf-like syntax fitting it into specified area, breaking it into lines
	  // (IGNORES normal line breaks!)
	void DrawTextInArea( int x, int y, int w, int h, const Color & color, int size, const char *fmt, ... );
	void DrawTextInArea( int x, int y, int w, int h, const Color & color, int size, const UTFString & text );
	void DrawTextInArea( ImageHandle image, int x, int y, int w, int h, const Color & color, int size, const char *fmt, ... );
	void DrawTextInAreaInternal( int x, int y, int w, int h, const Color & color, int size );

	  // Draws a quad filled with solid color. Uses TexPoints because I couldn't be bothered with yet ANOTHER point structure...
	void DrawFilledQuad( const TexPoint & p1, const TexPoint & p2, const TexPoint & p3, const TexPoint & p4, const Color & color );
	  // Allows to draw quads with gradients by specifying colors for each vertex
	void DrawColoredQuad(  const ColorPoint & p1, const ColorPoint & p2, const ColorPoint & p3, const ColorPoint & p4 );
	void DrawColoredQuad(  ImageHandle image, const ColorPoint & p1, const ColorPoint & p2, const ColorPoint & p3, const ColorPoint & p4 );
	  // Draws a textured quad. Be advised that U and V are NOT in [0,1] ranged, but rather in [0, textureWidth] and [0, textureHeight] range
	void DrawTexturedQuad( ImageHandle texture, const TexPoint & p1, const TexPoint & p2, const TexPoint & p3, const TexPoint & p4 );
	  // De-initializes render
	void Close();

	  // Readies Render to drawing a single bitmap many times to target image
	void BeginBitmapBatch( ImageHandle targetImage );
	void EndBitmapBatch();

	  // Toggles into fullscreen mode and back. Re-creates bitmaps (or, at least, attempts to)
	void ToggleFullscreen();
	  // Reinitializes Render with new parameters. Re-creates bitmaps (or, at least, attempts to)
	void Reinit( ERenderType type, bool fullscreen, int w, int h, bool vsync );

	void ShowMouseCursor();
	void HideMouseCursor();

	  // Load font from file
	IFontPtr CreateFont( const char *fontName );
	void DestroyFont( IFontPtr font );
	void SetFont( IFontPtr font );
	void SetDefaultFont();
	void SetFontSize( int pixels );
	int GetFontSize() const;
	  // Returns character width in current font
	int GetCharWidth( const char c, int fontSize = -1 ) const;
	int GetCharWidth( const wchar_t c, int fontSize = -1 ) const;
	  // Returns string width in current font
	int GetStringWidth( const char *str, int fontSize = -1 ) const;
	int GetStringWidth( const wchar_t *str, int length, int fontSize = -1 ) const;
	int GetStringWidth( const UTFString & str, int offset, int length = -1, int fontSize = -1 ) const ;
	  // Returns string dimensions in current font. Confusing and somewhat useless
	void MeasureText( const char *str, int &bbx, int &bby, int &bbw, int &bbh ) const;

	  // Locks image for faster pixel-level operations
	void LockImage( ImageHandle image );
	void LockImage( ImageHandle image, int x1, int y1, int x2, int y2 );
	void UnlockImage( ImageHandle image );

	  // Passes each display mode read from current adapter into specified callback (for listing resolutions in Option)
	void EnumerateDisplayModes( IEnumerateDisplayModesCallback & callback ) const;

	void ProcessEvents();
	DisplayEvent GetEvent();
};
