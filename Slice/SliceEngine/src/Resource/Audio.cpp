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
			auto resourceMgr = Core::GetInstance()->GetResourceManager();
			FMOD::System* mSoundSystem = mAudioManager->GetSoundSystem();

			std::filesystem::path filePath(file);

			std::string guidStr = filePath.stem().string();
			SliceEngine::GUID audioGUID = GUID::FromString(guidStr);

			std::filesystem::path metaPath;
			bool found = false;

			for (auto [key, value] : resourceMgr->mFileNameToGUID)
			{
				if (value == audioGUID)
				{
					std::filesystem::path assetBase = "../SliceEditor/Assets";
					metaPath = assetBase / (key + ".meta");
					found = true;
					break;
				}
			}

			if (!found || !std::filesystem::exists(metaPath))
			{
				SLICE_LOG_ERROR("Audio meta file not found at: " + metaPath.string());
				return false;
			}

			std::ifstream metaFile(metaPath);

			nlohmann::json metaData;

			try
			{
				// Parsing will now succeed because metaPath is valid
				metaFile >> metaData;
			}
			catch (const nlohmann::json::parse_error& e)
			{
				SLICE_LOG_ERROR("JSON Error in " + metaPath.string() + ": " + e.what());
				return false;
			}

			metaFile.close();

			SliceEditor::AudioData audioData;

			audioData.stream = metaData["stream"].get<SliceEditor::AudioStream>();
			

			
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


			SLICE_LOG("Successfully loaded audio resource " + file);
			return true;
			
		}

		FMOD::Sound* Audio::GetSound()
		{
			return sound;
		}

		void Audio::DestroyAudio()
		{
			sound->release();
		}
	}
}