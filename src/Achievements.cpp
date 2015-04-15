#include "stdafx.h"
#include "Achievements.h"
#include "PlayerProfile.h"
#include "GameContext.h"
#include "Vars.h"

/*
	Achievements are hard-coded to allow making hidden achievements
	and to protect their GUIDs for at least a bit. Since this game
	is open-source there is really no point in it, but still...
*/

namespace
{
	  // Statistic types to keep kill counts
	enum EStatTypes
	{
		STAT_TYPE_PROP,
		STAT_TYPE_JET,
		STAT_TYPE_COPTER,
		STAT_TYPE_TWO_SEATER,
		STAT_TYPE_BOMBER,
		STAT_TYPE_UFO,
		STAT_TYPE_HYPER_SLOW,
		STAT_TYPE_HYPER_FAST,
		STAT_TYPE_PARANOIA,
		STAT_TYPE_SHUTTLE,
		STAT_TYPE_SPACEJET,
		STAT_TYPE_PLAYER,
		STAT_TYPE_PLAYER_BOMB,
	};

	RegisterIntVar VAR_STAT_TYPE_PROP("STAT_TYPE_PROP", STAT_TYPE_PROP);
	RegisterIntVar VAR_STAT_TYPE_JET("STAT_TYPE_JET", STAT_TYPE_JET );
	RegisterIntVar VAR_STAT_TYPE_COPTER("STAT_TYPE_COPTER", STAT_TYPE_COPTER);
	RegisterIntVar VAR_STAT_TYPE_TWO_SEATER("STAT_TYPE_TWO_SEATER", STAT_TYPE_TWO_SEATER);
	RegisterIntVar VAR_STAT_TYPE_BOMBER("STAT_TYPE_BOMBER", STAT_TYPE_BOMBER);
	RegisterIntVar VAR_STAT_TYPE_UFO("STAT_TYPE_UFO", STAT_TYPE_UFO);
	RegisterIntVar VAR_STAT_TYPE_HYPER_SLOW("STAT_TYPE_HYPER_SLOW", STAT_TYPE_HYPER_SLOW);
	RegisterIntVar VAR_STAT_TYPE_HYPER_FAST("STAT_TYPE_HYPER_FAST", STAT_TYPE_HYPER_FAST);
	RegisterIntVar VAR_STAT_TYPE_PARANOIA("STAT_TYPE_PARANOIA", STAT_TYPE_PARANOIA);
	RegisterIntVar VAR_STAT_TYPE_SHUTTLE("STAT_TYPE_SHUTTLE", STAT_TYPE_SHUTTLE);
	RegisterIntVar VAR_STAT_TYPE_SPACEJET("STAT_TYPE_SPACEJET", STAT_TYPE_SPACEJET);
	RegisterIntVar VAR_STAT_TYPE_PLAYER("STAT_TYPE_PLAYER", STAT_TYPE_PLAYER);
	RegisterIntVar VAR_STAT_TYPE_PLAYER_BOMB("STAT_TYPE_PLAYER_BOMB", STAT_TYPE_PLAYER_BOMB);
}

  // Checks that percent of hits with gun is greater or equal to specified in a single level
class LevelHitPercGE : public IAchievChecker
{
	int m_reqPerc;
public:
	LevelHitPercGE( int perc )
		: m_reqPerc( perc )
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		if ( context.GetCurrentLevel() < 2 || prof.m_levelShotsFired <= 0 )
			return false;

		return prof.m_levelHits * 100 / prof.m_levelShotsFired >= m_reqPerc;
	}
};

  // Checks that percent of hits with falling enemies and bombs is greater or equal to specified in a single level
class LevelBombHitPercGE : public IAchievChecker
{
	int m_reqPerc;
public:
	LevelBombHitPercGE( int perc )
		: m_reqPerc( perc )
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		if ( prof.m_levelTargetHits + prof.m_levelTargetMisses <= 0 )
			return false;

		return prof.m_levelTargetHits * 100 / ( prof.m_levelTargetHits + prof.m_levelTargetMisses ) >= m_reqPerc;
	}
};

  // Checks that number of kills of given type is greater or equal to specified in a single level
