#ifndef PHYSICSDEBUG_H
#define PHYSICSDEBUG_H

#include <cstdarg>
#include <cstring>

namespace SliceEngine
{
	// Map Jolt messages to our Logger levels
	static Logger::LogLevel MapJoltMessage(const char* msg);

	// Trace implementation
	void JoltTraceImpl(const char* inFMT, ...);

	void LogJolt(const char* function_name, const char* fmt, Logger::LogLevel level, va_list args);
}

#endif //PHYSICSDEBUG_H
