#pragma once
//#include "IImageManager.h"
#include "ResourceHandle.h"

typedef Handle<class IImage*> ImageHandle;

class IFont
{
public:
};

class UTFString;

struct Color
{
	Color()
	{}

	Color( int _r, int _g, int _b, int _a = 255 )
		: r(_r)
		, g(_g)
		, b(_b)
		, a(_a)
	{}
	int r;
	int g;
	int b;
	int a;
	int color;

	bool IsSame( const Color & other, bool compareAlpha = true )
	{
		if ( compareAlpha )
			return color == other.color;

		return r == other.r && g == other.g && b == other.b;
	}
};

  // Describes a point with texture coordinates attached
struct TexPoint
{
	TexPoint()
	{}

	TexPoint( float _x, float _y, float _u, float _v )
		: x(_x)
		, y(_y)
		, u(_u)
		, v(_v)
	{}

	float x, y, u, v;
};

  // Describes a point with color attached, for gradient quads
struct ColorPoint
{
	ColorPoint()
	{}

	ColorPoint( float _x, float _y, const Color & _c )
		: x(_x)
		, y(_y)
		, c(_c)
	{}

	float x, y;
	Color c;
};

enum ETextAlign
{
	TEXT_ALIGN_LEFT   = 0,
	TEXT_ALIGN_RIGHT  = 1,
	TEXT_ALIGN_CENTER = 2,
};

enum EDrawBitmapFlag
{
	DRAW_BITMAP_FLIP_V = 0x0001,
	DRAW_BITMAP_FLIP_H = 0x0002,
};

typedef IFont* IFontPtr;

class IEnumerateDisplayModesCallback
{
public:
	virtual void EnumerateDisplayMode( int bits, int w, int h, int refresh ) = 0;
};

enum EScalingMode
{
	SCALING_NONE,
	SCALING_KEEP_ASPECT,
	SCALING_FREE,
};

enum ERenderType
{
	RENDERTYPE_INVALID  = 0,
	RENDERTYPE_DIRECT3D = 1,
	RENDERTYPE_OPENGL   = 2,
};

struct DisplayEvent
{
	enum EType
	{
		DISPLAY_INVALID = 0,
		DISPLAY_EXPOSE,
		DISPLAY_RESIZE,
		DISPLAY_CLOSE,
		DISPLAY_LOST,
		DISPLAY_FOUND,
		DISPLAY_SWITCH_IN,
		DISPLAY_SWITCH_OUT,
		DISPLAY_ORIENTATION,
		DISPLAY_HALT_DRAWING,
		DISPLAY_RESUME_DRAWING,
	};

	DisplayEvent()
		: m_type( DISPLAY_INVALID )
		, m_x(0)
		, m_y(0)
		, m_w(0)
		, m_h(0)
	{}

	EType m_type;
	int m_x;
	int m_y;
	int m_w;
	int m_h;
};

  // See AllegroRender5 for some comments
class IRender
{
public:
	virtual bool Init( ERenderType type, int w, int h, int bpp, bool fullscreen, bool vsync, const char *defaultFont ) = 0;
	virtual void SetGlobalScaling( EScalingMode mode, float baseW, float baseH ) = 0;
	
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	
	virtual void Flip() = 0;
	
	virtual ImageHandle CreateImage( int w, int h, bool noPreserve, const char *context = 0, bool memory = false ) = 0;
	virtual ImageHandle LoadImage( const char *filename ) = 0;
	virtual ImageHandle CloneImage( const char *filename, bool noPreserve = false ) = 0;
	virtual ImageHandle CloneImage( const ImageHandle & h, bool noPreserve = false ) = 0;
	virtual IImage *GetImageByHandle( const ImageHandle & h ) = 0;
	virtual bool SaveImage( const char *filename, ImageHandle image ) = 0;
	virtual void DestroyImage( ImageHandle image ) = 0;
	virtual void Clear() = 0;
	
	virtual Color MakeColor( int r, int g, int b, int a = 255 ) = 0;

	virtual void DrawImage( ImageHandle image, int x, int y, int sx, int sy, int w, int h, float angle = 0, int flags = 0 ) = 0;

	virtual Color GetPixel( ImageHandle, int x, int y ) = 0;

	virtual void CopyRegion( ImageHandle to, ImageHandle from, int toX, int toY, int fromX, int fromY, int fromW, int fromH, int toW, int toH ) = 0;
	virtual void CopyRegion( ImageHandle from, int toX, int toY, int fromX, int fromY, int fromW, int fromH, int toW, int toH ) = 0;

