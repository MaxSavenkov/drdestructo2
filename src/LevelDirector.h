#pragma once

#include "ComponentID.h"
#include "GameContext.h"
#include "TextManager.h"
#include "IRender.h"
#include "ReflectionController.h"
#include "GroundController.h"
#include "SkyController.h"
#include "Common.h"

class IGameContext;

typedef std::vector<GameObjectID> ObjectIDVector;

  // AI Scheme with weight for randomization purposes
struct SAISchemePreset
{
	std::string m_aiName;
	int m_weight;
};

  // Describes a spatial interval in which enemy may be spawned
  // and AI schemes it may use.
struct SAIInterval
{
	SAIInterval()
		: m_schemesWeight( 0 )
		, x1( 0 )
		, x2( 0 )
		, y1( 0 )
		, y2( 0 )
	{}

	int x1, y1;
	int x2, y2;
	EDirection dir;

	typedef std::vector<SAISchemePreset> SchemeVector;
	SchemeVector m_schemes;

	int m_schemesWeight;

	void AddScheme( const SAISchemePreset & scheme )
	{
		m_schemes.push_back( scheme );
		m_schemesWeight += scheme.m_weight;
	}

	bool SetRegion( const std::string & region )
	{
		if ( region == "LEFT" )
		{
			x1 = -49;
			x2 = -49;
			y1 = 50;
			y2 = 400;
		}
		else if ( region == "LEFT_NARROW" )
		{
			x1 = -49;
			x2 = -49;
			y1 = 150;
			y2 = 300;
		}
		else if ( region == "RIGHT" )
		{
			x1 = SCREEN_W + 49;
			x2 = SCREEN_W + 49;
			y1 = 50;
			y2 = 400;
		}
		else if ( region == "RIGHT_NARROW" )
		{
			x1 = SCREEN_W + 49;
			x2 = SCREEN_W + 49;
			y1 = 150;
			y2 = 300;
		}
		else if ( region == "TOP" )
		{
			x1 = 50;
			x2 = SCREEN_W - 50;
			y1 = -49;
			y2 = -49;
		}
		else if ( region == "TOP_NARROW" )
		{
			x1 = 300;
			x2 = SCREEN_W - 300;
			y1 = -49;
			y2 = -49;
		}
		else if ( region == "INNER" )
		{
			x1 = 50;
			x2 = SCREEN_W - 50;
			y1 = 50;
			y2 = 400;
		}

		return true;
	}
};

  // Describes a set of intervals for a single object,
  // along with spawn probability and spawn cooldown (see LevelDirector.cpp)
struct SAIProto
{
	SAIProto()
		: m_spawnProb( 1.0f )
		, m_cooldownPerSecond( 0 )
	{}

	std::string m_objectProto;

	typedef std::vector<SAIInterval> IntervalVector;
	IntervalVector m_intervals;

	float m_cooldownPerSecond;
	float m_spawnProb;

	void AddInterval( const SAIInterval & in )
	{
		m_intervals.push_back( in );
	}

	bool SetCooldown( const float & seconds )
	{
		m_cooldownPerSecond = 1.0f / seconds;
		return true;
	}
};

  // Information about object that already have been spawned
struct SSpawnedObject
{
	SSpawnedObject( const GameObjectID & id, int proto )
		: m_id( id )
		, m_proto( proto )
	{}

	GameObjectID m_id;
	int m_proto;
};

struct SSpawnChance
{
	float m_spawnProb;
};

  // A request for spawning an object
struct SSpawnRequest
{
	SSpawnRequest( const RequestID & id, int proto )
		: m_id( id )
		, m_proto( proto )
	{}

	RequestID m_id;
	int m_proto;
};

  // A briefing text entry
struct STalkEntry
{
	STalkEntry()
	{}

	STalkEntry( ImageHandle & img, TextHandle & txt )
		: m_image( img )
		, m_text( txt )
	{}

	ImageHandle m_image;
	TextHandle m_text;
};

  // A PBP text entry, which is displayed with icon during level
struct SPlayByPlayEntry
{
	SPlayByPlayEntry()
	{}

	SPlayByPlayEntry( ImageHandle & img, TextHandle & txt, int percent )
		: m_image( img )
		, m_text( txt )
		, m_percent( percent )
	{}

	ImageHandle m_image;
	TextHandle m_text;
	  // Percent of level completion at which this message should be shown
	int m_percent;
};

/*
	Class that stores and manages things related to level and AIs
*/
class LevelDirector
{
	  // Current level name
	std::string m_name;

	  // Count of objects by local prototype index
	typedef std::map<int, int> CountMap;
	CountMap m_objectsCounts;

