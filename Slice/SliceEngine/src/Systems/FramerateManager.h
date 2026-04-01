/******************************************************************************/
/*!
\file       Time.h
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
#ifndef FRAMERATE_MANAGER_H
#define FRAMERATE_MANAGER_H

namespace SliceEngine
{

	class FramerateManager //line 21
	{

	public:
		// SliceTime funcs
		FramerateManager(); //line 75
		FramerateManager(const FramerateManager &) = delete;
		FramerateManager &operator=(const FramerateManager &) = delete;

		// Get the singleton instance
		static FramerateManager &getInstance();

		// Method to update delta_time
		void updateDeltaTime();

		// Getter to access delta_time	
		double getDeltaTime() const;

		//Setter of accumulatedTime
		void setAccumulatedTime(double time);

		//Getter of currentNumberOfSteps
		int getCurrentNumberOfSteps() const;

		//Setter of currentNumberOfSteps
		void setCurrentNumberOfSteps(int steps);

		//Increment NumberOfSteps
		void IncrementNumberOfSteps();

		//Getter of TargetFps
		float getFixedDeltaTime() const;


		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = std::chrono::time_point<Clock>;
		void Init();
		void StartFrame();
		void EndFrame();
		void StartSystem(const std::string &name);  
		void EndSystem(const std::string &name);
		//void CapFPS(int maxFPS);
		void CalculateSystemPercentages();
		float GetCurrFPS();
		//const std::unordered_map<std::string, std::pair<TimePoint, TimePoint>> GetSysStartEndTimes();
		const std::unordered_map<std::string, float> GetSysDurations();
		const std::unordered_map<std::string, float> GetPrevSysDurations();
		const std::unordered_map<std::string, float> GetSystemPercentages() const;
		const float GetFrameTime();

	private:
		std::unordered_map<std::string, std::pair<TimePoint,TimePoint>> systemStartEndTimes;
		std::unordered_map<std::string, float> systemDurations;
		std::unordered_map<std::string, float> mPreviousSystemDurations;
		std::unordered_map<std::string, float> mSystemPercentages;

		// SliceTime objs
		float deltaTime;     // Holds the delta time value
		float prevTime;     // Holds the previous time
		float targetfps;	//Refresh rate to simulate physics world( normally 60)
		float fixedDeltaTime;
		float accumulatedTime;
		int currentNumberOfSteps;


		TimePoint frameStartTime;
		TimePoint frameEndTime;
		float currFPS;
		float mTotalFrameTime;
		float mUntrackedTime;
		bool firstFrameDone = false;

	};
}

#endif