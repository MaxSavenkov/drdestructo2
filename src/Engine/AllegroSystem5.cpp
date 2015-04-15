#include "stdafx.h"
#include "AllegroSystem5.h"
#include "LogCommon.h"

bool AllegroSystem5::Init( const char *appName, const char *logFile )
{
	al_set_app_name( appName );

	m_canWrite = false;

	if ( !al_init() )
	{
		//GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroSystem: Failed to initialize Allegro" );
		return false;
	}

	const bool dataExists = al_filename_exists("Data/dd2_icon.ico" );
	
	if ( !dataExists )
	{
		ALLEGRO_PATH *exePath = al_get_standard_path( ALLEGRO_EXENAME_PATH );
		al_set_path_filename( exePath, "" );
		printf("Binary at: %s\n", al_path_cstr( exePath, '/' ) );
		if ( !al_change_directory( al_path_cstr( exePath, '/' ) ) )
		{
			printf("Failed to change current directory\n");
			return false;
		}
		al_destroy_path( exePath );

		if ( !al_filename_exists( "Data/dd2_icon.ico" ) )
		{
			printf("Failed to Data/dd2_icon.ico test at '%s'\n", al_get_current_directory() );
			return false;
		}
	}

	ALLEGRO_FILE *testFile = al_fopen( "./testfile", "w" );

	if ( testFile )
	{
		printf("Application directory is writeable\n");
		al_fclose( testFile );
		al_remove_filename( "./testfile" );
		m_canWrite = true;
		m_userPath = "";
	}
	else
	{
		ALLEGRO_PATH *pUserPath = al_get_standard_path( ALLEGRO_USER_DATA_PATH );
		std::string userPathStr = al_path_cstr( pUserPath, '/' );
		al_set_path_filename( pUserPath, "testFile" );
		const char *testFilePath = al_path_cstr( pUserPath, '/' );

		if ( al_make_directory( userPathStr.c_str() ) )
		{
			testFile = al_fopen( testFilePath, "w" );
			if ( testFile )
			{
				printf("Using user data path: %s\n", userPathStr.c_str());
				al_fclose( testFile );
				al_remove_filename( testFilePath );

				m_canWrite = true;
				m_userPath = userPathStr;
			}
			else
			{
			    printf("Failed to write to %s\n", userPathStr.c_str());
			}
		}
		else
		{
		    printf("Failed to create %s\n", userPathStr.c_str());
		}

		al_destroy_path( pUserPath );
	}

	GetLog().StartLogger( logFile );

	if ( !m_canWrite )
		GetLog().Log( CommonLog(), LL_WARNING, "AllegroSystem: could not determine writable location for user data!", m_userPath.c_str() );
	else
		GetLog().Log( CommonLog(), LL_INFO, "AllegroSystem: user data path: \"%s\"", m_userPath.c_str() );

	return true;
}

void AllegroSystem5::SetWindowTitle( const std::string & title )
{
	al_set_window_title( al_get_current_display(), title.c_str() );
}

const char *AllegroSystem5::GetWriteablePath()
{
	static std::string s;
	s = GetFullFilename( "" );
	return s.c_str();
}

void AllegroSystem5::SetConfigValue_Int( const char *section, const char *key, int value )
{
	if ( !m_config )
	{
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroSystem: There is no configuration" );
		return;
	}

	char svalue[ 255 ];
	sprintf( svalue, "%i", value );
	al_set_config_value( m_config, section, key, svalue );
}

void AllegroSystem5::SetConfigValue_String( const char *section, const char *key, const char *value )
{
	if ( !m_config )
	{
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroSystem: There is no configuration" );
		return;
	}

	al_set_config_value( m_config, section, key, value );
}

void AllegroSystem5::SetConfigName( const std::string & cfg )
{
	m_configName = cfg;
}

void AllegroSystem5::SaveConfig()
{
	if ( !m_config )
	{
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroSystem: There is no configuration" );
		return;
	}

	const char *fullPath = GetFullFilename( m_configName.c_str() );

	if ( !al_save_config_file( fullPath, m_config ) )
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroSystem: Failed to save config file %s", m_configName.c_str() );
}

