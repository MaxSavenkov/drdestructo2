#pragma once

/*
	This Logger is not very easy to use, because you have
	to register a channel before using it. Therefore, most
	systems just write to default channel, which kind of
	defeats purpose of having channels...
*/

typedef unsigned int LogChannel;

enum LogLevel
{
	LL_CRITICAL = 1,
	LL_WARNING = 2,
	LL_INFO = 3,
	LL_VERBOSE = 4
};

enum LogMode
{
	LM_FILE = 1,
	LM_ALERT = 2
};

class ILogger
{
public:
	virtual bool StartLogger( const std::string & file ) = 0;
	virtual void Log(LogChannel channel, LogLevel level, const char *fmt, ...) = 0;
	virtual void Log(LogChannel channel, LogMode mode, LogLevel level, const char *fmt, ...) = 0;
	virtual LogChannel RegisterChannel(LogMode mode, const std::string& name) = 0;
	virtual ~ILogger(){}
};

ILogger & GetLog();