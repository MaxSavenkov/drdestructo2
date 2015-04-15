#include "stdafx.h"
#include "Game.h"
#include "IRender.h"
#include "ISystem.h"
#include "GameStateLevel.h"
#include "GameStateWin.h"
#include "GameStateGameOverLives.h"
#include "GameStateGameOverTime.h"
#include "GameStateNextLevel.h"
#include "GameStateMenu.h"
#include "GameStateWinGame.h"
#include "GameStateIntro.h"
#include "GameStateScoreTable.h"
#include "GameStatePause.h"
#include "GameStateTutorial.h"
#include <math.h>
#include "Vars.h"
#include "ObjectProtoManager.h"
#include "GraphicsProtoManager.h"
#include "PhysicsProtoManager.h"
#include "ControlsProtoManager.h"
#include "MechanicsProtoManager.h"
#include "SoundProtoManager.h"
#include "AIProtoManager.h"
#include "XMLDeserializer.h"
#include "Common.h"
#include <ios>

#include "AICommands.h"

#ifndef WIN32
    #include <sys/time.h>
    #define _sleep sleep
#endif

//#define PROFILE

namespace
{
	UseIntVar PHYS_CAPS_FOLLOW_ANGLE("PHYS_CAPS_FOLLOW_ANGLE");
	RegisterIntVar GRAPH_CAPS_CLONE("GRAPH_CAPS_CLONE");
	UseVariable<bool> DebugReflectionLine("DebugReflectionLine");
}

class LevelListDes : public RootXMLDeserializer
{	
	class LevelDeserializer : public XMLDataDeserializer
	{
		SLevelProgression & m_levels;

	public:
		LevelDeserializer( SLevelProgression & levels )
			: XMLDataDeserializer( "Level", false )
			, m_levels( levels )
		{
			Attrib_SetterValue<SLevelProgression, std::string>( "Path", false, m_levels, &SLevelProgression::AddLevel );
		}
	};

	int & m_firstLevel;
public:
	LevelDeserializer m_levelDes;

	LevelListDes( SLevelProgression & levels, int & firstLevel )
		: RootXMLDeserializer( "Levels" )
		, m_firstLevel( firstLevel )
		, m_levelDes( levels )
	{
		Attrib_Value( "FirstLevel", false, m_firstLevel );
		SubDeserializer( m_levelDes); 
	}
};

  // Define this if you want to get a separate console window under Windows for printfs
#ifdef _DEBUG_CONSOLE
void RedirectIOToConsole()
{
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;
	// allocate a console for this app
	AllocConsole();
	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
		&coninfo);
	coninfo.dwSize.Y = 500;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
		coninfo.dwSize);
	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );
	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	std::ios::sync_with_stdio();
}
#endif

Game::Game()
{
#if defined(_WINDOWS) && defined(_DEBUG_CONSOLE)
	RedirectIOToConsole();
#endif
	printf("Game started\n");
	m_stop = false;
	m_forcedPause = false;
	memset( m_gameStates, 0, sizeof(m_gameStates) );

	m_showFPS = false;
	m_frameLimit = true;
	m_godMode = false;
	m_debugAchMode = false;

	  // Initialize list of states. Add any new states here before usage!
	m_gameStates[ GAMESTATE_INTRO ].reset( new GameStateIntro() );
	m_gameStates[ GAMESTATE_MENU ].reset( new GameStateMenu() );
	m_gameStates[ GAMESTATE_NEXT_LEVEL ].reset( new GameStateNextLevel() );
	m_gameStates[ GAMESTATE_LEVEL ].reset( new GameStateLevel() );
	m_gameStates[ GAMESTATE_AFTER_LEVEL ].reset( new GameStateWin() );
	m_gameStates[ GAMESTATE_GAME_OVER_LIVES ].reset( new GameStateGameOverLives() );
	m_gameStates[ GAMESTATE_GAME_OVER_TIME ].reset( new GameStateGameOverTime() );
	m_gameStates[ GAMESTATE_WIN ].reset( new GameStateWinGame() );
	m_gameStates[ GAMESTATE_SCORES ].reset( new GameStateScoreTable() );
	m_gameStates[ GAMESTATE_PAUSE ].reset( new GameStatePause() );
	m_gameStates[ GAMESTATE_TUTORIAL ].reset( new GameStateTutorial() );

	  // Set the initial state
	PushState( GAMESTATE_INTRO );

	  // Register base key handlers
	AddKeyboardHandler( this, &Game::ToggleFrameLimit, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_F12 );
	AddKeyboardHandler( this, &Game::ToggleFPS, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_F11 );
#ifdef _DEBUG
	AddKeyboardHandler( this, &Game::ToggleDebugReflection, KeyboardEvent::KBD_KEY_UP, ALLEGRO_KEY_F1 );
	AddKeyboardHandler( this, &Game::Cheat, KeyboardEvent::KBD_KEY_REPEAT, ALLEGRO_KEY_F2 );
	AddKeyboardHandler( this, &Game::GodMode, KeyboardEvent::KBD_KEY_REPEAT, ALLEGRO_KEY_F3 );
	AddKeyboardHandler( this, &Game::DebugAch, KeyboardEvent::KBD_KEY_REPEAT, ALLEGRO_KEY_F4 );	
#endif
	m_playerProfile.Load();

	m_levels.m_levels.clear();
	LevelListDes root( m_levels, m_levels.m_currentLevel );
	XMLDeserializer des( root );
	des.Deserialize( "Data/Levels/levels_list.xml" );
}

