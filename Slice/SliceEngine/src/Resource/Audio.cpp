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
#include "../Core/Core.h"
#include "../Audio/AudioManager.h"

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		

		bool Audio::LoadAudioResource(std::string const& file)
		{
			auto mAudioManager = Core::GetInstance()->GetAudioManager();

			FMOD::System* mSoundSystem = mAudioManager->GetSoundSystem();

			
			FMOD_RESULT result = mSoundSystem->createSound(file.c_str(), FMOD_3D, nullptr, &sound);
			if (result != FMOD_OK)
			{
				SLICE_LOG_ERROR("FMOD failed to audio resource from '" + file + "'. Error: " + std::to_string(result));

				sound = nullptr;

				return false;
			}

			SLICE_LOG("Successfully loaded audio resource " + file);
			return true;
			
		}

		FMOD::Sound* Audio::GetSound()
		{
			return sound;
		}
	}
}