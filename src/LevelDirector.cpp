#include "stdafx.h"
#include "LevelDirector.h"
#include "ObjectsStorage.h"
#include "XMLDeserializer.h"
#include "Random.h"
#include "Common.h"

EDirection ParseDirection( const std::string & strValue )
{
	if ( strValue == "LEFT" )
		return RIGHT_TO_LEFT;

	return LEFT_TO_RIGHT;
}

LevelDirector::LevelDirector()
{
	Reset();
}

struct SpawnRequestRemover
{
	LevelDirector *m_director;
	IGameContext & m_context;
	SpawnRequestRemover( LevelDirector *director, IGameContext & context )
		: m_director( director )
		, m_context( context )
	{
	
	}

	bool operator()( const SSpawnRequest & request )
	{
		GameObjectID id = m_context.GetCreateRequestResult( request.m_id );
		if ( id.IsValid() )
		{
			m_director->m_spawns.push_back( SSpawnedObject( id, request.m_proto ) );
			return true;
		}

		return false;
	}
};

void LevelDirector::Update( float dt, IGameContext & context )
{
// Debug code to see current spawn parameters
{
//GetRender().DrawText( 0, 50, Color(255,255,255), 25, "%i% (%i/%i ~ %f)", m_sinkPercent, m_currentSinkPixels, m_initialSinkPixels, m_currentSinkPixels/(float)m_initialSinkPixels );

/*int totalObjects = 0;

for ( SpawnedVector::iterator iter = m_spawns.begin(); iter != m_spawns.end(); ++iter )
{
	const GameObject *pObj = context.GetOjectsStorage().GetGameObject( iter->m_id );
	if ( pObj )
		++totalObjects;
}
IRender & render = GetRender();
render.DrawText( 0, 50, Color(255,255,255), 10, "Spawn timeout: %f", m_spawnTimeout );
render.DrawText( 0, 60, Color(255,255,255), 10, "Objects: %i/%i", totalObjects, m_maxObjects );
float totalProb = 0;
for ( AIProtoVector::iterator iter = m_protos.begin(); iter != m_protos.end(); ++iter )
{
	totalProb += iter->m_spawnProb;
}

for ( AIProtoVector::iterator iter = m_protos.begin(); iter != m_protos.end(); ++iter )
{
	SAIProto & p = *iter;
	render.DrawText( 0, 70 + (iter-m_protos.begin())*10, Color(255,255,255), 10, "%10.10s: %3.3f %3.3f", p.m_objectProto.c_str(), iter->m_spawnProb, iter->m_spawnProb / totalProb );
}*/
}
      // Creating target objects
	if ( !m_targetObjectID.IsValid() )
	{
		if ( !m_createTargetRequestID.IsValid() )
		{
			SCreateObjectRequest::SData data;
			data.x = m_targetObjectX;
			data.y = m_targetObjectY;
			m_createTargetRequestID = context.CreateGameObject( m_targetObjectProto, data );
		}
		else
		{
			m_targetObjectID = context.GetCreateRequestResult( m_createTargetRequestID );
			assert( m_targetObjectID.IsValid() );
			m_createTargetRequestID.Invalidate();
			m_initialSinkPixels = 0;
			const PhysicComponent *pPhys = context.GetOjectsStorage().GetPhysicComponent( context.GetOjectsStorage().GetGameObject( m_targetObjectID )->GetPhysicComponent() );
			if ( pPhys )
			{
				for ( int x = 0; x < pPhys->m_collisionMask.GetWidth(); ++x )
				{
					if ( pPhys->m_collisionMask.Get( x, m_sinkLineY ) )
						++m_initialSinkPixels;
				}
			}
			m_currentSinkPixels = m_initialSinkPixels;
		}
	}

	SpawnRequestRemover remover( this, context );
	SpawnRequests::iterator newEnd = std::remove_if( m_requests.begin(), m_requests.end(), remover );

	m_requests.erase( newEnd, m_requests.end() );

	  // Process spawn cooldowns
	for ( AIProtoVector::iterator iter = m_protos.begin(); iter != m_protos.end(); ++iter )
	{
		if ( iter->m_spawnProb < 1.0f )
			iter->m_spawnProb += iter->m_cooldownPerSecond * dt;
		if ( iter->m_spawnProb > 1.0f )
			iter->m_spawnProb = 1.0f;
	}

	  // Process global spawn timeout
	m_spawnTimeout -= dt;
	if ( m_spawnTimeout > 0 )
		return;

	  // Count objects by type
	for ( CountMap::iterator iter = m_objectsCounts.begin(); iter != m_objectsCounts.end(); ++iter )
		iter->second = 0;

	const ObjectsStorage & storage = context.GetOjectsStorage();

	int totalObjects = 0;
	
	for ( int i = 0; i < (int)m_spawns.size(); ++i )
	{
		const SSpawnedObject & spawn = m_spawns[ i ];
		const GameObject *pObj = storage.GetGameObject( spawn.m_id );
		if ( pObj )
		{
			++m_objectsCounts[ spawn.m_proto ];
			++totalObjects;
		}
		else
		{
			m_spawns.erase( m_spawns.begin() + i );
			--i;
		}
	}

	if ( totalObjects >= m_maxObjects )
		return;

	  // Calculate total spawn probability for all objects
	float totalProb = 0.0f;

	for ( AIProtoVector::iterator iter = m_protos.begin(); iter != m_protos.end(); ++iter )
	{
		totalProb += iter->m_spawnProb;
	}

	if ( totalProb <= 0.0001f )
		return;

	  // Choose an object to spawn based on relative weight
	float prob1 = 0;
	float rnd1 = RndFloat( 0, 1.0f );

	for ( AIProtoVector::iterator iter = m_protos.begin(); iter != m_protos.end(); ++iter )
	{
		prob1 += iter->m_spawnProb / totalProb;
		if ( rnd1 < prob1 )
		{
			SAIProto & p = *iter;
			const int intervalIndex = RndInt( 0, (int)p.m_intervals.size() - 1 );
			const SAIInterval & in = p.m_intervals[ intervalIndex ];

			if ( in.m_schemesWeight <= 0 )
				continue;

			SCreateObjectRequest::SData data;

			int dx = ( in.x2 <= in.x1 ) ? 0 : ( RndInt( 0, in.x2 - in.x1 ) );
			int dy = ( in.y2 <= in.y1 ) ? 0 : ( RndInt( 0, in.y2 - in.y1 ) );
			data.x = in.x1 + dx;
			data.y = in.y1 + dy;
			data.dir = in.dir;

			int prob = 0;
			int rnd = RndInt( 0, in.m_schemesWeight - 1 );
			for ( SAIInterval::SchemeVector::const_iterator sIter = in.m_schemes.begin(); sIter != in.m_schemes.end(); ++sIter )
			{
				prob += sIter->m_weight;
				if ( rnd < prob )
				{
					data.ai = sIter->m_aiName;
					break;
				}
			}

			m_requests.push_back( SSpawnRequest( context.CreateGameObject( p.m_objectProto, data ), iter - m_protos.begin() ) );

			p.m_spawnProb = 0;
			
			const float fillPercent = totalObjects / (float)m_maxObjects;
			const int minSpawnTimeout = 100 + fillPercent * 500;
			const int maxSpawnTimeout = 200 + fillPercent * 1000;
			m_spawnTimeout = RndInt( minSpawnTimeout, maxSpawnTimeout ) / 1000.0f;
			break;
		}
	}
}