void Game::Run()
{
	  // Load resources
	GetVars().Load("Data/Protos/vars.xml");
	LoadGameObjects();
	LoadGraphics();
	LoadPhysics();
	LoadControls();
	LoadMechanics();
	LoadSounds();
	m_aiProcessor.RegisterFunctions( GetAI().GetBaseState() );
	LoadAI();	
	m_objectStorage.GetMusic().Load( "Data/Music/music.xml" );

	IInput & input = GetInput();
	IRender & render = GetRender();
	ISystem & system = GetSystem();

	// Testing code for graphics

	//GraphicCompID id =  CreateGraphicComponent( GameObjectID(0), "prop_plane_green" );
	//PhysicCompID pid =  CreatePhysicComponent( GameObjectID(0), "prop_plane" );
	//GraphicComponent *pC = m_objectStorage.GetGraphicComponent( id );
	//PhysicComponent *pPC = m_objectStorage.GetPhysicComponent( pid );
	//pC->m_currentAnim = 0;
	//pC->m_nextAnim = 0;
	//pC->m_x = 100; pC->m_y = 100;
	//pC->m_angle = 0;
	//pPC->m_x = 100; pPC->m_y = 100;
	//double t = 0.0f;
	//while( true )
	//{
	//	render.ClearToColor( Color(255,255,255) );
	//	m_graphicProcessor.Render( render, *pC );	
	//	m_graphicProcessor.Update( 0.01, *pC );
	//	pC->m_y = 100 + sinf( t * ALLEGRO_PI * 2.0f ) * 100;
	//	input.Update( 100 );
	//	
	//	InputEvent e;
	//	while( true )
	//	{
	//		e = input.GetEvent();
	//		if ( e.m_type == InputEvent::EV_INVALID )
	//			break;
	//		if ( e.m_type == InputEvent::EV_KEYBOARD && e.m_keyboardEvent.m_type == KeyboardEvent::KBD_KEY_UP && e.m_keyboardEvent.m_keycode == ALLEGRO_KEY_ENTER )
	//			pC->m_currentAnim++;
	//		input.PopEvent();
	//	}
	//	//pC->m_animations[ pC->m_currentAnim ].m_currentFrame = 1;
	//	t += 0.001;
	//	//for ( int i = 0; i < pPC->m_collisionSpheres.size(); ++i )
	//	//{
	//	//	const int x = 100 + pPC->m_collisionSpheres[ i ].m_dx;
	//	//	const int y = 100 + pPC->m_collisionSpheres[ i ].m_dy;
	//	//	const int r = pPC->m_collisionSpheres[ i ].m_r;
	//	//	render.DrawCircleFill( x, y, r, Color(255,0,255) );
	//	//}
	//	render.Flip();
	//	//input.WaitAnyKey();
	//}
	//

	// Testing code for quickly viewing PBP texts

	//class FileIterator : public IFileIterator
	//{
	//	PlayByPlay pbp;
	//public:
	//	FileIterator()
	//	{
	//		pbp.Init( GetRender() );
	//	}

	//	virtual void operator()( const char *filename, bool isDir )
	//	{
	//		std::string n = "Data/Texts/";
	//		n += filename;
	//		n +=".txt";

	//		std::vector<char> data;
	//		GetSystem().LoadFile( n.c_str(), data );
	//		UTFString str;
	//		str->Assign( &data[0], data.size() );
	//		pbp.SetDebugText( str );
	//		pbp.DebugRender();
	//		GetRender().DrawText( 0, 0, Color(255,255,255), 20, "%s", filename );
	//		GetRender().Flip();
	//		GetInput().WaitAnyKey();		
	//	}
	//}fileit;

	//system.IterateFiles( "Data/Texts/", "level*pbp.txt", false, false, fileit );

	double timePrev = system.GetTime();
	double timeNow = timePrev;

	float fpsCounterDelta = 0.0f;
	int fps = 0;
	int lastFps = 0;

	float timeProf = system.GetTime();
#ifdef PROFILE
float prof1, prof2, prof3, prof4, prof5, prof6;
float prof31, prof32, prof33, prof34, prof35, prof36;
#define PROF(x) {float timeProfNow = system.GetTime(); prof##x = timeProfNow - timeProf; timeProf = timeProfNow;}
#else
	#define PROF(x)
#endif
	//Profiler & profiler = GetProfiler();
	
	while( !m_stop )
	{
timeProf = system.GetTime();
		//------------ Timers & FPS --------------------

		//profiler.Start();
		timeNow = system.GetTime();
		float dt = timeNow - timePrev;
		timePrev = timeNow;
#ifdef PROFILE
		if ( dt > 0.02f )
		{
			printf("%f\n", dt);
			printf("prof1 %f\n", prof1);
			printf("prof2 %f\n", prof2);
			printf("prof3 %f\n", prof3);
				printf("prof31 %f\n", prof31);
				printf("prof32 %f\n", prof32);
				printf("prof33 %f\n", prof33);
				printf("prof34 %f\n", prof34);
				printf("prof35 %f\n", prof35);
				printf("prof36 %f\n", prof36);
			printf("prof4 %f\n", prof4);
			printf("prof5 %f\n", prof5);
			printf("prof6 %f\n", prof6);
		}
#endif
		if ( !m_forcedPause )
		{
			fpsCounterDelta += dt;
			if ( fpsCounterDelta >= 1.0f )
			{
				fpsCounterDelta -= 1.0f;
				lastFps = fps;
				fps = 0;
			}

	PROF(1);
			//------------ Input --------------------
		
			{
			input.Update( dt * 1000 );

			InputHandler::ProcessInput( input );

			for( StateVector::reverse_iterator iter = m_currentStates.rbegin(); iter != m_currentStates.rend(); ++iter )
				(*iter)->ProcessInput( input );
		
			input.Clear();
			}
	PROF(2);
			//------------ Render --------------------		
			{

			for( StateVector::iterator iter = m_currentStates.begin(); iter != m_currentStates.end(); ++iter )
				(*iter)->RenderBefore( render, *this );
	PROF(31);
			GraphicsContainer & graphics = m_objectStorage.GetGraphics();
			const GraphicsContainer::RenderLayerMap & layers = graphics.GetLayers();

			for ( GraphicsContainer::RenderLayerMap::const_iterator layerIter = layers.begin(); layerIter != layers.end(); ++layerIter )
			{
				for ( GraphicsContainer::GraphCompIDVector::const_iterator idIter = layerIter->second.begin(); idIter != layerIter->second.end(); ++idIter )
				{
				
					if ( m_currentStates.back()->AllowGraphicsUpdate() )
						m_graphicProcessor.Update( dt, *m_objectStorage.GetGraphicComponent( *idIter ) );
					if ( m_currentStates.back()->AllowGraphicsRender() )
						m_graphicProcessor.Render( render, *m_objectStorage.GetGraphicComponent( *idIter ) );
				}
			}
	PROF(32);
			ObjectsStorage::SoundsContainer & sounds = m_objectStorage.GetSounds();
			for ( ObjectsStorage::SoundsContainer::Iterator iter = sounds.GetIterator(); !iter.AtEnd(); ++iter )
			{
				m_soundProcesor.Update( dt, **iter );
			}
			}
	PROF(33);
			//profiler.Mark( "Objects" );		
			{
			for( StateVector::iterator iter = m_currentStates.begin(); iter != m_currentStates.end(); ++iter )
				(*iter)->RenderAfter( render, *this );
	PROF(34);
			for ( StateIDVector::iterator iter = m_removeStates.begin(); iter != m_removeStates.end(); ++iter )
			{
				while( !m_currentStates.empty() )		
				{
					const BaseGameState *pState = m_currentStates.back();
					EGameState stateID = pState->GetType();
					m_currentStates.back()->OnRemove( *this );
					m_currentStates.pop_back();
					if ( stateID == *iter )
						break;
				}
			}
			m_removeStates.clear();
	PROF(35);
			for ( StateIDVector::iterator iter = m_pushStates.begin(); iter != m_pushStates.end(); ++iter )
			{
				m_currentStates.push_back( m_gameStates[ (*iter) ].get() );
				m_currentStates.back()->OnPush( *this );
			}
			m_pushStates.clear();

			if ( m_currentStates.empty() )
				break;

			m_objectStorage.GetMusic().Update( dt );

			if ( m_showFPS )
				render.DrawText( 0, 0, Color(255,255,255), 12, "FPS: %i (%s)", lastFps, m_frameLimit ? "frame limit" : "no limit" );
	PROF(36);
	PROF(3);
			//------------ Update --------------------

			m_objectStorage.GetCollisions().Reset();

			if ( m_currentStates.back()->AllowPhysicsUpdate() )
			{
				for ( CompList<PhysicComponent>::Iterator iter = m_objectStorage.GetPhysics().GetIterator(); !iter.AtEnd(); ++iter )
					m_physicProcessor.Update( dt, **iter, m_objectStorage.GetCollisions() );

				m_physicProcessor.CalculateCollisions( m_objectStorage.GetPhysics(), m_objectStorage.GetCollisions() );
			}

			if ( m_currentStates.back()->AllowAIUpdate() )
			{
				for ( CompList<GameObject>::Iterator iter = m_objectStorage.GetGameObjects().GetIterator(); !iter.AtEnd(); ++iter )
				{
					GameObject & object = **iter;
					ControlComponent *pControls = m_objectStorage.GetControlComponent( object.GetControlComponent() );
					if ( pControls && pControls->m_type == ControlComponent::TYPE_AI )
						m_aiProcessor.Update( dt, *pControls, *this );
				}
			}

			m_currentStates.back()->Update( dt, *this );

			CreateObjects();
			DestroyObjects();

			m_objectStorage.Update();
			}
		}
		//------------ Flip & profiler --------------------
PROF(4);
		{
		//const int modified = GetBitmapManager().CountModified();
		//render.DrawText( 0, 40, Color(255,255,255), 20, "Mod: %i", modified );
		//render.DrawText( 0, 0, Color(255,255,255), 10, "CHEATING ENABLED. PRESS F2 TO WIN LEVEL, F3 TO TOGGLE GOD MODE (GOD MODE IS %s) F4 ALL ACHIEVS", m_godMode ? "on" : "off" );
		render.ProcessEvents();
		DisplayEvent dEv;
		while( true )
		{
			dEv = render.GetEvent();
			if ( dEv.m_type == DisplayEvent::DISPLAY_INVALID )
				break;

			switch( dEv.m_type )
			{
				case DisplayEvent::DISPLAY_CLOSE:
					m_stop = true;
					break;

				case DisplayEvent::DISPLAY_HALT_DRAWING:
					al_acknowledge_drawing_halt( al_get_current_display() );
				case DisplayEvent::DISPLAY_LOST:
					m_forcedPause = true;
					break;

				case DisplayEvent::DISPLAY_RESUME_DRAWING:
					al_acknowledge_drawing_resume( al_get_current_display() );
					m_forcedPause = false;
					break;

				case DisplayEvent::DISPLAY_FOUND:
					for( StateVector::iterator iter = m_currentStates.begin(); iter != m_currentStates.end(); ++iter )
						(*iter)->OnDisplayFound(*this);
					m_forcedPause = false;
					break;
			}
		}

		if ( !m_forcedPause )
			render.Flip();
		}
PROF(5);
		//------------ Sleep --------------------

		timeNow = system.GetTime();

		const bool vsync = system.HasConfigValue( "DD_Graphics", "VSync" ) ? system.GetConfigValue_Int( "DD_Graphics", "VSync" ) : false;
		if ( !vsync || m_forcedPause )
		{
			float cycleTime = timeNow - timePrev;
			if ( cycleTime < 1 / 100.0f )
			{
				if ( m_frameLimit )
					GetSystem().Sleep( (1.0f / 100.0f - cycleTime) );
			}
		}

		++fps;
PROF(6);
	}
}