	  // Timer for unconditional pause between spawn
	float m_spawnTimeout;

	  // List of possible objects for this level
	typedef std::vector<SAIProto> AIProtoVector;
	AIProtoVector m_protos;

	  // List of spawned objects
	typedef std::vector<SSpawnedObject> SpawnedVector;
	SpawnedVector m_spawns;

	  // List of spawn chances
	typedef std::vector<SSpawnChance> SpawnChances;
	SpawnChances m_chances;

	  // List of spawn requests
	typedef std::vector<SSpawnRequest> SpawnRequests;
	SpawnRequests m_requests;

	friend class LevelAIDeserializer;
	friend struct SpawnRequestRemover;

	  // Prototype of target object (ship, castle etc.)
	std::string m_targetObjectProto;
	  // Position of target object
	int m_targetObjectX;
	int m_targetObjectY;
	  // Y-coordinate (relative) of line at which destroyed pixels
	  // count toward level completion
	int m_sinkLineY;
	  // Needed percent of pixels destroyed to complete level
	int m_sinkPercent;
	  // Initial number of pixels on "sink line"
	int m_initialSinkPixels;
	  // Current number of pixels on "sink line"
	int m_currentSinkPixels;

	  // Maximum objects at once on screen in this level
	int m_maxObjects;
	
	  // Target object ID
	GameObjectID m_targetObjectID;
	  // Request ID to create target object
	RequestID m_createTargetRequestID;

	  // Current level's briefing
	typedef std::vector<STalkEntry> BriefingVector;
	BriefingVector m_briefing;	

	  // Current level's PBP texts
	typedef std::vector<SPlayByPlayEntry> PlayByPlayVector;
	PlayByPlayVector m_playByPlay;

	  // Parameters for level visualization
	SReflectionControllerParams m_reflectionParams;
	SGroundControllerParams m_groundParams;
	SSkyControllerParams m_skyParams;

public:
	const STalkEntry *GetBriefing( int index ) const
	{
		if ( index < 0 || index >= (int)m_briefing.size() )
			return 0;

		return &m_briefing[ index ];
	}

	const SPlayByPlayEntry *GetPBP( int index ) const
	{
		if ( index < 0 || index >= (int)m_playByPlay.size() )
			return 0;

		return &m_playByPlay[ index ];
	}

	void Cheat()
	{
		//m_currentSinkPixels -= 10;
		m_currentSinkPixels = 0;
	}

public:
	const SReflectionControllerParams & GetReflectionControllerParams() const { return m_reflectionParams; }
	bool SetHasReflection() { m_reflectionParams.m_active = true; return true; }

	const SGroundControllerParams & GetGroundControllerParams() const { return m_groundParams; }
	bool SetHasGround() { m_groundParams.m_active = true; return true; }

	const SSkyControllerParams & GetSkyControllerParams() const { return m_skyParams; }
	bool SetHasSky() { m_skyParams.m_active = true; return true; }

public:
	LevelDirector();

	void Update( float dt, IGameContext & context );
	bool LoadLevel( const std::string & level );

	void Reset();

	void AddProto( const SAIProto & proto )
	{
		m_protos.push_back( proto );
	}

	void UpdateSinkPixels( int delta ) { m_currentSinkPixels += delta; }
	float GetDestroyedPixelsPercent() const { return m_initialSinkPixels > 0 ? ( 100 - m_currentSinkPixels * 100 / m_initialSinkPixels ) : 0.0f; }
	float GetCompletionPercent() const { float ret = ( m_sinkPercent > 0 ? GetDestroyedPixelsPercent() / m_sinkPercent : 0.0f ); if ( ret >= 1.0f ) return 1.0f; return ret; }
	bool CheckWinConditions() const { return GetDestroyedPixelsPercent() >= m_sinkPercent; }
	int GetSinkLineY() const { return m_sinkLineY; }
	int GetPBPIndex() const
	{
		if ( m_playByPlay.empty() || m_sinkPercent <= 0 || m_initialSinkPixels <= 0 || m_currentSinkPixels <= 0 )
			return -1;

		const int currentPercent = (100 - m_currentSinkPixels * 100 / m_initialSinkPixels) * 100 / m_sinkPercent;

		for ( int i = 0; i < (int)m_playByPlay.size(); ++i )
		{			
			if ( currentPercent < m_playByPlay[ i ].m_percent )
				return i - 1;
		}

		return (int)m_playByPlay.size() - 1;
	}

	const std::string & GetName() const { return m_name; }

	GameObjectID GetTargetObjectID() const { return m_targetObjectID; }
};
