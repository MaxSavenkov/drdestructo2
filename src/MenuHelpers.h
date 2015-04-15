#pragma once
#include "BaseMenu.h"
#include "IRender.h"
#include "UIHelpers.h"

class GameMenuEntry;

/*
	Menu entry placement.

	PLACEMENT_VERTICAL - stack entries on top of each other, top to bottom
	PLACEMENT_FOOTER   - place entries next to each other, left to right, at the bottom of menu
*/
enum EPlacement
{
	PLACEMENT_VERTICAL,
	PLACEMENT_FOOTER,
};

/*
	Base graphical game menu
*/
class GameMenu : public BaseMenu
{
public:
	GameMenu( const std::string & caption, int x, int y, int width, int height )
		: BaseMenu( caption,x , y )
	{
		m_imgInit = false;
		m_width = width;
		m_height = height;
	}

	virtual void AddEntry( MenuEntry *pEntry );

	virtual void Render( IRender *pRender );

public:
	bool m_imgInit;

	ImageHandle m_atlas;

	WindowBorder m_border;
	Grid9 m_frame;

	int m_width, m_height;
};

/*
	A set of ready-made menu entries for you to use!
*/

  // A simple menu entry that can display a string of text.
  // Not very usable by itself, because it can't even process Enter key!
class TextMenuEntry : public MenuEntry
{
protected:
	std::string m_label;
	Color m_color;

public:
	TextMenuEntry( BaseMenu *menu, int order, const std::string & label, const Color & color, int placement = PLACEMENT_VERTICAL )
		: MenuEntry( menu, order, 305, 47, placement )
		, m_label( label )
		, m_color( color )
	{}

	virtual void Render( IRender *pRender, int x, int y );
	virtual void Update( float dt ) {}
};

  // A simple button menu entry. Displays a picture for each of its states and reacts to activation
  // by calling a specified callback
class ButtonMenuEntry : public MenuEntry
{
public:
	class ICallback
	{
	public:
		virtual void operator()( int id ) = 0;
	};

private:
	  // Callback to call
	ICallback *m_pCallback;
	  // ID which will be passed to callback
	int m_callbackID;

protected:
	  // Text to display
	std::string m_label;
	  // Text position relative to image
	int m_textOffsetX;
	int m_textOffsetY;

	  // Names of files for each state
	std::string m_fileOn;      // hover
	std::string m_fileOff;     // normal
	std::string m_filePressed; // pressed
	ImageHandle m_imageOn;
	ImageHandle m_imageOff;
	ImageHandle m_imagePressed;

	  // To delay callback call a bit after press
	bool  m_pressed;
	float m_pressedTimer;

public:
	ButtonMenuEntry( BaseMenu *menu, int order, const std::string & label, int tx, int ty, const char *fileOn, const char *fileOff, const char *filePressed, int w, int h, int placement, int callbackID, ICallback *pCallback )
		: MenuEntry( menu, order, w, h, placement )
		, m_label( label )
		, m_pCallback( pCallback )
		, m_callbackID( callbackID )
		, m_fileOn( fileOn )
		, m_fileOff( fileOff )
		, m_filePressed( filePressed )
		, m_textOffsetX( tx )
		, m_textOffsetY( ty )
		, m_pressed( false )
	{}

	virtual void Render( IRender *pRender, int x, int y );
	virtual void Update( float dt );
	virtual void ProcessInput( IInput *pInput );
	virtual void Activate();
};

  // Normal button, which are placed on top of each other
class VerticalButtonEntry : public ButtonMenuEntry
{
public:
	VerticalButtonEntry( BaseMenu *menu, int order , const std::string & label, int callbackID, ICallback *pCallback )
		: ButtonMenuEntry( menu, order, label, 35, 10, "Data/Sprites2/ui/menu_btn_selected.png", "Data/Sprites2/ui/menu_btn_normal.png", "Data/Sprites2/ui/menu_btn_pressed.png", 308, 48, PLACEMENT_VERTICAL, callbackID, pCallback )
	{}
};

  // Accept button, which is place into the footer