void Game::PushState( EGameState state )
{
	if ( !m_gameStates[ state ] || std::find( m_currentStates.begin(), m_currentStates.end(), m_gameStates[ state ].get() ) != m_currentStates.end() )
		return;

	m_pushStates.push_back( state );
}

void Game::RemoveState( EGameState state )
{
	StateVector::iterator iter = std::find( m_currentStates.begin(), m_currentStates.end(), m_gameStates[ state ].get() );
	if ( iter == m_currentStates.end() )
		return;

	m_removeStates.push_back( state );
}

void Game::InitObject( GameObject & object, const SGameObjectProto & proto, const SCreateObjectRequest::SData & data, int inheritedStatType )
{
	PhysicComponent *pPhys = m_objectStorage.GetPhysicComponent( object.GetPhysicComponent() );
	if ( pPhys )
	{
		pPhys->m_direction = data.dir;
		pPhys->m_x = data.x;
		pPhys->m_y = data.y;
		pPhys->m_angle = data.angle;
		if ( data.hasSpeed )
		{
			pPhys->m_speedX = data.speedX;
			pPhys->m_speedY = data.speedY;
		}
		else
		{
			if ( pPhys->m_caps & PHYS_CAPS_FOLLOW_ANGLE )
			{
				pPhys->m_speed = pPhys->m_minSpeed;
			}
			else
			{
				pPhys->m_speedX = pPhys->m_minSpeed * ( data.dir == LEFT_TO_RIGHT ? 1 : -1 );
			}
		}
	}

	ControlComponent *pCtrl = m_objectStorage.GetControlComponent( object.GetControlComponent() );
	if ( pCtrl )
	{
		pCtrl->m_direction = data.dir;
		if ( pCtrl->m_type == ControlComponent::TYPE_AI && !data.ai.empty() )
		{
			pCtrl->m_aiData.m_x = data.x;
			pCtrl->m_aiData.m_y = data.y;
			pCtrl->m_pState = GetAI().CreateAIState();
			pCtrl->m_function = data.ai;
		}
	}

	GraphicComponent *pGr = m_objectStorage.GetGraphicComponent( object.GetGraphicComponent() );
	if ( pGr )
	{
		pGr->m_x = data.x;
		pGr->m_y = data.y;
		pGr->m_angle = data.angle;
	}

	MechanicComponent *pMe = m_objectStorage.GetMechanicComponent( object.GetMechanicComponent() );
	if ( pMe )
	{
		if ( pMe->m_statType < 0 )
			pMe->m_statType = inheritedStatType;
	}
}

