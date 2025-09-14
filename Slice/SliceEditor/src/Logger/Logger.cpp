#include <pch.h>
#include "Logger.h"

namespace Logger
{
	#define RESET   "\033[0m"
	#define BLACK   "\033[30m"      /* Black */
	#define RED     "\033[31m"      /* Red */
	#define GREEN   "\033[32m"      /* Green */
	#define YELLOW  "\033[33m"      /* Yellow */
	#define BLUE    "\033[34m"      /* Blue */

	// early declaration
	static const char* LogLevelToString(LogLevel level);
	static const char* LogLevelToColor(LogLevel level);

	void Log(const char* function_name, const std::string& message, LogLevel level)
	{
		auto now = std::chrono::system_clock::now();
		std::time_t now_c = std::chrono::system_clock::to_time_t(now);
		std::tm local_tm{};
		localtime_s(&local_tm, &now_c);

		std::cout
			<< std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S")
			<< LogLevelToColor(level) << " [" << LogLevelToString(level) << "] " << RESET
			<< '(' << function_name << ") - "
			<< message << '\n';
	}

	void LogWarning(const char* function_name, const std::string& message)
	{
		Log(function_name, message, LogLevel::WARNING);
	}

	void LogError(const char* function_name, const std::string& message)
	{
		Log(function_name, message, LogLevel::ERROR);
	}

	void LogCritical(const char* function_name, const std::string& message)
	{
		Log(function_name, message, LogLevel::CRITICAL);
	}

	static const char* LogLevelToString(LogLevel level)
	{
		switch (level)
		{
		case Logger::LogLevel::INFO:
			return "INFO";
		case Logger::LogLevel::DEBUG:
			return "DEBUG";
		case Logger::LogLevel::WARNING:
			return "WARNING";
		case Logger::LogLevel::ERROR:
			return "ERROR";
		case Logger::LogLevel::CRITICAL:
			return "CRITICAL";
		default: 
			return "INFO";
		}
	}

	static const char* LogLevelToColor(LogLevel level)
	{
		switch (level)
		{
		case Logger::LogLevel::INFO:
			return RESET;
		case Logger::LogLevel::DEBUG:
			return GREEN;
		case Logger::LogLevel::WARNING:
			return YELLOW;
		case Logger::LogLevel::ERROR:
			return RED;
		case Logger::LogLevel::CRITICAL:
			return BLUE;
		default:
			return RESET;
		}
	}
}
