#pragma once

static const int ACH_GUID_LEN = 37;
static const int ACH_NAME_LEN = 32;
static const int ACH_DESC_LEN = 128;
static const int ACH_ICON_LEN = 32;

struct PlayerProfile;
class IGameContext;

  // This is the base interface for classes that
  // check achievements' conditions.
class IAchievChecker
{
public:
	virtual ~IAchievChecker() {}
	virtual bool operator()( const PlayerProfile & prof, const IGameContext & context ) const = 0;
};

  // This structure describes a single achievement
struct SAchievement
{
	char m_guid[ ACH_GUID_LEN ];
	char m_name[ ACH_NAME_LEN ];
	char m_desc[ ACH_DESC_LEN ];
	char m_icon[ ACH_ICON_LEN ];
	IAchievChecker *m_checkFunc;

	~SAchievement()
	{
		delete m_checkFunc;
	}

	bool Check( const PlayerProfile & prof, const IGameContext & context ) const
	{
		if ( !m_checkFunc )
			return false;

		return (*m_checkFunc)( prof, context );
	}
};

class Achievements
{
	SAchievement *m_achievements;
	int m_achievementsCount;

public:
	Achievements();
	const SAchievement* GetAchievement( int index ) const;
	int GetAchievementCount() const;
};

const Achievements & GetAchievements();