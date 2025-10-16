/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Audio.cpp
 author:		Lee Yong Yee
 email:			l.yongyee@digipen.edu
 brief:			Loads Audio

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Audio.h"
#include <fstream>
#include <filesystem>

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		void Audio::InitFMODSystem()
		{
			SLICE_LOG("Initializing FMOD");
			FMOD_RESULT result = FMOD::System_Create(&mSoundSystem);
			if (result != FMOD_OK)
			{
				SLICE_LOG_ERROR("FMOD System creation failed");
				return;
			}

			result = mSoundSystem->init(MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr);
			if (result != FMOD_OK)
			{
				SLICE_LOG_ERROR("FMOD System initialization failed");
				return;
			}
		}

		FMOD::Sound* Audio::LoadAudioResource(std::string const& file)
		{
			if (!mSoundSystem)
			{
				InitFMODSystem();
			}
			return false;
		}
	}
}