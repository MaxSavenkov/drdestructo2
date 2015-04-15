#pragma once
#include "BaseMenu.h"
#include "MenuHelpers.h"
#include "Common.h"

/*
	This file contains menu that displays Achievements and exports them.
	See BaseMenu for general comments on menu system.
*/

class IAchMenuCallback
{
public:
	virtual void BackFromAchievements() = 0;
};

class IGameContext;

  // Information about single achievement to be displayed
struct SAchEntry
{
	  // Icon
	ImageHandle m_icon;
	  // Name
	std::string m_name;
	  // Description
	std::string m_desc;
	  // True if player has unlocked that achievement
	bool m_has;
};

/*
	Menu class itself.
*/
class AchMenu : public BaseMenu, SimpleMenuEntry::ICallback
{
	static const int EID_EXPORT_BIG		= 1;
	static const int EID_EXPORT_SMALL	= 2;
	static const int EID_EXPORT_HOR		= 3;
	static const int EID_BACK			= 4;

	IAchMenuCallback *m_pCallback;

	//SimpleMenuEntry m_exportBig;
	//SimpleMenuEntry m_exportSmall;
	//SimpleMenuEntry m_exportHorizontal;
	SimpleMenuEntry m_back;

	  // If false, achievements' list has not yet been populated
	bool m_init;

	  // Achievements list
	std::vector<SAchEntry> m_achievements;
	  // If true, cursor is now in achievements section of screen, if false - in menu section
	bool m_inAchievements;
	  // Selected achievement index
	int m_selectedAchievement;

	  // Displayed achievements parameters
	  // Width
	static const int ACH_WIDTH = 150 + 20;
	  // Height
	static const int ACH_HEIGHT = 50 + 30;
	  // Number of achievements per line
	static const int ACH_PER_LINE = SCREEN_W / ACH_WIDTH;

	void SaveBinAchievements();
	void SaveSmallAchievements();
	void SaveHorizontalAchievements();

	bool TrySelectAch( int x, int y );
public:
	AchMenu( IAchMenuCallback *pCallback );
	void Init( IGameContext & context );
	void Render( IRender *pRender );
	void ProcessInput( IInput *pInput );
	void operator()( int id );
	void Back() { this->operator ()( EID_BACK ); }	
};