int ConvertTY( const std::string & ty )
{
	return SCREEN_H * 0.75f + atoi( ty.c_str() );
}

int ConvertTX( const std::string & tx )
{
	return SCREEN_W/2 - atoi( tx.c_str() );
}

int ConvertY( const std::string & y )
{
	return SCREEN_H * 0.75f + atoi( y.c_str() );
}

class LevelAIDeserializer : public RootXMLDeserializer
{
	class AIProtoDeserializer : public XMLObjectDeserializer<SAIProto>
	{
		class IntervalDeserializer : public XMLObjectDeserializer<SAIInterval>
		{
			class SchemeDeserailizer : public XMLObjectDeserializer<SAISchemePreset>
			{
				void Bind( SAISchemePreset & object )
				{
					Attrib_Value( "Name", false, object.m_aiName );
					Attrib_Value( "Weight", false, object.m_weight );
				}
				
			public:
				SchemeDeserailizer()
					: XMLObjectDeserializer<SAISchemePreset>( "Scheme", true )
				{}
			} m_schemeDes;

			void Bind( SAIInterval & object )
			{
				Attrib_SetterValue<SAIInterval, std::string>( "Region", false, object, &SAIInterval::SetRegion ); 
				Attrib_ConvertedValue( "Dir", false, &ParseDirection, object.dir );
				m_schemeDes.SetReceiver( object, &SAIInterval::AddScheme );
			}
		public:
			IntervalDeserializer()
				: XMLObjectDeserializer<SAIInterval>( "Interval", true )
			{
				SubDeserializer( m_schemeDes );
			}

		} m_interDeser;

