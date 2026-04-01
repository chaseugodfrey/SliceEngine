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
		mAverageTotalFrameTime.totalSum = 0.0f;
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
		auto engineFRM = SliceEngine::Core::GetInstance()->GetFramerateManager();
		float dt = static_cast<float>(engineFRM->getDeltaTime());
		float currentTotalFrameTime = engineFRM->GetFrameTime();
		ImVec2 canvas_size = ImGui::GetContentRegionAvail();
		const auto& currentDurations = engineFRM->GetPrevSysDurations();
		for (const auto& [system, time] : currentDurations)
		{
			if (time <= FLT_EPSILON)
			{
				SLICE_LOG_ERROR("This continue should not happen...");
				continue;
			}
			SystemHistory& history = mSystemMap[system];

			//Add new entry
			history.samples.push_back(time);
			history.totalSum += time;

			if (history.samples.size() > MAX_SAMPLES)
			{
				history.totalSum -= history.samples.front();
				history.samples.pop_front();
			}
		}


		mAverageTotalFrameTime.samples.push_back(currentTotalFrameTime);
		mAverageTotalFrameTime.totalSum += currentTotalFrameTime;

		if (mAverageTotalFrameTime.samples.size() > MAX_SAMPLES)
		{
			mAverageTotalFrameTime.totalSum -= mAverageTotalFrameTime.samples.front();
			mAverageTotalFrameTime.samples.pop_front();
		}
		
		static float updateTimer = 0.0f;
		const float updateInterval = 1.0f;

		updateTimer += dt;
		float trackedTime = 0.0f;
		if(updateTimer >= updateInterval)
		{
			updateTimer = 0;
			// 1. Get the real average total frame time (the 7ms)
			float actualAvgTotalFrameTime = mAverageTotalFrameTime.totalSum / mAverageTotalFrameTime.samples.size();

			// 2. Sum up the average of all tracked systems (the 4ms)
			float totalTrackedAvg = 0.0f;
			for (auto& [system, history] : mSystemMap)
			{
				totalTrackedAvg += (history.totalSum / history.samples.size());
			}
			//Check each system's percentage.
			for(auto& [system, history] : mSystemMap)
			{
				float averageTime = history.totalSum / history.samples.size();
				float averagePercentage = (actualAvgTotalFrameTime > 0) ? (averageTime / actualAvgTotalFrameTime) * 100.0f : 0.0f;

				ProfilerManager::DebugStats stats;

				stats.timeTaken = averageTime;
				stats.loadPercentage = averagePercentage;
				stats.width = (averagePercentage / 100.f) * canvas_size.x;
				mDebugStats.insert_or_assign(system, stats);
				trackedTime += averageTime;
			}
			mCurrFPS = engineFRM->GetCurrFPS();
			mDeltaTime = static_cast<float>(engineFRM->getDeltaTime());
			mTotalFrameTime = actualAvgTotalFrameTime;
			mUntrackedFrameTime = actualAvgTotalFrameTime - trackedTime;
			if(mUntrackedFrameTime > 0)
			{
				mUntrackedFrameTimePercentage = (mUntrackedFrameTime / actualAvgTotalFrameTime) * 100.0f;
			}
			else
			{
				mUntrackedFrameTimePercentage = 0.0f;
			}
		}
	}

	void ProfilerManager::ClearDebugStatistics()
	{
		mSystemMap.clear();
		mDebugStats.clear();
		mCurrFPS = 0.0f;
		mDeltaTime = 0.0f;
		mTotalFrameTime = 0.0f;
		mUntrackedFrameTime = 0.0f;
		mUntrackedFrameTimePercentage = 0.0f;
		mAverageTotalFrameTime.samples.clear();
		mAverageTotalFrameTime.totalSum = 0.0f;
		mClearStatistics = false;
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