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


#ifndef TIME_H
#define TIME_H

namespace SliceEngine
{
	class GameTime
	{
	private:
		double deltaTime;     // Holds the delta time value
		double prevTime;     // Holds the previous time
		double targetfps;	//Refresh rate to simulate physics world( normally 60)
		double fixedDeltaTime;
		double accumulatedTime;
		int currentNumberOfSteps;

		GameTime() : deltaTime(0.0), prevTime(glfwGetTime()), 
			targetfps(60.0), accumulatedTime(0), currentNumberOfSteps(0)	 
		{
			fixedDeltaTime = 1.0 / targetfps;
		}

	public:
		GameTime(const GameTime&) = delete;
		GameTime& operator=(const GameTime&) = delete;

		// Get the singleton instance
		static GameTime& getInstance();

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
		double getFixedDeltaTime() const;

	};
}


#endif //TIME_H