		void Bind( SAIProto & object )
		{
			Attrib_Value( "Object", false, object.m_objectProto );
			Attrib_SetterValue<SAIProto, float>( "Cooldown", false, object, &SAIProto::SetCooldown );
			m_interDeser.SetReceiver( object, &SAIProto::AddInterval );
		}
	public:
		AIProtoDeserializer()
			: XMLObjectDeserializer<SAIProto>( "AI", true )
		{
			SubDeserializer( m_interDeser );
		}
	} m_aiDes;

	class BriefingDeserializer : public XMLCustomDeserializer
	{
		LevelDirector & m_levelDirector;
	public:
		BriefingDeserializer( LevelDirector & director )
			: XMLCustomDeserializer( "Briefing", true )
			, m_levelDirector( director )
		{}

		bool CustomDeserialize( const TiXmlElement *pElement )
		{
			const TiXmlElement *pPhrase = 0;
			while( pPhrase = pPhrase ? pPhrase->NextSiblingElement( "Phrase" ) : pElement->FirstChildElement( "Phrase" ) )
			{
				std::string image, text;
				if ( pPhrase->QueryValueAttribute( "Image", &image ) != TIXML_SUCCESS )
					continue;
				if ( pPhrase->QueryValueAttribute( "Text", &text ) != TIXML_SUCCESS )
					continue;

				ImageHandle imgHandle = GetRender().LoadImage( image.c_str() );
				TextHandle txtHandle = GetTextManager().LoadResource( text.c_str() );
				m_levelDirector.m_briefing.push_back( STalkEntry( imgHandle, txtHandle ) );
			}

			return true;
		}
	} m_briefingDes;

	class PBPDeserializer : public XMLCustomDeserializer
	{
		LevelDirector & m_levelDirector;
	public:
		PBPDeserializer( LevelDirector & director )
			: XMLCustomDeserializer( "PlayByPlay", true )
			, m_levelDirector( director )
		{}

		bool CustomDeserialize( const TiXmlElement *pElement )
		{
			const TiXmlElement *pPhrase = 0;
			while( pPhrase = pPhrase ? pPhrase->NextSiblingElement( "Phrase" ) : pElement->FirstChildElement( "Phrase" ) )
			{
				std::string image, text;
				int percent;

				if ( pPhrase->QueryValueAttribute( "Image", &image ) != TIXML_SUCCESS )
					continue;
				if ( pPhrase->QueryValueAttribute( "Text", &text ) != TIXML_SUCCESS )
					continue;
				if ( pPhrase->QueryValueAttribute( "Percent", &percent ) != TIXML_SUCCESS )
					continue;

				ImageHandle imgHandle = GetRender().LoadImage( image.c_str() );
				TextHandle txtHandle = GetTextManager().LoadResource( text.c_str() );
				m_levelDirector.m_playByPlay.push_back( SPlayByPlayEntry( imgHandle, txtHandle, percent ) );
			}

			return true;
		}
	} m_pbpDes;