	virtual void DrawImage( ImageHandle image, int x, int y, int flags = 0 ) = 0;
	virtual void DrawImage( ImageHandle target, ImageHandle image, int x, int y ) = 0;
	virtual void DrawImage( ImageHandle image, int x, int y, float angle ) = 0;
	virtual void DrawImage( ImageHandle image, int x, int y, int px, int py, float angle ) = 0;
	virtual void DrawTintedImage( ImageHandle image, const Color & tint, int x, int y, int flags = 0 ) = 0;
	virtual void DrawTintedImage( ImageHandle target, ImageHandle image, const Color & tint, int x, int y, int flags = 0 ) = 0;
	virtual void DrawTintedImage( ImageHandle target, ImageHandle image, const Color & tint, int x, int y, int sx, int sy, int w, int h, int flags = 0 ) = 0;
	virtual void DrawTintedImage( ImageHandle image, const Color & tint, int x, int y, int sx, int sy, int w, int h, int flags = 0 ) = 0;
	virtual void DrawTintedImage( ImageHandle image, const Color & tint, int x, int y, int cx, int cy, float angle, int sx, int sy, int w, int h, int flags = 0 ) = 0;
	virtual void StretchImage( ImageHandle image, int x, int y, float scale ) = 0;
	virtual void StretchImage( ImageHandle image, int x, int y, float scaleX, float scaleY, int flags ) = 0;
	virtual void ClearToColor( const Color & color ) = 0;
	virtual void ClearImage( ImageHandle image, const Color & color ) = 0;
	virtual void DrawLine( int x1, int y1, int x2, int y2, const Color & color, float thikness = 0.0f ) = 0;
	virtual void DrawLine( ImageHandle image, int x1, int y1, int x2, int y2, const Color & color, float thikness = 0.0f ) = 0;
	virtual void DrawRect( int x1, int y1, int x2, int y2, const Color & color ) = 0;
	virtual void DrawRect( ImageHandle image, int x1, int y1, int x2, int y2, const Color & color ) = 0;
	virtual void DrawRectFill( int x1, int y1, int x2, int y2, const Color & color ) = 0;
	virtual void DrawRectFill( ImageHandle image, int x1, int y1, int x2, int y2, const Color & color ) = 0;
	virtual void DrawCircle( int x, int y, int radius, const Color & color ) = 0;
	virtual void DrawCircle( ImageHandle image, int x, int y, int radius, const Color & color ) = 0;
	virtual void DrawCircleFill( int x, int y, int radius, const Color & color ) = 0;
	virtual void DrawCircleFill( ImageHandle image, int x, int y, int radius, const Color & color ) = 0;
	virtual void DrawPixel( int x, int y, const Color & color, bool blend = false ) = 0;
	virtual void DrawPixel( ImageHandle image, int x, int y, const Color & color, bool blend = false ) = 0;
	virtual void DrawText( int x, int y, const Color & color, int size, const char *fmt, ... ) = 0;
	virtual void DrawText( int x, int y, const Color & color, int size, const UTFString & str, int offset, int length = -1 ) = 0;
	virtual void DrawText( ImageHandle image, int x, int y, const Color & color, int size, const char *fmt, ... ) = 0;
	virtual void DrawText( ImageHandle image, int x, int y, const Color & color, int size, const UTFString & str, int offset, int length = -1 ) = 0;
	virtual void DrawAlignedText( int x, int y, const Color & color, int size, ETextAlign align, const char *fmt, ... ) = 0;
	virtual void DrawAlignedText( ImageHandle image, int x, int y, const Color & color, int size, ETextAlign align, const char *fmt, ... ) = 0;
	virtual void DrawTextInArea( int x, int y, int w, int h, const Color & color, int size, const char *fmt, ... ) = 0;
	virtual void DrawTextInArea( int x, int y, int w, int h, const Color & color, int size, const UTFString & text ) = 0;
	virtual void DrawTextInArea( ImageHandle image, int x, int y, int w, int h, const Color & color, int size, const char *fmt, ... ) = 0;
	virtual void DrawFilledQuad( const TexPoint & p1, const TexPoint & p2, const TexPoint & p3, const TexPoint & p4, const Color & color ) = 0;
	virtual void DrawColoredQuad(  const ColorPoint & p1, const ColorPoint & p2, const ColorPoint & p3, const ColorPoint & p4 ) = 0;
	virtual void DrawColoredQuad(  ImageHandle image, const ColorPoint & p1, const ColorPoint & p2, const ColorPoint & p3, const ColorPoint & p4 ) = 0;
	virtual void DrawTexturedQuad( ImageHandle texture, const TexPoint & p1, const TexPoint & p2, const TexPoint & p3, const TexPoint & p4 ) = 0;

	virtual void BeginBitmapBatch( ImageHandle targetImage = ImageHandle::INVALID ) = 0;
	virtual void EndBitmapBatch() = 0;

	virtual void ShowMouseCursor() = 0;
	virtual void HideMouseCursor() = 0;

	virtual void ToggleFullscreen() = 0;
	virtual void Reinit( ERenderType type, bool fullscreen, int w, int h, bool vsync ) = 0;	

	virtual IFontPtr CreateFont( const char *fontName ) = 0;
	virtual void DestroyFont( IFontPtr font ) = 0;
	virtual void SetFont( IFontPtr font ) = 0;
	virtual void SetDefaultFont() = 0;
	virtual void SetFontSize( int pixels ) = 0;
	virtual int GetFontSize() const = 0;
	virtual int GetCharWidth( const char c, int fontSize = -1 ) const = 0;
	virtual int GetCharWidth( const wchar_t c, int fontSize = -1 ) const = 0;
	virtual int GetStringWidth( const char *str, int fontSize = -1 ) const = 0;
	virtual int GetStringWidth( const wchar_t *str, int length = -1, int fontSize = -1 ) const = 0;
	virtual int GetStringWidth( const UTFString & str, int offset, int length = -1, int fontSize = -1 ) const = 0;
	virtual void MeasureText( const char *str, int &bbx, int &bby, int &bbw, int &bbh ) const = 0;
	virtual void Close() = 0;
	virtual void LockImage( ImageHandle image ) = 0;
	virtual void LockImage( ImageHandle image, int x1, int y1, int x2, int y2 ) = 0;
	virtual void UnlockImage( ImageHandle image ) = 0;

	virtual void EnumerateDisplayModes( IEnumerateDisplayModesCallback & callback ) const = 0;

	virtual void ProcessEvents() = 0;
	virtual DisplayEvent GetEvent() = 0;
};

IRender & GetRender();
