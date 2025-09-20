#include <pch.h>
#include "PhysicsDebug.h"

namespace SliceEngine
{

    // Map Jolt messages to Logger levels
    static Logger::LogLevel MapJoltMessage(const char* msg)
    {
        if (strncmp(msg, "Error", 5) == 0)
            return Logger::LogLevel::ERROR;
        if (strncmp(msg, "Warning", 7) == 0)
            return Logger::LogLevel::WARNING;
        if (strncmp(msg, "Assert", 6) == 0)
            return Logger::LogLevel::CRITICAL;

        return Logger::LogLevel::DEBUG; // default
    }

    void JoltTraceImpl(const char* inFMT, ...)
    {
        va_list args;
        va_start(args, inFMT);

        Logger::LogLevel level = MapJoltMessage(inFMT);

        LogJolt("Jolt", inFMT, level, args);

        va_end(args);
    }

    void LogJolt(const char* function_name, const char* fmt, Logger::LogLevel level, va_list args)
    {
        char buffer[1024];
        std::vsnprintf(buffer, sizeof(buffer), fmt, args);
        Logger::Log(function_name, std::string(buffer), level);
    }

#ifdef JPH_ENABLE_ASSERTS

    // Callback for asserts, connect this to your own assert handler if you have one
    bool __cdecl AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, unsigned int inLine)
    {

        std::string test{ std::string(inFile) + ":" + std::to_string(inLine) + ": (" + std::string(inExpression) + ") " + (inMessage != nullptr ? std::string(inMessage) : "") };

        SLICE_LOG_CRITICAL(test);

        // Breakpoint
        return true;
    };

#endif // JPH_ENABLE_ASSERTS
}