GraphicCompID Game::CreateGraphCompImmediate( const std::string & proto )
{
	if ( !proto.empty() )
		return CreateGraphicComponent( -1, proto );
	else
		return m_objectStorage.CreateGraphicComponent()->GetID();
}

void Game::RemoveGraphCompImmediate( const GraphicCompID & id )
{
	m_objectStorage.GetGraphics().Remove( id );
}

void Game::CreateObjects()
{
	m_createResults.clear();

	for ( CreateRequestVector::const_iterator iter = m_creates.begin(); iter != m_creates.end(); ++iter )
	{
		const SCreateObjectRequest & req = *iter;

		const SGameObjectProto *pProto = GetGameObjects().GetResource( req.m_proto );
		if ( !pProto )
			continue;

		GameObject *obj = 0;

		int mechStatType = -1;

		if ( req.m_replaceID.IsValid() )
		{
			obj = m_objectStorage.GetGameObject( req.m_replaceID );
			if ( !obj )
				continue;

			const MechanicComponent *pMechComp = m_objectStorage.GetMechanics().Get( obj->GetMechanicComponent() );
			if ( pMechComp )
				mechStatType = pMechComp->m_statType;

			m_objectStorage.GetGraphics().Remove( obj->GetGraphicComponent() );
			m_objectStorage.GetPhysics().Remove( obj->GetPhysicComponent() );
			m_objectStorage.GetControls().Remove( obj->GetControlComponent() );
			m_objectStorage.GetMechanics().Remove( obj->GetMechanicComponent() );
		}
		else
		{
			obj = m_objectStorage.CreateGameObject();
		}

		if ( !pProto->m_graphics.empty() )
			obj->SetGraphicComponent( CreateGraphicComponent( obj->GetID(), pProto->m_graphics ) );

		if ( !pProto->m_physics.empty() )
			obj->SetPhysicComponent( CreatePhysicComponent( obj->GetID(), pProto->m_physics ) );

		if ( !pProto->m_controls.empty() )
			obj->SetControlComponent( CreateControlComponent( obj->GetID(), pProto->m_controls ) );

		if ( !pProto->m_mechanics.empty() )
			obj->SetMechanicComponent( CreateMechanicComponent( obj->GetID(), pProto->m_mechanics ) );

		if ( !pProto->m_sounds.empty() )
			obj->SetSoundComponent( CreateSoundComponent( obj->GetID(), pProto->m_sounds ) );

		InitObject( *obj, *pProto, req.m_data, mechStatType );

		m_createResults.insert( std::make_pair( req.m_id,obj->GetID() ) );
	}

	m_creates.clear();
}

