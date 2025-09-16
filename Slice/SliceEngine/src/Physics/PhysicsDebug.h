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

	#ifdef JPH_ENABLE_ASSERTS
	//Assert faile Implementation
	bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine);
	#endif // JPH_ENABLE_ASSERTS
}


#endif //PHYSICSDEBUG_H

