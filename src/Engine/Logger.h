#pragma once
#include "ILogger.h"

struct LogChannelInfo
{
	LogChannel m_id;
	std::string m_name;
	LogMode m_mode;

	LogChannelInfo(LogChannel id, const std::string& name, LogMode mode):
		m_id(id), m_name(name), m_mode(mode)
		{}
};

class Logger : public ILogger
{
	LogChannel LOGGER_OWN_CHANNEL;

	typedef std::map<LogChannel, LogChannelInfo> ChannelMap;
	typedef std::map<LogLevel, std::string> LevelMap;
	void LogV(const std::string &name, LogMode mode, LogLevel level, const char* fmt, va_list args);
public:
	bool StartLogger( const std::string & file );
	void Log(LogChannel channel, LogLevel level, const char *fmt, ...);
	void Log(LogChannel channel, LogMode mode, LogLevel level, const char *fmt, ...);
	void LogBinary(LogChannel, LogLevel level, const char *data, int len);
	LogChannel RegisterChannel(LogMode mode, const std::string& name);	

	Logger():m_file(NULL),m_started(false)
	{}
	~Logger()
	{
		if ( m_file )
			fclose(m_file);
	}
private:
	bool m_started;
	std::vector<char> m_buffer;
	ChannelMap m_channels;
	LevelMap m_levels;
	FILE *m_file;
};