class FooterAcceptButtonEntry : public ButtonMenuEntry
{
public:
	FooterAcceptButtonEntry( BaseMenu *menu, int order , const std::string & label, int callbackID, ICallback *pCallback )
		: ButtonMenuEntry( menu, order, label, 0, 15, "Data/Sprites2/ui/menu_btn_apply_selected.png", "Data/Sprites2/ui/menu_btn_apply_normal.png", "Data/Sprites2/ui/menu_btn_apply_pressed.png", 218, 60, PLACEMENT_FOOTER, callbackID, pCallback )
	{}
};

  // Cancel/Decline button, which is place into the footer
class FooterDeclineButtonEntry : public ButtonMenuEntry
{
public:
	FooterDeclineButtonEntry( BaseMenu *menu, int order , const std::string & label, int callbackID, ICallback *pCallback )
		: ButtonMenuEntry( menu, order, label, 0, 15, "Data/Sprites2/ui/menu_btn_cancel_selected.png", "Data/Sprites2/ui/menu_btn_cancel_normal.png", "Data/Sprites2/ui/menu_btn_cancel_pressed.png", 218, 60, PLACEMENT_FOOTER, callbackID, pCallback )
	{}
};

// Obsolete text menu entry, which calls a callback when
// Enter key is pressed on it.
class SimpleMenuEntry : public TextMenuEntry
{
public:
	class ICallback
	{
	public:
		virtual void operator()( int id ) = 0;
	};

private:
	  // Callback to call
	ICallback *m_pCallback;
	  // ID which will be passed to callback
	int m_callbackID;

public:
	SimpleMenuEntry( BaseMenu *menu, int order , const std::string & label, const Color & color, int callbackID, ICallback *pCallback, int placement = PLACEMENT_VERTICAL )
		: TextMenuEntry( menu, order, label, color, placement )
		, m_pCallback( pCallback )
		, m_callbackID( callbackID )
	{}

	virtual void ProcessInput( IInput *pInput );
	virtual void Activate();
	virtual int GetWidth();
};

  // This entry can display a label and a changeable value. 
  // You can scroll through the list of value susing Left/Right keys.
  // It's a base class for such entries. Use ValueMenuEntry<T>.
class ValueMenuEntry : public MenuEntry
{
protected:
	std::string m_label;
	ImageHandle m_leftOn;
	ImageHandle m_leftOff;
	ImageHandle m_rightOn;
	ImageHandle m_rightOff;
	ImageHandle m_arrowLeft;
	ImageHandle m_arrowRight;
	ImageHandle m_arrowLeftPressed;
	ImageHandle m_arrowRightPressed;	

	virtual const char *CurrentValue() = 0;

protected:
	bool m_pressedLeft;
	bool m_pressedRight;
	bool m_needArrows;

protected:
	int m_screenX;

public:
	ValueMenuEntry( BaseMenu *menu, int order, const std::string & label, int w, int h, bool needArrows = true )
		: MenuEntry( menu, order, w, h, PLACEMENT_VERTICAL )
		, m_label( label )
		, m_needArrows( needArrows )
		, m_pressedLeft( false )
		, m_pressedRight( false )
		, m_screenX(-1)
	{}

	virtual void Render( IRender *pRender, int x, int y );
	virtual int GetWidth();
	virtual void Update( float dt ) {}
};

class ValueListMenuEntryBase : public ValueMenuEntry
{
public:
	ValueListMenuEntryBase( BaseMenu *menu, int order, const std::string & label, int w = 800, int h = 64 )
		: ValueMenuEntry( menu, order, label, w, h )
	{}

	virtual void ProcessInput( IInput *pInput );

	virtual void NextValue() = 0;
	virtual void PrevValue() = 0;
};

  // This entry can display a label and a changeable value. 
  // You can scroll through the list of value using Left/Right keys.
  // With each displayed value a hidden value of type TData is
  // associated.
template<typename TData>
class ValueListMenuEntry : public ValueListMenuEntryBase
{
protected:
	virtual void OnValueChanged( int index ){}

