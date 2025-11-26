/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        Logger.cpp

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the Logger class, which is responsible for logging messages to the terminal.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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
	//static const char* LogLevelToString(LogLevel level);
	static const char* LogLevelToColor(LogLevel level);
	std::deque <std::pair<LogLevel, std::string>> engineLogs;
	std::deque <std::pair<LogLevel, ConsoleMessage>> consoleLogs;
	std::mutex _stdoutMutex{};

	void Log(const char* function_name, const std::string& message, LogLevel level)
	{
		auto now = std::chrono::system_clock::now();
		std::time_t now_c = std::chrono::system_clock::to_time_t(now);
		std::tm local_tm{};
		localtime_s(&local_tm, &now_c);
		std::ostringstream oss;
		oss /*<< std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S")
			<< " [" << LogLevelToString(level) << "] "*/
			<< '(' << function_name << ") - "
			<< message << '\n';

		std::lock_guard<std::mutex> LoggersLock{ _stdoutMutex };
		std::cout
			<< std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S")
			<< LogLevelToColor(level) << " [" << LogLevelToString(level) << "] " << RESET
			<< '(' << function_name << ") - "
			<< message << '\n';


		

		std::string newMessage = oss.str();

		if (engineLogs.size() == 2000)
		{
			engineLogs.pop_front();
		}

		engineLogs.push_back(std::pair(level, newMessage));
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

	void LogConsole(int level, ConsoleMessage&& message)
	{
		LogLevel lvl = static_cast<LogLevel>(level);
		consoleLogs.push_back({ lvl, std::move(message) });
	}

	const char* LogLevelToString(LogLevel level)
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
