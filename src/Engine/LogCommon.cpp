#include "stdafx.h"
#include "LogCommon.h"

LogChannel CommonLog()
{
	static LogChannel channel = GetLog().RegisterChannel( LM_FILE, "Game" );
	return channel;
}