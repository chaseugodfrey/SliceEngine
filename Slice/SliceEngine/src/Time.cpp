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

#include "pch.h"
#include "Time.h"

#include <chrono>
#include <thread>


namespace SliceEngine
{
	// Get the singleton instance
	GameTime& GameTime::getInstance()
	{
		static GameTime instance;
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
	void GameTime::updateDeltaTime()
	{
		currentNumberOfSteps = 0;
		double curr_time = glfwGetTime();       // Get the current time
		deltaTime = curr_time - prevTime;	    // Calculate delta time
		prevTime = curr_time;                  // Update prev_time to the current time

		accumulatedTime += deltaTime;

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
	double GameTime::getDeltaTime() const
	{
		return deltaTime;
	}

	void GameTime::setAccumulatedTime(double time)
	{
		accumulatedTime = time;
	}

	int GameTime::getCurrentNumberOfSteps() const
	{
		return currentNumberOfSteps;
	}

	void GameTime::setCurrentNumberOfSteps(int steps)
	{
		currentNumberOfSteps = steps;
	}

	void GameTime::IncrementNumberOfSteps()
	{
		++currentNumberOfSteps;
	}

	double GameTime::getFixedDeltaTime() const
	{
		return fixedDeltaTime;
	}

}