void Game::DestroyObjects()
{
	for ( DestroyRequestVector::const_iterator iter = m_destroys.begin(); iter != m_destroys.end(); ++iter )
	{
		const SDestroyObjectRequest & req = *iter;
		const GameObject *pObject = m_objectStorage.GetGameObject( req.m_objectID );
		if ( !pObject )
			continue;

		m_objectStorage.GetGraphics().Remove( pObject->GetGraphicComponent() );
		m_objectStorage.GetPhysics().Remove( pObject->GetPhysicComponent() );
		m_objectStorage.GetControls().Remove( pObject->GetControlComponent() );
		m_objectStorage.GetMechanics().Remove( pObject->GetMechanicComponent() );
		m_objectStorage.GetSounds().Remove( pObject->GetSoundComponent() );
		m_objectStorage.GetGameObjects().Remove( req.m_objectID );
	}

	m_destroys.clear();
}

GraphicCompID Game::CreateGraphicComponent( const GameObjectID & parent, const std::string & proto )
{
	const SGraphicsProto *pProto = GetGraphics().GetResource( proto );
	if ( !pProto )
		return GraphicCompID();

	GraphicComponent *comp = m_objectStorage.CreateGraphicComponent();
	comp->SetParent( parent );
	
	comp->m_type = pProto->m_type;
	comp->m_caps = pProto->m_caps;
	if ( comp->m_caps & GRAPH_CAPS_CLONE )
	{
		comp->m_cache = GetRender().LoadImage( pProto->m_image.c_str() );
		comp->m_image = GetRender().CloneImage( comp->m_cache, true );
	}
	else
		comp->m_image = GetRender().LoadImage( pProto->m_image.c_str() );

	if ( !pProto->m_imageBg.empty() )
		comp->m_staticBg = GetRender().LoadImage( pProto->m_imageBg.c_str() );

	for ( SGraphicsProto::AnimProtoMap::const_iterator iter = pProto->m_animations.begin(); iter != pProto->m_animations.end(); ++iter )
	{
		const SAnimProto & animProto = iter->second;

		Animation anim;
		anim.m_fps = animProto.m_fps;
		anim.m_dir = animProto.m_dir;
		anim.m_reverse = animProto.m_reverse;
		anim.m_flipV = animProto.m_flipV;
		anim.m_flipH = animProto.m_flipH;
		anim.m_frames = animProto.m_frames;
		anim.m_oneShot = animProto.m_oneShot;
		anim.m_soundEvent = animProto.m_soundEvent;
		
		comp->m_animations[ iter->first ] = anim;
	}

	if ( !pProto->m_animations.empty() )
	{
		comp->m_nextAnim = pProto->m_animations.begin()->first;
	}
	
	comp->SetLayer( pProto->m_layerID );
	
	return comp->GetID();
}

