/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ProfilerManager.cpp

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Defines the ProfilerManager class, which manages the data of the profiler window of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "ProfilerManager.h"
#include "ProfilerWindow.h"
#include "../Core/Registry.h"
#include <Systems/FramerateManager.h>
#include "Selection/SelectionManager.h"


namespace SliceEditor
{
	void ProfilerManager::Init()
	{
		SLICE_LOG("Initializing Profiler Data.");
	}

	void ProfilerManager::Update()
	{

	}

	std::unique_ptr<EditorWindow> ProfilerManager::CreateEditorWindow()
	{
		SLICE_LOG("Creating Profiler Window.");
		auto window = std::make_unique<ProfilerWindow>(*this, registry);

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
	
	void ProfilerManager::UpdateDebugStatistics()
	{
		mCurrFPS = SliceEngine::Core::GetInstance()->GetFramerateManager()->GetCurrFPS();
		ImVec2 canvas_size = ImGui::GetContentRegionAvail();
		
		const auto& sysPercentages = SliceEngine::Core::GetInstance()->GetFramerateManager()->GetSystemPercentages();

		for (const auto& [system, time] : SliceEngine::Core::GetInstance()->GetFramerateManager()->GetSysDurations())
		{
			ProfilerManager::DebugStats stats;

			auto it = sysPercentages.find(system);

			if(it != sysPercentages.end())
			{
				stats.width = (it->second / 100.f) * canvas_size.x;
				stats.timeTaken = time;
				stats.loadPercentage = it->second;
			}
			else
			{
				SLICE_LOG_CRITICAL("System Durations has something that Percentages does not have!");
			}

			mDebugStats.insert_or_assign(system, stats);
		}
	}

	ImU32 ProfilerManager::GetSystemColor(const std::string& systemName)
	{
		// Simple string hash function for consistent colors
		std::size_t hash = 0;
		for (char c : systemName) {
			hash = (hash * 31) + c;
		}

		// Generate RGB colors from hash
		unsigned char r = (hash & 0xFF) | 0x40;        // Ensure some red
		unsigned char g = ((hash >> 8) & 0xFF) | 0x40; // Ensure some green
		unsigned char b = ((hash >> 16) & 0xFF) | 0x40; // Ensure some blue

		return IM_COL32(r, g, b, 255);
	}

	float ProfilerManager::LuminanceCalculation(ImU32 color)
	{
		float r = static_cast<float>((color & 0xFF0000) >> 16);
		float g = static_cast<float>((color & 0x00FF00) >> 8);
		float b = static_cast<float>(color & 0x0000FF);

		r /= 255.0f;
		g /= 255.0f;
		b /= 255.0f;

		return 0.299f * r + 0.587f * g + 0.114f * b;
	}

}