class KillsByType : public IAchievChecker
{
	int m_type;
	int m_reqKills;	
public:
	KillsByType( int type, int reqKills )
		: m_type( type )
		, m_reqKills( reqKills )
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		if ( m_type < 0 || m_type >= STAT_TYPES_COUNT )
			return false;

		return prof.m_levelKillsByType[ m_type ] >= m_reqKills;
	}
};

  // Checks that total number of kills is greater or equal to specified in a single level
class KillsTotal : public IAchievChecker
{
	int m_reqKills;
public:
	KillsTotal( int kills )
		: m_reqKills( kills )
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		if ( context.GetCurrentLevel() < 2 )
			return false;

		int kills = 0;
		for ( int i = 0; i < STAT_TYPES_COUNT; ++i )
			kills += prof.m_levelKillsByType[ i ];
		
		return kills >= m_reqKills;
	}
};

  // Checks that number of kills in longes series (in 5 seconds) is greater or equal to specified in a single level
class TimedKills : public IAchievChecker
{
	int m_reqKills;
public:
	TimedKills( int kills )
		: m_reqKills( kills )
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		return prof.m_maxLevelTimedKills >= m_reqKills;
	}
};

  // Checks that time of level completion is less or equal to specified
class LevelTimeLE : public IAchievChecker
{
	int m_reqTime;
public:
	LevelTimeLE( float time )
		: m_reqTime( time )
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		return prof.m_levelTime <= m_reqTime;
	}	
};

  // Checks that player has won specified level
class WinLevel : public IAchievChecker
{
	int m_reqLevel;
	bool m_hardcore;
public:
	WinLevel( int level, bool hardcore )
		: m_reqLevel( level )
		, m_hardcore( hardcore )
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		return ( context.GetCurrentLevel() == m_reqLevel ) && ( !m_hardcore || prof.m_hardcore );
	}	
};

  // Checks that player has won specified number of levels without losing a life
class UnkillableStreak : public IAchievChecker
{
	int m_reqStreak;
public:
	UnkillableStreak( int streak )
		: m_reqStreak( streak )
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		return prof.m_unkillableStreak >= m_reqStreak;
	}	
};

  // Checks that target was sunk with object of specified type
class LastHitType : public IAchievChecker
{
	EStatTypes m_type;
public:
	LastHitType( EStatTypes type )
		: m_type( type )
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		return prof.m_levelLastTargetHitType == m_type;
	}	
};

  // Checks that target was only hit with copters, bombers, UFOs and shuttles
class Picky : public IAchievChecker
{
public:
	Picky()
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		int allHits = 0;
		for ( int i = 0; i < STAT_TYPES_COUNT; ++i )
			allHits += prof.m_levelTargetHitsByType[ i ];

		const int bigHits = 
			prof.m_levelTargetHitsByType[ STAT_TYPE_COPTER ] +
			prof.m_levelTargetHitsByType[ STAT_TYPE_BOMBER ] +
			prof.m_levelTargetHitsByType[ STAT_TYPE_UFO ] +
			prof.m_levelTargetHitsByType[ STAT_TYPE_SHUTTLE ];

		const int smallHits = allHits - bigHits;

		return bigHits > 0 && smallHits == 0;
	}
};

  // Checks that target was only hit with basic prop planes, basic jet planes or space jets
class EasyPickings : public IAchievChecker
{
public:
	EasyPickings()
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		if ( context.GetCurrentLevel() < 5 )
			return false;

		int allHits = 0;
		for ( int i = 0; i < STAT_TYPES_COUNT; ++i )
			allHits += prof.m_levelTargetHitsByType[ i ];

		const int simpleHits = 
			prof.m_levelTargetHitsByType[ STAT_TYPE_PROP ] +
			prof.m_levelTargetHitsByType[ STAT_TYPE_SPACEJET ] +
			prof.m_levelTargetHitsByType[ STAT_TYPE_JET ];

		const int bigHits = allHits - simpleHits;

		return simpleHits > 0 && bigHits == 0;
	}
};

  // Checks that player never stayed too long in the bottom part of the screen
class HighFlyer : public IAchievChecker
{
public:
	HighFlyer()
	{}