PhysicCompID Game::CreatePhysicComponent( const GameObjectID & parent, const std::string & proto )
{
	const SPhysicsProto *pProto = GetPhysics().GetResource( proto );
	if ( !pProto )
		return PhysicCompID();

	PhysicComponent *comp = m_objectStorage.CreatePhysicComponent();
	comp->SetParent( parent );
	comp->m_speed = pProto->m_minSpeed;
	comp->m_minSpeed = pProto->m_minSpeed;
	comp->m_maxSpeed = pProto->m_maxSpeed;
	comp->m_accel = pProto->m_accel;
	comp->m_vertSpeed = pProto->m_verticalSpeed;
	comp->m_maxTurnSpeed = pProto->m_turnSpeed;
	comp->m_caps = pProto->m_caps;
	comp->m_speedX = pProto->m_maxSpeed;
	comp->m_speedY = 0;
	comp->m_flipTimer = pProto->m_flipTimer;
	comp->SetCollisionLayer( pProto->m_collisionLayer );
	comp->m_collisionSpheres = pProto->m_collisionSpheres;

	if ( !pProto->m_collisionMask.empty() )
	{
		IRender & render = GetRender();
		ImageHandle imgHandle = render.LoadImage( pProto->m_collisionMask.c_str() );
		if ( imgHandle == ImageHandle::INVALID )
			return false;

		IImage *pImg = render.GetImageByHandle( imgHandle );
		if ( pImg )
		{
			const int w = pImg->GetWidth();
			const int h = pImg->GetHeight();

			comp->m_collisionMask.Create( w, h );
			render.LockImage( imgHandle );

			for ( int y = 0; y < h; ++y )
			{
				for ( int x = 0; x < w; ++x )
				{
					Color c = render.GetPixel( imgHandle, x, y );
					comp->m_collisionMask.Set( x, y, c.r != 0 );
				}
			}

			render.UnlockImage( imgHandle );
			//comp->m_collisionMaskDebug = pProto->m_collisionMask;
		}
	}

	return comp->GetID();
}