	class ReflectionParamsDeserializer : public XMLDataDeserializer
	{
		SReflectionControllerParams & m_params;
	public:
		ReflectionParamsDeserializer( SReflectionControllerParams & params, DataFinisherDelegate delegate )
			: XMLDataDeserializer( "ReflectionController", true, delegate )
			, m_params( params )
		{
			Attrib_Value( "ReflectionPercent", true, params.m_reflectionPercent );
			Attrib_Value( "BeforeAfterY", false, params.m_beforeAfterY );			
		}
	}m_reflectDes;

	class GroundParamsDeserializer : public XMLDataDeserializer
	{
		SGroundControllerParams & m_params;
	public:
		GroundParamsDeserializer( SGroundControllerParams & params, DataFinisherDelegate delegate )
			: XMLDataDeserializer( "GroundController", true, delegate )
			, m_params( params )
		{
			Attrib_ConvertedValue( "Yfg", true, &ConvertY, params.m_yFG );
			Attrib_Value( "ImageFg", true, params.m_imageFG );
			Attrib_ConvertedValue( "Ymg", true, &ConvertY, params.m_yMG );
			Attrib_Value( "ImageMg", true, params.m_imageMG );
			Attrib_ConvertedValue( "Ybg", true, &ConvertY, params.m_yBG );
			Attrib_Value( "ImageBg", true, params.m_imageBG );
		}
	}m_groundDes;
	
	class SkyParamsDeserializer : public XMLCustomDeserializer
	{
		SSkyControllerParams & m_params;

	public:
		SkyParamsDeserializer( SSkyControllerParams & params )
			: XMLCustomDeserializer( "SkyController", true )
			, m_params( params )
		{
		}

