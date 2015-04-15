#pragma once
#include "ISystem.h"

struct FileFilter
{
	struct Filter
	{
		std::string m_name;
		std::string m_ext;
	};

	std::vector< Filter > m_filters;

	FileFilter( const std::string & mask );
	bool FilterFile( const char *basename, const char *ext ) const;
	bool FilterString( const std::string & string, const std::string & filter ) const;
};

/*
	Implementation of ISystem for Allegro5.
	I don't hide ALL platform-specific functions here, even though
	I probably should.
*/
class AllegroSystem5 : public ISystem
{
	ALLEGRO_CONFIG *m_config;
	std::string m_configName;
	std::string m_userPathSubdirectory;
	std::string m_userPath;
	bool m_canWrite;
	bool m_useSubdirectory;

	std::string m_tmpFilename;

private:
	void SetUserPathSubdirectory( const char *dirName );
	const char *GetFullFilename( const char *filename );

public:
	AllegroSystem5()
		: m_config( 0 )
	{}

	bool Init( const char *appName, const char *logFile );
	void SetWindowTitle( const std::string & title );
	
	const char *GetWriteablePath();

	void SetConfigValue_Int( const char *section, const char *key, int value );
	void SetConfigValue_String( const char *section, const char *key, const char *value );
	void SetConfigName( const std::string & cfg );
	void SaveConfig();
	void LoadConfig();
	bool HasConfigValue( const char *section, const char *key );
	int GetConfigValue_Int( const char *section, const char *key, int defValue );
	const char *GetConfigValue_String( const char *section, const char *key );
	void IterateFiles( const std::string & startPath, const std::string & mask, bool includeDirs, bool iterateSubDirs, IFileIterator & iterator );
	bool CreateDir( const char *path );

	void IterateDir( ALLEGRO_FS_ENTRY *pEntry, const FileFilter & filter, bool includeDirs, bool iterateSubDirs, IFileIterator & iterator );

	void Sleep( float ms );
	double GetTime() const;
	bool LoadFile( bool fromAssets, const char *filename, std::vector<char> & data );
	bool SaveFile( const char *filename, const std::vector<char> & data );
};