	bool operator()( const PlayerProfile & prof, const IGameContext & context ) const
	{
		if ( context.GetCurrentLevel() < 10 )
			return false;

		return prof.m_highFlyerTimer < 5.0f;
	}
};

  // Array of achievements
SAchievement g_achievements[] = 
{
	{ "71836A10-8B95-11E1-8DAC-A382BBE59B18", "Gunfighter",			"Achieve 75% accuracy in a single level (level 2 and higher)",		"ach_acc1.png", new LevelHitPercGE( 75 ) },
	{ "85028A62-8B95-11E1-BAE6-BB82BBE59B18", "Sniper",				"Achieve 85% accuracy in a single level (level 2 and higher)",		"ach_acc2.png", new LevelHitPercGE( 85 ) },
	{ "883FB588-8B95-11E1-82F0-C282BBE59B18", "Deadeye",			"Achieve 95% accuracy in a single level (level 2 and higher)",		"ach_acc3.png", new LevelHitPercGE( 95 ) },
	{ "8C322ACC-8B95-11E1-A0FF-D382BBE59B18", "One Shot One Kill",  "Achieve 100% accuracy in a single level (level 2 and higher)",		"ach_acc4.png", new LevelHitPercGE( 100 ) },
	{ "8F2E3E82-8B95-11E1-8242-D782BBE59B18", "Bombardier",			"Achieve 90% hits on target in a single level",						"ach_bmb1.png", new LevelBombHitPercGE( 90 ) },
	{ "91C6A09E-8B95-11E1-A26E-D882BBE59B18", "Bomber Ace",			"Achieve 95% hits on target in a single level",						"ach_bmb2.png", new LevelBombHitPercGE( 95 ) },
	{ "94B34636-8B95-11E1-A96D-DE82BBE59B18", "Tight Cluster",		"Achieve 100% hits on target in a single level",					"ach_bmb3.png", new LevelBombHitPercGE( 100 ) },
	{ "98C0E10C-8B95-11E1-825F-EC82BBE59B18", "Werewolf Hunter",    "Destroy 20 two-seater in a single level",							"ach_twoseat.png", new KillsByType( STAT_TYPE_TWO_SEATER, 20 ) },
	{ "9B52E6A4-8B95-11E1-9386-F382BBE59B18", "Illegal Aliens",		"Destroy 10 UFOs in a single level",								"ach_ufo.png", new KillsByType( STAT_TYPE_UFO, 10 ) },
	{ "9E003758-8B95-11E1-B513-FB82BBE59B18", "Ground Repulses Them", "Destroy 15 helicopters in a single level",						"ach_copter.png", new KillsByType( STAT_TYPE_COPTER, 15 ) },
	{ "A06C48BA-8B95-11E1-83BA-0183BBE59B18", "Scramble",			"Destroy 15 bombers in a single level",								"ach_bomber.png", new KillsByType( STAT_TYPE_BOMBER, 15 ) },
	{ "A291CA52-8B95-11E1-941A-0383BBE59B18", "Stop 'em Cold",		"Destroy 10 speeding hyperjets in a single level",					"ach_hyper_fast.png", new KillsByType( STAT_TYPE_HYPER_FAST, 10 ) },
	{ "8CEB0851-5940-4231-9E14-187E6680027F", "Sitting Ducks",		"Destroy 10 hyperjets before they accelerate in a single level",	"ach_hyper_slow.png", new KillsByType( STAT_TYPE_HYPER_SLOW, 10 ) },
	{ "E93EF61A-8D3F-43b2-A016-39F771F32166", "Space Ace",			"Destroy 10 shuttles in a single level",							"ach_shuttles.png", new KillsByType( STAT_TYPE_SHUTTLE, 10 ) },
	{ "2B2DA11A-787D-4ac5-9ED9-411A7B0B1B76", "Red Baron",			"Achieve 80 kills in a single level (level 2 and higher)",			"ach_red_baron.png", new KillsTotal( 80 ) },
	{ "4EF2A2F1-1DD5-428f-874A-F92E3CD838EA", "Hat Trick",			"Destroy 12 enemy planes in 5 seconds",								"ach_time_kill1.png", new TimedKills( 12 ) },
	{ "4B9C9E79-DABE-423e-8AD8-80C78D4BBBD8", "Killing Spree",		"Destroy 15 enemy planes in 5 seconds",								"ach_time_kill2.png", new TimedKills( 15 ) },
	{ "9F0907B6-1625-46c1-A932-3F280B76DA4E", "Bloodbath",			"Destroy 20 enemy planes in 5 seconds",								"ach_time_kill3.png", new TimedKills( 20 ) },
	{ "EF639354-B51F-4791-9844-204942210162", "Heavy Barrage",		"Finish level in 45 seconds",										"ach_level_time.png", new LevelTimeLE( 45 ) },
	{ "4542273D-481D-4fd6-B4F6-2119940910E8", "Playing the Game",	"Win the first level",												"ach_level_1.png", new WinLevel( 1, false ) },
	{ "EDC3CF43-E316-475e-8D96-C28E3E1F882F", "Properly Paranoid",	"Win 17th level",													"ach_level_17.png", new WinLevel( 17, false ) },
	{ "B49FFCE8-9681-4cdb-A1A3-3BDD9703C0CE", "Oldskool Hardcore",	"Win the game in hardcore mode",									"ach_win_hard.png", new WinLevel( 21, true ) },
	{ "AFB9A910-AF98-4bd9-A695-55C6215831E5", "Bring It Down",		"Win the game in any mode",											"ach_win_any.png", new WinLevel( 21, false ) },
	{ "15A0A5DE-BD83-42e0-9A5A-AE7D3DC46375", "Lucky",				"Win a level without losing a life (level 2 and higher)",			"ach_unkill1.png", new UnkillableStreak( 1 ) },
	{ "707715A0-C1A2-4ed3-83CB-E3319E374F14", "Ace Pilot",			"Win 3 levels without losing a life (level 2 and higher)",			"ach_unkill2.png", new UnkillableStreak( 3 ) },
	{ "203160F6-01B6-46aa-AF57-7208E4D72709", "Surviviour",			"Win 5 levels without losing a life (level 2 and higher)",			"ach_unkill3.png", new UnkillableStreak( 5 ) },
	{ "6A57A9EF-F40D-42cd-886A-00D5F0A4F0B2", "Unkillable",			"Win 10 levels without losing a life (level 2 and higher)",			"ach_unkill4.png", new UnkillableStreak( 10 ) },
	{ "40A8BFBF-F93C-459b-87E3-78A43DDD56E3", "One-Man Show",		"Win the game without losing a life",								"ach_unkill5.png", new UnkillableStreak( 20 ) },
	{ "F61674BA-DF27-4ffe-AF62-345D4F82925F", "Wind Of Gods",		"Sink target with your own plane",									"ach_last_self.png", new LastHitType( STAT_TYPE_PLAYER ) },
	{ "762EE28C-EE93-45f6-AE7C-D569EC37B332", "One Last Touch",		"Sink target with your bomb",										"ach_last_bomb.png", new LastHitType( STAT_TYPE_PLAYER_BOMB ) },
	{ "63652CBA-138A-497f-B682-34E7B35086AD", "Picky",				"Sink target while hitting it only with copters, bombers, ufos and shuttles",	  "ach_big_kills.png", new Picky() },
	{ "B19BE9C1-7A49-43ee-BE44-A0E277E05C57", "Easy Pickings",		"Sink target while hitting it only with basic enemy planes (level 5 and higher)", "ach_small_kills.png", new EasyPickings() },
	{ "D938049C-C888-4505-A251-16072A3E3BF0", "High Flyer",			"Stay in the upper part of the screen for the whole level (level 10 and higher)", "ach_high_flyer.png", new HighFlyer() },
};

Achievements::Achievements()
	: m_achievementsCount( 0 )
	, m_achievements( g_achievements )
{
	m_achievementsCount = sizeof( g_achievements ) / sizeof( g_achievements[0] );
}

const SAchievement* Achievements::GetAchievement( int index ) const
{
	if ( index < 0 || index >= m_achievementsCount )
		return 0;

	return &m_achievements[ index ];
}

int Achievements::GetAchievementCount() const
{
	return m_achievementsCount;
}

const Achievements & GetAchievements()
{
	static Achievements ach;
	return ach;
}
