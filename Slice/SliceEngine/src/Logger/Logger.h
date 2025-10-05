/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Logger.h
 author:		Chase Roderigues
 email:			roderigues.i@digipen.edu
 brief:			Handles internal logging of data

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef LOGGER_H
#define LOGGER_H
#include <string>

// Prints out a message to terminal
#define SLICE_LOG(msg)				Logger::Log(__FUNCTION__, msg)
// Prints out a debugging message to terminal
#define SLICE_LOG_DEBUG(msg)		Logger::Log(__FUNCTION__, msg, Logger::LogLevel::DEBUG)
// Prints out a warning message to terminal
#define SLICE_LOG_WARNING(msg)		Logger::LogWarning(__FUNCTION__, msg)
// Prints out an error message to terminal
#define SLICE_LOG_ERROR(msg)		Logger::LogError(__FUNCTION__, msg)
// Prints out a critcal message to terminal
#define SLICE_LOG_CRITICAL(msg)		Logger::LogCritical(__FUNCTION__, msg)
// Prints out a value.
// This requires the values to be printed out in ostream.
#define SLICE_LOG_VALUES(...)		Logger::LogValue(__FUNCTION__, __VA_ARGS__)

namespace Logger
{
	enum class LogLevel
	{
		INFO,
		DEBUG,
		WARNING,
		ERROR,
		CRITICAL
	};

	void Log(const char* function_name, const std::string& message, LogLevel level = LogLevel::INFO);
	void LogWarning(const char* function_name, const std::string& message);
	void LogError(const char* function_name, const std::string& message);
	void LogCritical(const char* function_name, const std::string& message);

	template <typename ... Values>
	void LogValue(const char* function_name, Values ... values)
	{
		std::stringstream ss;
		((ss << values << " "), ...);
		Log(function_name, ss.str(), LogLevel::DEBUG);
		ss.clear();
	}

}

#endif