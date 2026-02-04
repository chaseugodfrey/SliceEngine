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
            // ResourceManager is only needed if we are looking for meta files
            auto resourceMgr = Core::GetInstance()->GetResourceManager();
            FMOD::System* mSoundSystem = mAudioManager->GetSoundSystem();

            std::filesystem::path filePath(file);

           
            bool shouldStream = false;
            bool metaFoundAndLoaded = false;

            
            std::string guidStr = filePath.stem().string();
            SliceEngine::GUID audioGUID = GUID::FromString(guidStr);

            std::filesystem::path metaPath;

            if (!resourceMgr->mFileNameToGUID.empty())
            {
                for (auto [key, value] : resourceMgr->mFileNameToGUID)
                {
                    if (value == audioGUID)
                    {
                        
                        std::filesystem::path assetBase = "../SliceEditor/Assets";
                        metaPath = assetBase / (key + ".meta");

                        if (std::filesystem::exists(metaPath))
                        {
                            std::ifstream metaFile(metaPath);
                            nlohmann::json metaData;
                            try
                            {
                                metaFile >> metaData;
                                auto streamType = metaData["stream"].get<SliceEditor::AudioStream>();

                                if (streamType == SliceEditor::AudioStream::CREATE_SAMPLE)
                                    shouldStream = false;
                                else
                                    shouldStream = true;

                                metaFoundAndLoaded = true;

                                
                            }
                            catch (...)
                            {
                                
                                SLICE_LOG_WARNING("JSON Error in meta file, falling back to heuristic: " + metaPath.string());
                            }

                            metaFile.close();
                        }
                        break;
                    }
                }
            }

           
            if (!metaFoundAndLoaded)
            {
                if (!std::filesystem::exists(filePath))
                {
                    SLICE_LOG_ERROR("Audio file not found: " + file);
                    return false;
                }

                try
                {
                   
                    const uintmax_t STREAM_THRESHOLD = 512 * 1024;
                    if (std::filesystem::file_size(filePath) > STREAM_THRESHOLD)
                    {
                        shouldStream = true;
                    }
                    else
                    {
                        shouldStream = false;
                    }
                }
                catch (std::filesystem::filesystem_error& e)
                {
                    SLICE_LOG_ERROR("Filesystem error checking size: " + std::string(e.what()));
                    return false;
                }
            }

           
            FMOD_MODE mode = FMOD_DEFAULT;

            if (shouldStream)
            {
                mode |= FMOD_CREATESTREAM;
            }
            else
            {
                mode |= FMOD_CREATESAMPLE;
            }

            mode |= FMOD_3D;

            FMOD_RESULT result = mSoundSystem->createSound(file.c_str(), mode, nullptr, &sound);
            if (result != FMOD_OK)
            {
                SLICE_LOG_ERROR("FMOD failed to load audio resource from '" + file + "'. Error: " + std::to_string(result));
                sound = nullptr;
                return false;
            }

            SLICE_LOG("Successfully loaded audio resource: " + file + (shouldStream ? " [Stream]" : " [Sample]"));
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