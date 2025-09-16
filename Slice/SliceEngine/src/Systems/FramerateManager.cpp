#include <pch.h>
#include <thread>
#include "FramerateManager.h"

namespace SliceEngine
{
	void FramerateManager::Init()
	{
		firstFrameDone = false;
	}

	void FramerateManager::StartFrame()
	{
		frameStartTime = Clock::now();
		systemDurations.clear();
	}

	void FramerateManager::EndFrame()
	{

		float frameTime = std::chrono::duration<float>(frameEndTime - frameStartTime).count();
		currFPS = 1.0f / frameTime;

		if (!firstFrameDone)
		{
			// can be removed if don't want it to be printed for every startup
			std::cout << "First frame time: " << frameTime * 1000.0f << " ms\n";
			for (auto &[name, duration] : systemDurations)
				std::cout << name << ": " << duration << " ms\n";

			firstFrameDone = true;
		}
	}

	void FramerateManager::StartSystem(const std::string &name)
	{
		systemStartEndTimes[name].first = Clock::now();
	}

	void FramerateManager::EndSystem(const std::string &name)
	{
		auto endTime = Clock::now();
		systemStartEndTimes[name].second = endTime;

		float duration = std::chrono::duration<float, std::milli>(endTime - systemStartEndTimes[name].first).count();
		systemDurations[name] = duration;
	}

	float FramerateManager::GetCurrFPS()
	{
		return currFPS;
	}

	const std::unordered_map<std::string, std::pair<FramerateManager::TimePoint, FramerateManager::TimePoint>> FramerateManager::GetSysStartEndTimes()
	{
		return systemStartEndTimes;
	}

	const std::unordered_map<std::string, float> FramerateManager::GetSysDurations()
	{
		return systemDurations;
	}

	void FramerateManager::CapFPS(int targetFPS)
	{
		using namespace std::chrono;

		auto targetFrameDuration = duration<double>(1.0 / targetFPS);

		auto currentTime = Clock::now();
		auto elapsedTime = duration<double>(currentTime - frameStartTime); 

		if (elapsedTime < targetFrameDuration)
		{
			auto sleepDuration = targetFrameDuration - elapsedTime;
			std::this_thread::sleep_for(sleepDuration); 
		}
		frameEndTime = Clock::now();
	}
}