ControlCompID Game::CreateControlComponent( const GameObjectID & parent, const std::string & proto )
{
	const SControlsProto *pProto = GetControls().GetResource( proto );
	if ( !pProto )
		return ControlCompID();

	ControlComponent *comp = m_objectStorage.CreateControlComponent();
	comp->SetParent( parent );
	comp->m_type = pProto->m_type;
	comp->m_controls = 0;

	return comp->GetID();
}

MechanicCompID Game::CreateMechanicComponent( const GameObjectID & parent, const std::string & proto )
{
	const SMechanicsProto *pProto = GetMechanics().GetResource( proto );
	if ( !pProto )
		return MechanicCompID();

	MechanicComponent *comp = m_objectStorage.CreateMechanicComponent();
	comp->SetParent( parent );
	comp->m_type = pProto->m_type;
	comp->m_owner = pProto->m_owner;
	comp->m_deathObject = pProto->m_deathObject;
	comp->m_deathAI = pProto->m_deathAI;
	comp->m_score = pProto->m_score;
	comp->m_statType = pProto->m_statType;

	for ( int i = 0; i < MAX_WEAPONS; ++i )
	{
		SWeapon & wpn = comp->m_weapons[ i ];
		const SWeaponProto & wProto = pProto->m_weapons[ i ];
		wpn.m_object = wProto.m_object;
		wpn.m_point = wProto.m_point;
		wpn.m_ammoType = wProto.m_ammoType;
		wpn.m_ammoMax = wProto.m_ammo;
		wpn.m_cooldown = wProto.m_cooldown;
		wpn.m_angleLimit = wProto.m_angleLimit;
		wpn.m_heightLimit = wProto.m_heightLimit;
		wpn.m_ai = wProto.m_ai;
		wpn.m_currentAmmo = wpn.m_ammoMax;
		wpn.m_date = wProto.m_date;
	}

	return comp->GetID();
}

SoundCompID Game::CreateSoundComponent( const GameObjectID & parent, const std::string & proto )
{
	const SSoundProto *pProto = GetSounds().GetResource( proto );
	if ( !pProto )
		return SoundCompID();

	SoundComponent *comp = m_objectStorage.CreateSoundComponent();
	comp->SetParent( parent );
	for ( size_t i = 0; i < pProto->m_events.size(); ++i )
	{
		const SSoundEventProto & e = pProto->m_events[ i ];
		SSound & s = comp->m_sounds[ e.m_event ];
		s.m_sound.Load( e.m_sound );
		s.m_sound.SetLoopMode( e.m_looped ? ISample::LOOPMODE_LOOP : ISample::LOOPMODE_ONCE );
		s.m_sound.SetVolume( e.m_volume );
		s.m_pitchFollowsAngle = e.m_pitchFollowsAngle;
		s.m_minPitch = e.m_minPitch;
		s.m_maxPitch = e.m_maxPitch;
	}

	comp->PushEvent( "spawn" );

	return comp->GetID();
}