		bool CustomDeserialize( const TiXmlElement *pElement )
		{
			if ( pElement->QueryFloatAttribute( "Percent", &m_params.m_skyPercent ) != TIXML_SUCCESS ) return false;

			const TiXmlElement *pSkyPoint = 0;
			while( pSkyPoint = pSkyPoint ? pSkyPoint->NextSiblingElement("SkyKeyPoint") : pElement->FirstChildElement("SkyKeyPoint") )
			{
				SSkyKeyPoint p;
				if ( pSkyPoint->QueryFloatAttribute( "Time", &p.m_time ) != TIXML_SUCCESS ) return false;
				if ( pSkyPoint->QueryIntAttribute( "TopR", &p.m_top.r ) != TIXML_SUCCESS ) return false;
				if ( pSkyPoint->QueryIntAttribute( "TopG", &p.m_top.g ) != TIXML_SUCCESS ) return false;
				if ( pSkyPoint->QueryIntAttribute( "TopB", &p.m_top.b ) != TIXML_SUCCESS ) return false;
				if ( pSkyPoint->QueryIntAttribute( "BotR", &p.m_bottom.r ) != TIXML_SUCCESS ) return false;
				if ( pSkyPoint->QueryIntAttribute( "BotG", &p.m_bottom.g ) != TIXML_SUCCESS ) return false;
				if ( pSkyPoint->QueryIntAttribute( "BotB", &p.m_bottom.b ) != TIXML_SUCCESS ) return false;
				m_params.m_skyKeyPoints.push_back( p );
			}

			const TiXmlElement *pStarPoint = 0;
			while( pStarPoint = pStarPoint ? pStarPoint->NextSiblingElement("StarsKeyPoint") : pElement->FirstChildElement("StarsKeyPoint") )
			{
				SStarsKeyPoint p;
				if ( pStarPoint->QueryFloatAttribute( "Time", &p.m_time ) != TIXML_SUCCESS ) return false;
				if ( pStarPoint->QueryValueAttribute( "Visible", &p.m_visible ) != TIXML_SUCCESS ) return false;
				if ( pStarPoint->QueryFloatAttribute( "Transparency", &p.m_transparency ) != TIXML_SUCCESS ) return false;
				m_params.m_starsKeyPoints.push_back( p );
			}

			const TiXmlElement *pAstroPoint = 0;
			while( pAstroPoint = pAstroPoint ? pAstroPoint->NextSiblingElement("AstroKeyPoint") : pElement->FirstChildElement("AstroKeyPoint") )
			{
				SAstroKeyPoint p;
				if ( pAstroPoint->QueryFloatAttribute( "Time", &p.m_time ) != TIXML_SUCCESS ) return false;
				if ( pAstroPoint->QueryIntAttribute( "SunR", &p.m_sunColor.r ) != TIXML_SUCCESS ) return false;
				if ( pAstroPoint->QueryIntAttribute( "SunG", &p.m_sunColor.g ) != TIXML_SUCCESS ) return false;
				if ( pAstroPoint->QueryIntAttribute( "SunB", &p.m_sunColor.b ) != TIXML_SUCCESS ) return false;
				if ( pAstroPoint->QueryIntAttribute( "MoonR", &p.m_moonColor.r ) != TIXML_SUCCESS ) return false;
				if ( pAstroPoint->QueryIntAttribute( "MoonG", &p.m_moonColor.g ) != TIXML_SUCCESS ) return false;
				if ( pAstroPoint->QueryIntAttribute( "MoonB", &p.m_moonColor.b ) != TIXML_SUCCESS ) return false;
				m_params.m_astroKeyPoints.push_back( p );
			}

			m_params.m_active = true;

			return true;
		}

	}m_skyDes;

public:
	LevelAIDeserializer( LevelDirector & level )
		: RootXMLDeserializer( "Level" )
		, m_briefingDes( level )
		, m_pbpDes( level )
		, m_reflectDes( level.m_reflectionParams, XMLDataDeserializer::DataFinisherDelegate( &level, &LevelDirector::SetHasReflection ) )
		, m_groundDes( level.m_groundParams, XMLDataDeserializer::DataFinisherDelegate( &level, &LevelDirector::SetHasGround ) )
		, m_skyDes( level.m_skyParams )
	{
		SubDeserializer( m_aiDes );
		m_aiDes.SetReceiver( level, &LevelDirector::AddProto );

		SubDeserializer( m_briefingDes );
		SubDeserializer( m_pbpDes );
		SubDeserializer( m_reflectDes );
		SubDeserializer( m_groundDes );
		SubDeserializer( m_skyDes );

		Attrib_Value( "Name", false, level.m_name );
		Attrib_Value( "TargetObject", false, level.m_targetObjectProto );
		Attrib_ConvertedValue( "TX", false, &ConvertTX, level.m_targetObjectX );
		Attrib_ConvertedValue( "TY", false, &ConvertTY, level.m_targetObjectY );
		Attrib_Value( "SinkLineY", false, level.m_sinkLineY );
		Attrib_Value( "SinkPercent", false, level.m_sinkPercent );
		Attrib_Value( "MaxObjects", true, level.m_maxObjects );
	}
};

void LevelDirector::Reset()
{
	m_maxObjects = 5;
	m_initialSinkPixels = 0;
	m_currentSinkPixels = 0;
	m_spawnTimeout = 0;
	m_objectsCounts.clear();
	m_protos.clear();
	m_spawns.clear();
	m_chances.clear();
	m_requests.clear();
	m_targetObjectProto = "";
	m_targetObjectX = 0;
	m_targetObjectY = 0;
	m_sinkLineY = 0;
	m_sinkPercent = 0;
	m_initialSinkPixels = 0;
	m_currentSinkPixels = 0;
	m_targetObjectID = GameObjectID();
	m_createTargetRequestID = RequestID();
	m_playByPlay.clear();
	m_briefing.clear();
	m_reflectionParams = SReflectionControllerParams();
	m_groundParams = SGroundControllerParams();
	m_skyParams = SSkyControllerParams();
}

bool LevelDirector::LoadLevel( const std::string & level )
{
	Reset();
	LevelAIDeserializer root( *this );
	XMLDeserializer des( root );
	return des.Deserialize( level );
}