	void NextValue()
	{
		if ( !m_values.empty() )
		{
			++m_selectedValue;
			if ( m_selectedValue > (int)m_values.size() - 1 )
				m_selectedValue = 0;//(int)m_values.size() - 1;
			OnValueChanged( m_selectedValue );
		}	
	}
	void PrevValue()
	{
		if ( !m_values.empty() )
		{
			--m_selectedValue;
			if ( m_selectedValue < 0 )
				m_selectedValue = (int)m_values.size() - 1; //0
			OnValueChanged( m_selectedValue );
		}
	}
	const char *CurrentValue()
	{
		if ( m_selectedValue < 0 || m_selectedValue >= (int)m_values.size() )
			return 0;

		return m_values[ m_selectedValue ].m_value.c_str();
	}

protected:
	struct SData
	{
		SData( const std::string & value, const TData & data )
			: m_value( value )
			, m_data( data )
		{}

		std::string m_value;
		TData m_data;
	};
	
	std::vector< SData > m_values;
	int m_selectedValue;

public:
	ValueListMenuEntry( BaseMenu *menu, int order, const std::string & label, int w = 800, int h = 64 )
		: ValueListMenuEntryBase( menu, order, label, w, h )
		, m_selectedValue( -1 )
	{}

	void ClearValues() { m_values.clear(); m_selectedValue = -1; }
	int AddValue( const std::string & value, const TData & data ) { m_values.push_back( SData( value, data ) ); return (int)m_values.size() - 1; }
	void SetSelectedIndex( int index ) { if ( index < 0 || index >= (int)m_values.size() ) return; m_selectedValue = index; }
	int GetSelectedIndex() const { return m_selectedValue; }
	int GetValuesCount() const { return (int)m_values.size(); }
	const TData *GetSelectedData() const
	{
		if ( m_selectedValue < 0 || m_selectedValue >= (int)m_values.size() )
			return 0;

		return &m_values[ m_selectedValue ].m_data;
	}
	void SetSelectedIndexByData( const TData & data )
	{
		for ( size_t i = 0; i < m_values.size(); ++i )
		{
			if ( m_values[ i ].m_data == data )
			{
				m_selectedValue = i;
				SetSelectedIndex( (int)i );
				break;
			}
		}
	}

	void Activate()
	{
		NextValue();
	}
};

  // This menu entry supports text input
  // Press Enter to start entering text.
  // Press Enter again to finish or Escape to cancel
  // It will call a callback when accepting or canceling text input
class TextInputMenuEntry : public TextMenuEntry
{
public:
	class ICallback
	{
	public:
		virtual void AcceptEdit( const std::string & value ) = 0;
		virtual void CancelEdit() = 0;
	};	

protected:
	  // Current displayed value
	std::string m_value;
	  // Maximum value length
	int m_maxLength;
	  // Old value for cancelling
	std::string m_oldValue;
	  // If true, user is typing text right now
	bool m_editing;
	  // If true, user can cancel input
	bool m_canCancel;
	  // Callback to call when text is entered or cancelled
	ICallback *m_pCallback;
public:

	TextInputMenuEntry( BaseMenu *menu, int order, const std::string & label, const std::string & value, int maxLength, const Color & color, ICallback *pCallback )
		: TextMenuEntry( menu, order, label, color )
		, m_value( value )
		, m_maxLength( maxLength )
		, m_editing( false )
		, m_canCancel( true )
		, m_pCallback( pCallback )
	{
	}

	virtual void Render( IRender *pRender, int x, int y );
	virtual void ProcessInput( IInput *pInput );
	void StartEdit() { m_editing = true; }
	void SetCanCancel( bool value ) { m_canCancel = value; }
	void SetValue( const std::string & value ) { m_value = value; }
};

  // A specialization of ValueMenuEntry for a simple case
  // when you only want to display On/Off choice for player.
class OnOffMenuEntry : public ValueListMenuEntry<bool>
{
public:
	OnOffMenuEntry( BaseMenu *menu, int order, const std::string & label, bool on )
		: ValueListMenuEntry<bool>( menu, order, label )
	{
		AddValue( "Off", false );
		AddValue( "On", true );
		SetSelectedIndex( on ? 1 : 0 );
	}
};
