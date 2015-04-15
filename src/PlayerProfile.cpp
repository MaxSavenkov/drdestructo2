#include "stdafx.h"
#include "Engine/ISystem.h"
#include "PlayerProfile.h"

PlayerProfile::PlayerProfile()
	: m_maxUnlockedLevel( 1 )
	/*, m_shotsFired( 0 )
	, m_hits( 0 )
	, m_objectsDestroyed( 0 )
	, m_objectsHitTarget( 0 )
	, m_gamesStarted( 0 )
	, m_levelsWon( 0 )
	, m_gamesWon( 0 )
	, m_livesSpent( 0 )
	, m_timePlayed( 0 )*/
	, m_hardcore( false )
	, m_unkillableStreak( 0 )
{
	ResetLevelStats();
}

void PlayerProfile::Load()
{
	m_achievements.clear();

	ISystem & sys = GetSystem();
	std::vector<char> buf;
	if ( !sys.LoadFile( false, "player.xml", buf ) )
		return;

	TiXmlDocument doc;
	if ( !doc.Parse( &buf[0] ) )
		return;

	TiXmlElement *pRoot = doc.FirstChildElement("Player");
	if ( !pRoot )
		return;

	TiXmlElement *pStats = pRoot->FirstChildElement("Stats");
	pStats->QueryIntAttribute( "MaxLevelUnlocked", &m_maxUnlockedLevel );
	/*pStats->QueryIntAttribute( "ShotsFired", &m_shotsFired );
	pStats->QueryIntAttribute( "Hits", &m_hits );
	pStats->QueryIntAttribute( "ObjectsDestroyed", &m_objectsDestroyed );
	pStats->QueryIntAttribute( "ObjectsHitTarget", &m_objectsHitTarget );
	pStats->QueryIntAttribute( "GamesStarted", &m_gamesStarted );
	pStats->QueryIntAttribute( "LevelsWon", &m_levelsWon );
	pStats->QueryIntAttribute( "LivesSpent", &m_livesSpent );
	pStats->QueryIntAttribute( "GamesWon", &m_gamesWon );
	pStats->QueryIntAttribute( "TimePlayed", &m_timePlayed );*/

	TiXmlElement *pAchs = pRoot->FirstChildElement("Achievements");
	TiXmlElement *pAch = 0;
	while( pAch = pAch ? pAch->NextSiblingElement("Achievement") : pAchs->FirstChildElement("Achievement") )
	{
		std::string ID;
		if ( pAch->QueryValueAttribute( "ID", &ID ) != TIXML_SUCCESS )
			continue;

		m_achievements.insert( ID );
	}
}

void PlayerProfile::Save()
{
	TiXmlDocument doc;
	TiXmlElement root( "Player" );
	
	TiXmlElement stats( "Stats" );
	stats.SetAttribute( "MaxLevelUnlocked", m_maxUnlockedLevel );
	/*stats.SetAttribute( "ShotsFired", m_shotsFired );
	stats.SetAttribute( "Hits", m_hits );
	stats.SetAttribute( "ObjectsDestroyed", m_objectsDestroyed );
	stats.SetAttribute( "ObjectsHitTarget", m_objectsHitTarget );
	stats.SetAttribute( "GamesStarted", m_gamesStarted );
	stats.SetAttribute( "LevelsWon", m_levelsWon );
	stats.SetAttribute( "LivesSpent", m_livesSpent );
	stats.SetAttribute( "GamesWon", m_gamesWon );
	stats.SetAttribute( "TimePlayed", m_timePlayed );*/
	root.InsertEndChild( stats );	

	TiXmlElement achs( "Achievements" );
	for ( Achievements::const_iterator iter = m_achievements.begin();
	      iter != m_achievements.end();
		  ++iter )
	{
		TiXmlElement ach("Achievement");
		ach.SetAttribute( "ID", iter->c_str() );
		achs.InsertEndChild( ach );
	}
	root.InsertEndChild( achs );

	doc.InsertEndChild( root );

	std::string path = GetSystem().GetWriteablePath();
	if ( !path.empty() && *path.rbegin() != '/' && *path.rbegin() != '\\' )
		path += "/";
	path += "player.xml";

	doc.SaveFile( path.c_str() );
}

void PlayerProfile::ResetGameStats()
{
	m_unkillableStreak = 0;
}

void PlayerProfile::ResetLevelStats()
{
	m_levelShotsFired = 0;
	m_levelHits = 0;
	m_levelTargetMisses = 0;
	m_levelTargetHits = 0;
	memset( m_levelKillsByType, 0, sizeof( m_levelKillsByType ) );
	memset( m_levelTargetHitsByType, 0, sizeof( m_levelTargetHitsByType ) );
	m_levelLastTargetHitType = -1;
	m_levelTimedKills.clear();
	m_maxLevelTimedKills = 0;
	m_levelTime = 0;
	m_levelLivesLost = 0;
	m_highFlyerTimer = 0;
}
