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
#include "../../SliceEditor/src/AssetManager/AssetTypes.h"

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		

		bool Audio::LoadAudioResource(std::string const& file)
		{
			auto mAudioManager = Core::GetInstance()->GetAudioManager();

			FMOD::System* mSoundSystem = mAudioManager->GetSoundSystem();

			std::filesystem::path filePath(file);

			std::filesystem::path metaPath = filePath.replace_extension(".meta");

			std::ifstream metaFile(metaPath);

			nlohmann::json metaData;

			metaFile >> metaData;

			metaFile.close();

			SliceEditor::AudioData audioData;

			audioData.stream = metaData["stream"].get<SliceEditor::AudioStream>();
			audioData.dimension = metaData["dimension"].get<SliceEditor::AudioDimension>();
			audioData.category = metaData["category"].get<SliceEditor::AudioCategory>();

			
			FMOD_MODE mode = FMOD_DEFAULT;

			if (audioData.stream == SliceEditor::AudioStream::CREATE_SAMPLE)
			{
				mode |= FMOD_CREATESAMPLE;
			}
			else
			{
				mode |= FMOD_CREATESTREAM;
			}

			mode |= FMOD_3D;
			
			FMOD_RESULT result = mSoundSystem->createSound(file.c_str(), mode, nullptr, &sound);
			if (result != FMOD_OK)
			{
				SLICE_LOG_ERROR("FMOD failed to audio resource from '" + file + "'. Error: " + std::to_string(result));

				sound = nullptr;

				return false;
			}

			

			if (audioData.dimension == SliceEditor::AudioDimension::FMOD2D)
			{
				sound->set3DMinMaxDistance( 0.0f,0.0f );
				is3D = false;
			}
			else
			{
				sound->set3DMinMaxDistance(1.0f, 20.0f);
				is3D = true;
			}

			category = static_cast<int>(audioData.category);

			SLICE_LOG("Successfully loaded audio resource " + file);
			return true;
			
		}

		FMOD::Sound* Audio::GetSound()
		{
			return sound;
		}

		int Audio::GetCategory()
		{
			return category;
		}

		bool Audio::GetDimension()
		{
			return is3D;
		}

		void Audio::DestroyAudio()
		{
			sound->release();
		}
	}
}