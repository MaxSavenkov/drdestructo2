#include "stdafx.h"
#include "Logger.h"
#include "ISystem.h"

#ifdef WIN32
	#include "windows.h"
#endif

#if defined(ALLEGRO_ANDROID) || defined(ALLEGRO_IPHONE)
	ALLEGRO_DEBUG_CHANNEL( "DrDestructo2" );
#endif

bool Logger::StartLogger( const std::string & file )
{	
	m_levels[LL_CRITICAL] = "critical";
	m_levels[LL_WARNING] = "warning";
	m_levels[LL_INFO] = "info";
	m_levels[LL_VERBOSE] = "verbose";

	LOGGER_OWN_CHANNEL = RegisterChannel(LM_FILE, "Logger");

	std::string path = GetSystem().GetWriteablePath();
	if ( !path.empty() && *path.rbegin() != '/' && *path.rbegin() != '\\' )
		path += "/";
	path += file;

	m_file = fopen(path.c_str(), "w+");
	m_started = ( m_file != NULL );
	
	printf("Log file at: %s\n", path.c_str());

	return m_started;
}

void Logger::LogV(const std::string &name, LogMode mode, LogLevel level, const char *fmt, va_list args)
{	
	if (!m_started)
	{
	#ifdef WIN32
		MessageBoxA(NULL, "Logger not started!", "Error!", MB_OK);
	#else
		printf("Logger not started!\n");
	#endif
		return;
	}
	
	
	va_list argsCopy;

#ifdef WIN32
	argsCopy = args;
#else
	va_copy( argsCopy, args );
#endif	

	#ifdef WIN32
	unsigned int bufLen = _vscprintf(fmt, args) + 1;
	#else
	unsigned int bufLen = vsnprintf(0,0,fmt,args)+1;
	#endif
	if ((int)m_buffer.size()	<= bufLen)
	    m_buffer.resize(bufLen);
	// WIN32 only - breaks portability
	vsprintf(&m_buffer[0], fmt, argsCopy);

#ifdef WIN32
    struct tm *newtime;
    __time64_t long_time;
    _time64( &long_time );
    newtime = _localtime64( &long_time );
#else
    struct tm *newtime;
    time_t long_time;
    time( &long_time );
    newtime = localtime( &long_time );
#endif

	if (mode >= LM_FILE)
	{
#if defined(ALLEGRO_ANDROID) || defined(ALLEGRO_IPHONE)
		ALLEGRO_DEBUG(&m_buffer[0]);
#else
		fprintf(m_file, "%d.%d.%d %d:%d:%2.2d [%s] [%s]: %s\n",
			newtime->tm_mday, newtime->tm_mon, 1900 + newtime->tm_year, 
			newtime->tm_hour, newtime->tm_min, newtime->tm_sec,
			name.c_str(), m_levels[level].c_str(), &m_buffer[0]);
		fflush(m_file);
#endif
	}

	if (mode >= LM_ALERT)
	{
		#ifdef WIN32
		MessageBoxA(NULL, &m_buffer[0], m_levels[level].c_str(), MB_OK);
		#endif
	}		
}

void Logger::LogBinary(LogChannel channel, LogLevel level, const char *data, int len)
{
#if !defined(ALLEGRO_ANDROID) && !defined(ALLEGRO_IPHONE)
	ChannelMap::iterator iter = m_channels.find(channel);
	fprintf(m_file, "[%s] [%s]: [",
		iter->second.m_name.c_str(), m_levels[level].c_str());
	for (int i = 0; i < len; ++i)
		fprintf(m_file, "%c", data[i]);
	fprintf(m_file, "]\n");
	fflush(m_file);
#endif
}

void Logger::Log(LogChannel channel, LogLevel level, const char *fmt, ...)
{
	if ( m_channels.empty() )
		return;

	ChannelMap::iterator iter = m_channels.find(channel);
	if (iter != m_channels.end())
	{
		va_list args;
		va_start(args, fmt);
		LogV(iter->second.m_name, iter->second.m_mode, level, fmt, args);
		va_end( args );
	}
	/*else
		Log(LOGGER_OWN_CHANNEL, LL_CRITICAL, "Channel %d not found!", channel);*/
}

void Logger::Log(LogChannel channel, LogMode mode, LogLevel level, const char *fmt, ...) 
{
	ChannelMap::iterator iter = m_channels.find(channel);
	if (iter != m_channels.end())
	{
		va_list args;
		va_start(args, fmt);

		LogV(iter->second.m_name, mode, level, fmt, args);

		va_end( args );
	}
	else
		Log(LOGGER_OWN_CHANNEL, LL_CRITICAL, "Channel %d not found!", channel);
}

LogChannel Logger::RegisterChannel(LogMode mode, const std::string& name)
{
	LogChannel newChannel = static_cast<unsigned int>(m_channels.size()) + 1;
	m_channels.insert(std::make_pair(newChannel, LogChannelInfo(newChannel, name, mode)));
	return newChannel;
}

ILogger & GetLog()
{
	static Logger log;
	return log;
}