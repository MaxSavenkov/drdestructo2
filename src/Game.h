#pragma once
#include "BaseGameState.h"
#include "ObjectsStorage.h"
#include "GraphicsProcessor.h"
#include "PhysicsProcessor.h"
#include "SoundProcessor.h"
#include "AIProcessor.h"
#include "LevelDirector.h"
#include "PlayerProfile.h"

struct SGameObjectProto;

  // This struct describes a sequence of game levels and stores current level index
struct SLevelProgression
{
	struct SLevel
	{
		SLevel( const std::string & level, const std::string & name, const std::string & object )
			: m_level( level )
			, m_name( name )
			, m_object( object )
		{}

		std::string m_level;
		std::string m_name;
		std::string m_object;
	};

	SLevelProgression()
		: m_currentLevel( -1 )
	{}

	typedef std::vector<SLevel> Levels;
	Levels m_levels;
	int m_currentLevel;

	const std::string NextLevel()
	{
		++m_currentLevel;
		if ( m_currentLevel >= (int)m_levels.size() || m_currentLevel < 0 )
			return "";

		return m_levels[ m_currentLevel ].m_level.c_str();
	}

	const std::string CurrentLevel() const
	{
		if ( m_currentLevel >= (int)m_levels.size() || m_currentLevel < 0 )
			return "";

		return m_levels[ m_currentLevel ].m_level.c_str();
	}

	void RestartGame( int level )
	{
		m_currentLevel = level;
	}

	bool AddLevel( const std::string & path )
	{
		std::vector<char> data;
		if ( !GetSystem().LoadFile( true, path.c_str(), data )) 
			return false;

		TiXmlDocument doc;
		if ( !doc.Parse( &data[0] ) )
			return false;

		TiXmlElement *pLevel = doc.FirstChildElement( "Level" );
		if ( !pLevel )
			return false;

		std::string name;
		if ( pLevel->QueryValueAttribute( "Name", &name ) != TIXML_SUCCESS )
			return false;

		std::string object;
		if ( pLevel->QueryValueAttribute( "TargetObject", &object ) != TIXML_SUCCESS )
			return false;

		m_levels.push_back( SLevel( path, name, object ) );
		return true;
	}
};

  // Initial number of game days
static const int INITIAL_DAYS = 7;

  // Main game class
class Game : public IGameContext, public InputHandler
{
	  // List of all known game states
	typedef boost::scoped_ptr<BaseGameState> StatePtr;
	StatePtr m_gameStates[ MAX_GAME_STATES ];

	  // States "stack"
	typedef std::vector<BaseGameState*> StateVector;
	StateVector	m_currentStates;
	  // List of states to add and remove
	typedef std::vector<EGameState> StateIDVector;
	StateIDVector m_pushStates;
	StateIDVector m_removeStates;

	  // Information about player which is loaded and saved to player.xml
	PlayerProfile m_playerProfile;

	  // If true, main loop should quit
	bool m_stop;

	  // If true, don't draw or do any logic (mostly for mobile version, but also to avoid drawing with DirectX display lost)
	bool m_forcedPause;

	ObjectsStorage m_objectStorage;
	GraphicsProcessor m_graphicProcessor;
	SoundProcessor m_soundProcesor;
	PhysicsProcessor m_physicProcessor;
	AIProcessor m_aiProcessor;

	  // Next request ID to use
	RequestID m_freeRequestID;

	  // List of requests for objects creation
	typedef std::vector<SCreateObjectRequest> CreateRequestVector;
	CreateRequestVector m_creates;

	  // List of results of objects creation (stored for one frame)
	typedef std::map<RequestID, GameObjectID> CreateResults;
	CreateResults m_createResults;

	  // List of requests for objects destruction
	typedef std::vector<SDestroyObjectRequest> DestroyRequestVector;
	DestroyRequestVector m_destroys;

      // See LevelDirector.h
	LevelDirector m_director;

	SLevelProgression m_levels;

	  // Current day time
	float m_dayTime;
	  // Days left before game over
	int m_daysLeft;

	bool m_showFPS;

	bool m_godMode;
	bool m_debugAchMode;

	void PushState( EGameState state );
	void RemoveState( EGameState state );
	void ExitGame() { m_stop = true; }

	RequestID NextRequestID()
	{
		int nextID = m_freeRequestID + 1;
		if ( nextID <= 0 )
			nextID = 1;
		m_freeRequestID = RequestID( nextID );
		return m_freeRequestID;
	}

	  // Initializes freshly created object from prototype and passed data
	void InitObject( GameObject & object, const SGameObjectProto & proto, const SCreateObjectRequest::SData & data, int inheritedStatType );
	void CreateObjects();
	void DestroyObjects();
	void DestroyAllObjects();

	GraphicCompID CreateGraphicComponent( const GameObjectID & parent, const std::string & proto );
	PhysicCompID CreatePhysicComponent( const GameObjectID & parent, const std::string & proto );
	ControlCompID CreateControlComponent( const GameObjectID & parent, const std::string & proto );
	MechanicCompID CreateMechanicComponent( const GameObjectID & parent, const std::string & proto );
	SoundCompID CreateSoundComponent( const GameObjectID & parent, const std::string & proto );

	ObjectsStorage & GetOjectsStorage() { return m_objectStorage; }
	const ObjectsStorage & GetOjectsStorage() const { return m_objectStorage; }

	const RequestID & CreateGameObject( const std::string & proto, const SCreateObjectRequest::SData & data, GameObjectID replaceID = GameObjectID() );
	GameObjectID GetCreateRequestResult( const RequestID & requestID ) const;
	void DestroyGameObject( const GameObjectID & id );

	bool LoadNextLevel();
	int GetCurrentLevel() const { return m_levels.m_currentLevel; }
	bool IsLastLevel() { return m_levels.m_currentLevel >= 0 ? m_levels.m_currentLevel >= (int)m_levels.m_levels.size() - 1 : false; }
	void UpdateUnlockedLevel();
	LevelDirector & GetLevelDirector() { return m_director; }
	const LevelDirector & GetLevelDirector() const { return m_director; }
	void EnumerateLevels( ILevelsEnumerator & enumerator );

	PlayerProfile & GetPlayerProfile() { return m_playerProfile; }
	const PlayerProfile & GetPlayerProfile() const { return m_playerProfile; }

	const char *GetObjectPictureForMenu( const char *objectName ) ;

	bool IsGodMode() const { return m_godMode; }
	bool IsDebugAchMode() const { return m_debugAchMode; }

	GraphicCompID CreateGraphCompImmediate(  const std::string & proto );
	void RemoveGraphCompImmediate( const GraphicCompID & id );

	void UpdateDayTime( float dtSec );
	float GetDayTime() const;
	int GetDaysLeft() const;

	void RestartGame( int level );

	  // If true, current frame rate is artifically limited (only makes sense when VSync is off)
	bool m_frameLimit;

	bool ToggleFrameLimit( const InputEvent & ev )
	{
		m_frameLimit = !m_frameLimit;
		return true;
	}

	bool ToggleFPS( const InputEvent & ev )
	{
		m_showFPS = !m_showFPS;
		return true;
	}

	  // Shows where devide between "back" and "front" parts of water are
	bool ToggleDebugReflection( const InputEvent & ev );
	  // Does something you have coded into it. Only works in debug mode
	bool Cheat( const InputEvent & ev );
	bool GodMode( const InputEvent & ev );
	bool DebugAch( const InputEvent & ev );

public:
	Game();
	void Run();
};
