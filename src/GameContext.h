#pragma once

#include "ComponentID.h"

class IRender;
class ObjectsStorage;
struct PlayerProfile;

/*
	Game states enum. Add new states here if needed.
*/
enum EGameState
{
	GAMESTATE_INVALID = -1,
	GAMESTATE_INTRO,
	GAMESTATE_MENU,
	GAMESTATE_NEXT_LEVEL,
	GAMESTATE_TUTORIAL,
	GAMESTATE_LEVEL,
	GAMESTATE_AFTER_LEVEL,
	GAMESTATE_GAME_OVER_LIVES,
	GAMESTATE_GAME_OVER_TIME,
	GAMESTATE_WIN,
	GAMESTATE_SCORES,
	GAMESTATE_PAUSE,

	MAX_GAME_STATES
};

/*
	Object direction for AI
*/
enum EDirection
{
	LEFT_TO_RIGHT = 1,
	RIGHT_TO_LEFT = 2,
};

/*
	Object creation request identifier
*/
class RequestID
{
	int m_id;
public:
	RequestID()
		: m_id( 0 )
	{}

	RequestID( int id )
		: m_id( id )
	{}

	operator int() const { return m_id; }

	bool IsValid() const { return m_id != 0; }
	void Invalidate() { m_id = 0; }
};

/*
	Request for object creation
*/
struct SCreateObjectRequest
{
	struct SData
	{
		SData()
			: x( 0 )
			, y( 0 )
			, dir( LEFT_TO_RIGHT )
			, angle( 0 )
			, speedX( 0 )
			, speedY( 0 )
			, hasSpeed( false )
		{}

		int x,y;
		EDirection dir;
		float angle;
		float speedX;
		float speedY;
		bool hasSpeed;
		std::string ai;
	};

	SCreateObjectRequest()
	{}

	SCreateObjectRequest( const RequestID & id, const std::string & proto, const SCreateObjectRequest::SData & data, GameObjectID replaceID )
		: m_id( id )
		, m_replaceID( replaceID )
		, m_proto( proto )
		, m_data( data )
	{}

	RequestID m_id;
	GameObjectID m_replaceID;
	std::string m_proto;
	SData m_data;

	GameObjectID m_resultID;
};

/*
	Object destruction request	
*/
struct SDestroyObjectRequest
{
	SDestroyObjectRequest()
	{}

	SDestroyObjectRequest( const RequestID & id, const GameObjectID & objectID )
		: m_id( id )
		, m_objectID( objectID )
	{}

	RequestID m_id;
	GameObjectID m_objectID;
};

/*
	Functor interface for enumerating loaded levels via IGameContext
*/
class ILevelsEnumerator
{
public:
	virtual void operator()( const char *level, const char *object, int index, bool locked ) = 0;
};

class LevelDirector;

/*
	This interface is passed to all states and other objects.
	It allows some operations on Game class and it's aggregates.
*/
class IGameContext
{
public:
	virtual ~IGameContext(){}
   
	  // State "stack" operations
	virtual void PushState( EGameState state ) = 0;
	virtual void RemoveState( EGameState state ) = 0;
  
	  // Forces exit from game
	virtual void ExitGame() = 0;

	  // Returns LevelDirector for further operations
	virtual LevelDirector & GetLevelDirector() = 0;
	virtual const LevelDirector & GetLevelDirector() const = 0;
	  // Uses passed interface to enumerate all loaded levels
	virtual void EnumerateLevels( ILevelsEnumerator & enumerator ) = 0;

	  // Allows operations with objects storage
	virtual ObjectsStorage & GetOjectsStorage() = 0;
	virtual const ObjectsStorage & GetOjectsStorage() const = 0;

	  // Allows object creation and destruction. Only object creation requests return results
	virtual const RequestID & CreateGameObject( const std::string & proto, const SCreateObjectRequest::SData & data, GameObjectID replaceID = GameObjectID() ) = 0;
	virtual GameObjectID GetCreateRequestResult( const RequestID & requestID ) const = 0;
	virtual void DestroyGameObject( const GameObjectID & id ) = 0;
	virtual void DestroyAllObjects() = 0;

	  // Returns true if current level is the last one
	virtual bool IsLastLevel() = 0;
	  // Loads next level
	virtual bool LoadNextLevel() = 0;
	  // Returns current level index
	virtual int GetCurrentLevel() const = 0;

	  // Updates list of unlocked levels based on current one
	virtual void UpdateUnlockedLevel() = 0;

	  // (Re)starts game from specified level
	virtual void RestartGame( int level = 1 ) = 0;

	  // Operations with day time and days
	virtual void UpdateDayTime( float dtSec ) = 0;
	virtual float GetDayTime() const = 0;
	virtual int GetDaysLeft() const = 0;

	  // Allows creation of separate graphic components
	virtual GraphicCompID CreateGraphCompImmediate( const std::string & proto = "" ) = 0;
	virtual void RemoveGraphCompImmediate( const GraphicCompID & id ) = 0;

	  // Allows operations with player's profile
	virtual PlayerProfile & GetPlayerProfile() = 0;
	virtual const PlayerProfile & GetPlayerProfile() const = 0;

	  // A bit of a hack to get object's picture for UI
	virtual const char *GetObjectPictureForMenu( const char *objectName ) = 0;

	  // Check if god mode is on
	virtual bool IsGodMode() const = 0;
	virtual bool IsDebugAchMode() const = 0;
};
