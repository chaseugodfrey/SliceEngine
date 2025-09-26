#include <pch.h>
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
		frameEndTime = Clock::now();

		float frameTime = std::chrono::duration<float, std::milli>(frameEndTime - frameStartTime).count();
		mTotalFrameTime =frameTime;

		currFPS = 1.0f / frameTime;



		std::cout << "1 frame time: " << frameTime << std::endl;

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

	const float FramerateManager::GetFrameTime()
	{
		return mTotalFrameTime;
	}

	const std::unordered_map<std::string, float> FramerateManager::GetSystemPercentages()
	{
		return mSystemPercentages;
	}

	void FramerateManager::CapFPS(int targetFPS)
	{
		using namespace std::chrono;

		auto targetFrameDuration = duration<double>(1.0 / targetFPS);

		auto currentTime = Clock::now();
		auto elapsedTime = duration<double>(currentTime - frameStartTime); 

		while (elapsedTime < targetFrameDuration)
		{
			currentTime = Clock::now();
			elapsedTime = duration<double>(currentTime - frameStartTime);
		}
	}

	void FramerateManager::CalculateSystemPercentages()
	{
		mSystemPercentages.clear();

		for (const auto [system, time] : systemDurations)
		{
			auto systemPercentage = (time / mTotalFrameTime);

			mSystemPercentages[system] = systemPercentage;
		}
	}
}