const RequestID & Game::CreateGameObject( const std::string & proto, const SCreateObjectRequest::SData & data, GameObjectID replaceID )
{
	m_creates.push_back( SCreateObjectRequest( NextRequestID(), proto, data, replaceID ) );
	return m_creates.back().m_id;
}

GameObjectID Game::GetCreateRequestResult( const RequestID & requestID ) const
{
	CreateResults::const_iterator iter = m_createResults.find( requestID );
	if ( iter == m_createResults.end() )
		return GameObjectID();

	return iter->second;
}

void Game::DestroyGameObject( const GameObjectID & id )
{
	m_destroys.push_back( SDestroyObjectRequest( NextRequestID(), id ) );
}

void Game::DestroyAllObjects()
{
	ObjectsStorage::GameObjectContainer & objs = m_objectStorage.GetGameObjects();
	m_creates.clear();
	m_destroys.clear();
	for ( ObjectsStorage::GameObjectContainer::Iterator iter = objs.GetIterator(); !iter.AtEnd(); ++iter )
	{
		m_destroys.push_back( SDestroyObjectRequest( NextRequestID(), (*iter)->GetID() ) );
	}
}

bool Game::LoadNextLevel()
{
	const std::string & levelName = m_levels.NextLevel();
	return m_director.LoadLevel( levelName );
}

void Game::UpdateUnlockedLevel()
{
	if ( m_levels.m_currentLevel + 1 > m_playerProfile.m_maxUnlockedLevel && m_levels.m_currentLevel < (int)m_levels.m_levels.size() - 1 )
	{
		m_playerProfile.m_maxUnlockedLevel = m_levels.m_currentLevel + 1;
		m_playerProfile.Save();
	}
}

const char *Game::GetObjectPictureForMenu( const char *objectName ) 
{
	const SGameObjectProto *pProto = GetGameObjects().GetResource( objectName );
	if ( !pProto )
		return "";

	const SGraphicsProto *pGrProto = GetGraphics().GetResource( pProto->m_graphics );
	if ( !pGrProto )
		return "";

	return pGrProto->m_image.c_str();
}

void Game::UpdateDayTime( float dtSec )
{
	const float dayLength = GetSystem().GetConfigValue_Int( "DD_Gameplay", "Length" )*60.0f;
	m_dayTime += dtSec / dayLength;
	if ( m_dayTime > 1.0f )
	{
		m_dayTime -= 1.0f;
		--m_daysLeft;
	}
}

float Game::GetDayTime() const
{
	return m_dayTime;
}

int Game::GetDaysLeft() const
{
	return m_daysLeft;
}

void Game::EnumerateLevels( ILevelsEnumerator & enumerator )
{
	for ( SLevelProgression::Levels::const_iterator iter = m_levels.m_levels.begin();
		  iter != m_levels.m_levels.end();
		  ++iter )
	{
		const int index = iter - m_levels.m_levels.begin();
		enumerator( iter->m_name.c_str(), iter->m_object.c_str(), index, index > 1 && index > m_playerProfile.m_maxUnlockedLevel );
	}
}

void Game::RestartGame( int level )
{
	m_objectStorage.Clear();
	m_freeRequestID = RequestID();
	m_creates.clear();
	m_createResults.clear();
	m_destroys.clear();
	m_director.Reset();
	m_levels.m_currentLevel = level - 1;
	m_daysLeft = INITIAL_DAYS;
	m_dayTime = 0.0f;
	m_playerProfile.ResetGameStats();
}

bool Game::ToggleDebugReflection( const InputEvent & ev )
{
	DebugReflectionLine = !DebugReflectionLine;
	return true;
}

#ifdef _DEBUG
bool Game::Cheat( const InputEvent & ev )
{
	if ( m_currentStates.back()->GetType() != GAMESTATE_LEVEL )
		return true;

	m_director.Cheat();
	((GameStateLevel*)(m_gameStates[ GAMESTATE_LEVEL ].get() ))->CheckWinConditions( *this );
	return true;
}

bool Game::GodMode( const InputEvent & ev )
{
	m_godMode = !m_godMode;
	return true;
}

bool Game::DebugAch( const InputEvent & ev )
{
	m_debugAchMode = true;
	return true;
}
#endif

