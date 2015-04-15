#pragma once

class IFileIterator
{
public:
	virtual void operator()( const char *filename, bool isDir ) = 0;
};

  // See AllegroSystem5 for some comments
class ISystem
{
public:
	virtual bool Init( const char *appName, const char *logFile ) = 0;
	virtual void SetWindowTitle( const std::string & title ) = 0;

	virtual const char *GetWriteablePath() = 0;

	virtual void SetConfigValue_Int( const char *section, const char *key, int value ) = 0;
	virtual void SetConfigValue_String( const char *section, const char *key, const char *value ) = 0;
	virtual void SetConfigName( const std::string & cfg ) = 0;
	virtual void SaveConfig() = 0;
	virtual void LoadConfig() = 0;
	virtual bool HasConfigValue( const char *section, const char *key ) = 0;
	virtual int GetConfigValue_Int( const char *section, const char *key, int defValue = 0 ) = 0;
	virtual const char *GetConfigValue_String( const char *section, const char *key ) = 0;
	virtual void IterateFiles( const std::string & startPath, const std::string & mask, bool includeDirs, bool iterateSubDirs, IFileIterator & iterator ) = 0;
	virtual bool CreateDir( const char *path ) = 0;
	virtual void Sleep( float ms ) = 0;
	virtual double GetTime() const = 0;
	virtual bool LoadFile( bool fromAssets, const char *filename, std::vector<char> & data ) = 0;
	virtual bool SaveFile( const char *filename, const std::vector<char> & data ) = 0;
};

ISystem & GetSystem();
