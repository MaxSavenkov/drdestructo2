#pragma once

#include "Common.h"

/*
	Player's statistics, achievements and temporary data.
	Some of it gets saved
*/
struct PlayerProfile
{
	int m_maxUnlockedLevel;
	  // Statistic
	/*int m_shotsFired;
	int m_hits;
	int m_objectsDestroyed;
	int m_objectsHitTarget;
	int m_gamesStarted;
	int m_levelsWon;
	int m_gamesWon;
	int m_livesSpent;
	int m_timePlayed;*/
	  // Game type (not saved)
	bool m_hardcore;
	int m_unkillableStreak;
	  // Level statistic (not saved)
	int m_levelShotsFired;
	int m_levelHits;
	int m_levelTargetMisses;
	int m_levelTargetHits;
	int m_levelKillsByType[ STAT_TYPES_COUNT ];
	int m_levelTargetHitsByType[ STAT_TYPES_COUNT ];
	int m_levelLastTargetHitType;
	std::deque<float> m_levelTimedKills;
	int m_maxLevelTimedKills;
	float m_levelTime;
	int m_levelLivesLost;
	float m_highFlyerTimer;
	  // Achievements
	typedef std::set<std::string> Achievements;
	Achievements m_achievements;

	PlayerProfile();

	void Load();
	void Save();
	void ResetGameStats();
	void ResetLevelStats();
};
