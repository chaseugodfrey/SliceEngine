#include <pch.h>
#include "ProfilerManager.h"
#include "ProfilerWindow.h"


namespace SliceEditor
{
	void ProfilerManager::Init()
	{
		SLICE_LOG("Initializing Profiler Data.");
	}

	std::unique_ptr<EditorWindow> ProfilerManager::CreateWindow()
	{
		SLICE_LOG("Creating Profiler Window.");
		auto window = std::make_unique<ProfilerWindow>(*this);

		return window;
	}

	ImVec4 ProfilerManager::LogLevelToImVec4(Logger::LogLevel level)
	{
		switch (level)
		{
		case Logger::LogLevel::INFO:
			return ImVec4(1,1,1,1);
		case Logger::LogLevel::DEBUG:
			return ImVec4(0,1,0,1);
		case Logger::LogLevel::WARNING:
			return ImVec4(1,1,0,1);
		case Logger::LogLevel::ERROR:
			return ImVec4(1,0,0,1);
		case Logger::LogLevel::CRITICAL:
			return ImVec4(0,0,1,1);
		default:
			return ImVec4(1,1,1,1);
		}
	}


}