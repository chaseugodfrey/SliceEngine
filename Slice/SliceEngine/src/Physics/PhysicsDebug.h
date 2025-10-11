/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			PhysicsDebug.h
 author:		Aloysius Teo
 email:			teo.k@digipen.edu
 brief:			Handles debugging for physics systems

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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
	bool __cdecl AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, unsigned int inLine);
	#endif // JPH_ENABLE_ASSERTS
}


#endif //PHYSICSDEBUG_H

