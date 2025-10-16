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
		

		FMOD::Sound* Audio::LoadAudioResource(std::string const& file)
		{
			if (!mSoundSystem)
			{
				
			}
			return false;
		}
	}
}