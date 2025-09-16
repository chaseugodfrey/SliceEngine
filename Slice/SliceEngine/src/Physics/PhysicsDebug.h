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
}

#endif //PHYSICSDEBUG_H
