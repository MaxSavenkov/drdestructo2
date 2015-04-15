#pragma once

#include "Engine/IImageManager.h"

/*
	This is the base classes for all game menus.
*/

typedef int EntryID;

class MenuEntry;
class IRender;
class IInput;

/*
	Base class for menu. Provides basic functionality like storing entries,
	supporting cursor movement and selection.
*/
class BaseMenu
{
protected:
	  // Menu caption
	std::string m_caption;
	  // List of menu entries
	typedef std::vector< MenuEntry* > EntryVector;
	EntryVector m_entries;
	  // Menu position on screen (top-left cornet)
	int m_x, m_y;
	  // Selected entry
	EntryID m_selected;

	friend class MenuEntry;

	  // Entries add themselves. Ain't it nice?
	  // Virtual, so child classes could have a chance to change something
	virtual void AddEntry( MenuEntry *entry );

protected:
	bool CursorUp();
	bool CursorDown();
	bool CursorLeft();
	bool CursorRight();
	void SelectAt( int x, int y );
	void ActivateAt( int x, int y );

public:
	BaseMenu( const std::string & caption, int x, int y )
		: m_caption( caption )
		, m_x( x )
		, m_y( y )
		, m_selected( 0 )
	{
	
	}

	virtual void Render( IRender *pRender );
	virtual void ProcessInput( IInput *pInput );
	virtual void Update( float dt );
	virtual void OnShow(){}

	  // Called when player presses Escape or Back button (on Android)
	virtual void Back(){}

	void SetSelected( int index );
	void SetSelected( const MenuEntry *entry );
};

/*
	This is a very basic menu entry class.
	It doesn't even know how to display itself.
	You will only need to use it if you want to
	create your own kind of menu entry. Otherwise,
	looks at MenuHelpers.h.
*/
class MenuEntry
{
protected:
	  // The bigger this is, the lower entry's position will be
	int m_order;
	  // True if entry is selected
	bool m_selected;	

	int m_nextUp, m_nextDown, m_nextLeft, m_nextRight;

public:
	MenuEntry( BaseMenu *menu, int order, int w, int h, int placementType = 0 )
		: m_selected( false )
		, m_order( order )
		, m_nextUp( -1 )
		, m_nextDown( -1 )
		, m_nextLeft( -1 )
		, m_nextRight( -1 )
		, m_w( w )
		, m_h( h )
		, m_placementType( placementType )
	{
		if ( menu )
			menu->AddEntry( this );
	}

	virtual void Render( IRender *pRender, int x, int y ) = 0;
	virtual void ProcessInput( IInput *pInput ) = 0;
	virtual void Update( float dt ) = 0;
	virtual void Activate() = 0;
	
	int GetNextUp() const { return m_nextUp; }
	int GetNextDown() const { return m_nextDown; }
	int GetNextLeft() const { return m_nextLeft; }
	int GetNextRight() const { return m_nextRight; }
	
	int GetOrder() const { return m_order; }
	void SetSelected( bool value ) { m_selected = value; }

	void SetNextUp( int value ) { m_nextUp = value; }
	void SetNextDown( int value ) { m_nextDown = value; }
	void SetNextLeft( int value ) { m_nextLeft = value; }
	void SetNextRight( int value ) { m_nextRight = value; }

	virtual int GetX1() const { return m_x; }
	virtual int GetX2() const { return m_x + m_w; }
	virtual int GetY1() const { return m_y; }
	virtual int GetY2() const { return m_y + m_h; }

	int GetPlacementType() const { return m_placementType; }
	void SetPlacementType( int type ) { m_placementType = type; }

	void SetX( int x ) { m_x = x; }
	void SetY( int y ) { m_y = y; }

protected:
	int m_x, m_y, m_w, m_h;
	int m_placementType;
};
