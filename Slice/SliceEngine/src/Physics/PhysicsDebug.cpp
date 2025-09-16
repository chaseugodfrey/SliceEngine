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

        Logger::LogJolt("Jolt", inFMT, level, args);

        va_end(args);
    }
}