void AllegroSystem5::LoadConfig()
{
	const char *fullPath = GetFullFilename( m_configName.c_str() );
	m_config = al_load_config_file( fullPath );
	if ( !m_config )
	{
		GetLog().Log( CommonLog(), LL_WARNING, "AllegroSystem: Failed to load config file %s. New config will be created", m_configName.c_str() );
		m_config = al_create_config();
		SetConfigValue_String( "graphics", "min_filter", "linear" );
		SetConfigValue_String( "graphics", "mag_filter", "linear" );
		SaveConfig();
	}
}

bool AllegroSystem5::HasConfigValue( const char *section, const char *key )
{
	if ( !m_config )
		return false;

	const char *value = al_get_config_value( m_config, section, key );

	return value != 0;
}

int AllegroSystem5::GetConfigValue_Int( const char *section, const char *key, int defValue )
{
	if ( !m_config )
	{
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroSystem: There is no configuration" );
		return defValue;
	}

	const char *value = al_get_config_value( m_config, section, key );
	
	return value ? atoi( value ) : defValue;
}

const char *AllegroSystem5::GetConfigValue_String( const char *section, const char *key )
{
	if ( !m_config )
	{
		GetLog().Log( CommonLog(), LL_CRITICAL, "AllegroSystem: There is no configuration" );
		return "";
	}

	const char *value = al_get_config_value( m_config, section, key );
	return value ? value : "";
}

FileFilter::FileFilter( const std::string & mask )
{
	size_t prevPos = 0;
	size_t pos = 0;

	while( pos != std::string::npos )
	{
		std::string name, ext;

		pos = mask.find( ";", prevPos );
		std::string f = mask.substr( prevPos, pos - prevPos );

		while( !f.empty() && isspace( f[ 0 ] ) )
			f.erase( 0, 1 );
		while( !f.empty() && isspace( f[ f.size() - 1 ] ) )
			f.erase( f.size() - 1, 1 );

		size_t dotPos = f.find( "." );
		if ( dotPos == std::string::npos )
		{
			name = f;
			ext = "";
		}
		else
		{
			name = f.substr( 0, dotPos );
			ext = f.substr( dotPos + 1 );
		}

		while( !name.empty() && isspace( name[ 0 ] ) )
			name.erase( 0, 1 );
		while( !name.empty() && isspace( name[ name.size() - 1 ] ) )
			name.erase( name.size() - 1, 1 );

		while( !ext.empty() && isspace( ext[ 0 ] ) )
			ext.erase( 0, 1 );
		while( !name.empty() && isspace( ext[ ext.size() - 1 ] ) )
			ext.erase( name.size() - 1, 1 );

		if ( !name.empty() || !ext.empty() )
		{
			Filter newFilter;
			newFilter.m_name = name;
			newFilter.m_ext = ext;
			m_filters.push_back( newFilter );
		}

		prevPos = pos + 1;
	}
}

bool FileFilter::FilterString( const std::string & string, const std::string & filter ) const
{
	size_t strPos = 0;
	for ( size_t i = 0; i < filter.size(); ++i )
	{
		if ( strPos >= string.size() )
			return false;

		char c = filter[ i ];
		if ( c == '*' )
		{
			if ( i == filter.size() - 1 )
				return true;
			char t = filter[ ++i ];
			
			size_t tPos = string.find( t, strPos );
			if ( tPos == std::string::npos )
				return false;
			strPos = tPos + 1;
		}
		else
		{
			if ( string[ strPos++ ] != c )
				return false;
		}
	}

	return true;
}

bool FileFilter::FilterFile( const char *basename, const char *extension ) const
{
	if ( !basename || !extension )
		return false;

	std::string name = basename;
	std::string ext = extension;

	if ( !ext.empty() && ext[0] == '.' )
		ext.erase( 0, 1 );

	if ( m_filters.empty() )
		return true;

	for ( std::vector<Filter>::const_iterator iter = m_filters.begin();
		  iter != m_filters.end();
		  ++iter )
	{
		bool nameOK = false, extOK = false;

		nameOK = FilterString( name, iter->m_name );
		if ( !nameOK )
			continue;
		extOK = FilterString( ext, iter->m_ext );
		if ( !extOK )
			continue;

		return true;
	}

	return false;
}

