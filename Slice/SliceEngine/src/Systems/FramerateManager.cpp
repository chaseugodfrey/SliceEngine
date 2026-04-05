/******************************************************************************/
/*!
\file       Time.cpp
\author     Teo Kok Chin Aloysius (teo.k@digipen.edu)
\date       Mon day year
\brief      Implementation of time-related functionality for the project.
			This file contains the implementation of the GameTime class, which
			handles delta time calculation.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/
#include <pch.h>
#include "FramerateManager.h"

namespace SliceEngine
{
	FramerateManager::FramerateManager() : deltaTime(0.0), prevTime(static_cast<float>(glfwGetTime())),
		targetfps(60.0), accumulatedTime(0), currentNumberOfSteps(0), currFPS(0.0f) , mUntrackedTime(0.0f)
	{
		fixedDeltaTime = 1.0f / targetfps;
	}

	FramerateManager &FramerateManager::getInstance()
	{
		static FramerateManager instance;
		return instance;
	}

	/******************************************************************************/
	/*!
	\brief      Updates the delta time and calculates FPS.
				This method calculates the time elapsed since the last frame,
				updates the delta time, and recalculates the FPS every second.

	\return     None.
	*/
	/******************************************************************************/
	void FramerateManager::updateDeltaTime()
	{
		currentNumberOfSteps = 0;
		float curr_time =static_cast<float>(glfwGetTime());       // Get the current time
		deltaTime = curr_time - prevTime;	    // Calculate delta time
		prevTime = curr_time;                  // Update prev_time to the current time

		if (deltaTime > 0.25f) 
		{
			deltaTime = 0.25f;
		}

		accumulatedTime += deltaTime;

		//if (accumulatedTime >= fixedDeltaTime * 8.0f)
		//{
		//	accumulatedTime = 0.0f;
		//}

		while (accumulatedTime >= fixedDeltaTime)
		{
			accumulatedTime -= fixedDeltaTime;
			++currentNumberOfSteps;
		}
	}
	/******************************************************************************/
	/*!
	\brief      Retrieves the current delta time.

	\return     double - The time elapsed since the last frame in seconds.
	*/
	/******************************************************************************/
	double FramerateManager::getDeltaTime() const
	{
		return deltaTime;
	}

	int FramerateManager::getCurrentNumberOfSteps() const
	{
		return currentNumberOfSteps;
	}

	float FramerateManager::getFixedDeltaTime() const
	{
		return static_cast<float>(fixedDeltaTime);
	}

	void FramerateManager::setAccumulatedTime(double t)
	{
		accumulatedTime = static_cast<float>(t);
	}

	void FramerateManager::setCurrentNumberOfSteps(int steps)
	{
		currentNumberOfSteps = steps;
	}

	void FramerateManager::IncrementNumberOfSteps()
	{
		++currentNumberOfSteps;
	}

	void FramerateManager::Init()
	{
		firstFrameDone = false;
	}

	void FramerateManager::StartFrame()
	{
		frameStartTime = Clock::now();
		systemDurations.clear();
		systemStartEndTimes.clear();
	}

	void FramerateManager::EndFrame()
	{
		frameEndTime = Clock::now();

		float frameTime = std::chrono::duration<float, std::milli>(frameEndTime - frameStartTime).count();
		mTotalFrameTime = frameTime;

		currFPS = (frameTime > 0.0f) ? (1000.0f / frameTime) : 0.0f;
		mPreviousSystemDurations = systemDurations;
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
		systemDurations[name] += duration;
	}

	float FramerateManager::GetCurrFPS()
	{
		return currFPS;
	}

	//const std::unordered_map<std::string, std::pair<FramerateManager::TimePoint, FramerateManager::TimePoint>> FramerateManager::GetSysStartEndTimes()
	//{
	//	return systemStartEndTimes;
	//}

	const std::unordered_map<std::string, float> FramerateManager::GetSysDurations()
	{
		return systemDurations;
	}

	const std::unordered_map<std::string, float> FramerateManager::GetPrevSysDurations()
	{
		return mPreviousSystemDurations;
	}

	const float FramerateManager::GetFrameTime()
	{
		return mTotalFrameTime;
	}

	const std::unordered_map<std::string, float> FramerateManager::GetSystemPercentages() const
	{
		return mSystemPercentages;
	}

	/*void FramerateManager::CapFPS(int targetFPS)
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
	}*/

	void FramerateManager::CalculateSystemPercentages()
	{
		mSystemPercentages.clear();
		float trackedTime = 0.0f;

		for (const auto [system, time] : systemDurations)
		{
			auto systemPercentage = (time / mTotalFrameTime) * 100.0f;

			mSystemPercentages[system] = systemPercentage;
			trackedTime += time;
		}
		mUntrackedTime = mTotalFrameTime - trackedTime;
		//SLICE_LOG("Untracked Time on FRM Side: " + std::to_string(mUntrackedTime));
	}
}