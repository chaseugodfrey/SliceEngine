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
#define SLICE_LOG(...)				Logger::LogValue(__FUNCTION__, Logger::LogLevel::INFO, __VA_ARGS__)
// Prints out a debugging message to terminal
#define SLICE_LOG_DEBUG(...)		Logger::LogValue(__FUNCTION__, Logger::LogLevel::DEBUG, __VA_ARGS__)
// Prints out a warning message to terminal
#define SLICE_LOG_WARNING(...)		Logger::LogValue(__FUNCTION__, Logger::LogLevel::WARNING, __VA_ARGS__)
// Prints out an error message to terminal
#define SLICE_LOG_ERROR(...)		Logger::LogValue(__FUNCTION__, Logger::LogLevel::ERROR, __VA_ARGS__)
// Prints out a critcal message to terminal
#define SLICE_LOG_CRITICAL(...)		Logger::LogValue(__FUNCTION__, Logger::LogLevel::CRITICAL, __VA_ARGS__)
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

	template <typename ... Values>
	void LogValue(const char* function_name, LogLevel log_level, Values ... values)
	{
		std::stringstream ss;
		((ss << values << " "), ...);
		switch (log_level)
		{
		case LogLevel::INFO:
			Logger::Log(function_name, ss.str());
			break;
		case LogLevel::DEBUG:	
			Logger::Log(function_name, ss.str(), LogLevel::DEBUG);
			break;
		case LogLevel::WARNING:	
			Logger::LogWarning(function_name, ss.str());
			break;
		case LogLevel::ERROR:	
			Logger::LogError(function_name, ss.str());
			break;
		case LogLevel::CRITICAL:
			Logger::LogCritical(function_name, ss.str());
			break;
		}
		ss.clear();
	}
}

#endif