void AllegroSystem5::IterateFiles( const std::string & startPath, const std::string & mask, bool includeDirs, bool iterateSubDirs, IFileIterator & iterator )
{
	std::string realPath = startPath;

	// Because Allegro can't handle trailing slashes in dir names :(
	while( !realPath.empty() && ( 
		realPath[ realPath.length() - 1 ] == '/' ||
		realPath[ realPath.length() - 1 ] == '\\' ) )
		realPath.erase( realPath.length() - 1, 1 );

	ALLEGRO_FS_ENTRY *entry;
	//ALLEGRO_PATH *p = al_create_path_for_directory( realPath.c_str() );
	//al_make_path_absolute( p );
	//const char *fp = al_path_cstr( p, ALLEGRO_NATIVE_PATH_SEP );	
	entry = al_create_fs_entry( realPath.c_str() );
	if ( !al_open_directory( entry ) )
	{
		//al_destroy_path( p );
		al_destroy_fs_entry( entry );
		return;
	}
	//al_destroy_path( p );

	FileFilter filter( mask );
	IterateDir( entry, filter, includeDirs, iterateSubDirs, iterator );

	al_close_directory( entry );
	al_destroy_fs_entry( entry );
}

void AllegroSystem5::IterateDir( ALLEGRO_FS_ENTRY *pDir, const FileFilter & filter, bool includeDirs, bool iterateSubDirs, IFileIterator & iterator )
{
	while( ALLEGRO_FS_ENTRY *pEntry = al_read_directory( pDir ) )
	{
		bool isDir = ( al_get_fs_entry_mode( pEntry ) & ALLEGRO_FILEMODE_ISDIR );
		if ( isDir && iterateSubDirs )
		{
			if ( al_open_directory( pEntry ) )
			{
				IterateDir( pEntry, filter, includeDirs, iterateSubDirs, iterator );
				al_close_directory( pEntry );
			}
		}

		if ( !isDir || includeDirs )
		{
			const char *charPath = al_get_fs_entry_name( pEntry );
			ALLEGRO_PATH *pPath = al_create_path( charPath );
			if ( pPath )
			{
				const char *name = al_get_path_basename( pPath );
				const char *ext = al_get_path_extension( pPath );
			
				if ( filter.FilterFile( name, ext ) )
					iterator( name, isDir );
			}
			al_destroy_path( pPath );
		}
	}
}

bool AllegroSystem5::CreateDir( const char *path )
{
	return al_make_directory( path );
}

void AllegroSystem5::Sleep( float ms )
{
	al_rest( ms );
}

double AllegroSystem5::GetTime() const
{
	return al_get_time();
}

bool AllegroSystem5::LoadFile( bool fromAssets, const char *filename, std::vector<char> & data )
{
	const char *fullPath = filename;
	if ( !fromAssets )
		fullPath = GetFullFilename( filename );

	ALLEGRO_FILE *file = al_fopen( fullPath, "rb" );

	if ( !file )
		return false;

	const int len = al_fsize( file );
	data.resize( len + 1 );

	if ( al_fread( file, &data[0], len ) != len )
	{
		al_fclose( file );
		return false;
	}

	al_fclose( file );

	return true;
}

bool AllegroSystem5::SaveFile( const char *filename, const std::vector<char> & data )
{
	const char *fullPath = GetFullFilename( filename );

	ALLEGRO_FILE *file = al_fopen( fullPath, "w" );

	if ( !file )
		return false;

	if ( !al_fwrite( file, &data[0], data.size() ) )
	{
		al_fclose( file );
		return false;
	}

	al_fclose( file );

	return true;	
}

ISystem & GetSystem()
{
	static AllegroSystem5 sys;
	return sys;
}

const char *AllegroSystem5::GetFullFilename( const char *filename )
{
	ALLEGRO_PATH *path = al_create_path( m_userPath.c_str() );
	al_set_path_filename( path, filename );

	m_tmpFilename = al_path_cstr( path, '/' );

	al_destroy_path( path );

	return m_tmpFilename.c_str();
}

/*struct Test
{
	Test()
	{
		FileFilter f( "*.profile; abc*.*;   a*z.*" );
		bool t1 = f.FilterFile( "zep", "" );
		bool t2 = f.FilterFile( "zep", "profile" );
		bool t3 = f.FilterFile( "abcnarm", "exe" );
		bool t4 = f.FilterFile( "a123z345", "exe" );
		bool t5 = f.FilterFile( "a", "bat" );
	}
}t;*/