/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AudioSettings.cpp

 author:	  Lee Yong Yee

 email:       l.yongyee@digipen.edu

 brief:		  Defines the functions of the Audio Settings

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
//#include "AudioManager.h"
#include "AudioSettings.h"
#include "Audio/AudioManager.h"
#include "../src/Core/Core.h"
#include "Input/InputSystem.h"
#include "Resource/ResourceManager.h"
#include "Resource/Audio.h"
#include "Serializer/JSONSerializer.h"

namespace SliceEngine
{
	void AudioSettings::Init()
	{
		mSystem = Core::GetInstance()->GetAudioManager()->GetSoundSystem();

		srand((unsigned int)time(NULL)); // Initialize random seed

		//SLICE_LOG("Current file path " + std::filesystem::current_path().string());
		Deserialize(AUDIO_SETTINGS_PATH);
	}

	void AudioSettings::DeleteAM()
	{
		auto audioManager = FactoryInstance.GetGOByName("AudioManager");

		FactoryInstance.Destroy(audioManager);
	}



	void AudioSettings::Exit()
	{
		Serialize(AUDIO_SETTINGS_PATH);
		mSystem = nullptr;
	}

	void AudioSettings::Serialize(const std::filesystem::path& desc_path)
	{
		nlohmann::json audioSettingsOutput;

		nlohmann::json sfxArray = nlohmann::json::array();

		for (const auto& [key, entry] : mSFXMap)
		{

			nlohmann::json entryJson;

			to_json(entryJson, entry);

			sfxArray.push_back(entryJson);
		}

		audioSettingsOutput["SFXEntries"] = sfxArray;

		std::ofstream outFile(desc_path);
		if (outFile.is_open())
		{
			outFile << audioSettingsOutput.dump(4);
			outFile.close();
			SLICE_LOG("AudioSettings saved to: " + desc_path.string());
		}
		else
		{
			SLICE_LOG_ERROR("Failed to open file for writing: " + desc_path.string());
		}
	}

	void AudioSettings::Deserialize(const std::filesystem::path& desc_path)
	{

		std::ifstream inFile(desc_path);
		nlohmann::json audioSettingsInput;

		if (!inFile.is_open())
		{
			SLICE_LOG_WARNING("File not found for Deserialisation!");
			return;
		}
		else
		{
			inFile >> audioSettingsInput;
			inFile.close();
		}

		if (audioSettingsInput.contains("SFXEntries"))
		{
			mSFXMap.clear();

			auto& entriesArray = audioSettingsInput["SFXEntries"];

			if (entriesArray.is_array())
			{
				for (const auto& jsonEntry : entriesArray)
				{

					SFXEntry entryData;

					from_json(jsonEntry, entryData);

					CreateSoundGroup(entryData.key);

					SFXEntry* mapEntry = GetSFXEntry(entryData.key);

					if (mapEntry)
					{
						FMOD::SoundGroup* currentSoundGroup = mapEntry->soundGroup;

						*mapEntry = entryData;

						mapEntry->soundGroup = currentSoundGroup;

						if (mapEntry->soundGroup)
						{
							mapEntry->soundGroup->setVolume(mapEntry->volume);
							mapEntry->soundGroup->setMaxAudible(mapEntry->maxInstances);
						}
					}
				}

			}

		}

	}

	void AudioSettings::CreateSoundGroup(const std::string& key)
	{
		// do while loop check instead
		if (mSFXMap.contains(key))
		{
			return;
		}

		FMOD::SoundGroup* soundGroup = nullptr;
		FMOD_RESULT soundGroupCreation = mSystem->createSoundGroup(key.c_str(), &soundGroup);

		if (soundGroupCreation != FMOD_OK)
		{
			SLICE_LOG_ERROR("Sound Group creation failed");
			return;
		}

		SFXEntry entryData{};

		entryData.key = key;

		entryData.soundGroup = soundGroup;
		mSFXMap.emplace(key, entryData);

		SLICE_LOG("Sound Group has been created");

		//->createSoundGroup(key.c_str(), soundGroup);

	}

	void AudioSettings::RemoveSoundGroup()
	{
		SFXEntry& entry = std::prev(mSFXMap.end())->second;

		//Returns all audio clips with this sound group to the master group
		entry.soundGroup->release();

		entry.soundGroup = nullptr;

		mSFXMap.erase(std::prev(mSFXMap.end()));
	}

	void AudioSettings::AddAudioClip(FMOD::SoundGroup* soundGroup, GUID soundGUID, std::vector<GUID>& audioClips)
	{

		auto audioClip = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Audio>(soundGUID).get();

		if (audioClip == nullptr)
		{
			SLICE_LOG_WARNING("Could not load GUID");
			return;
		}

		audioClip->GetSound()->setSoundGroup(soundGroup);

		
		audioClips.push_back(soundGUID);

	}

	void AudioSettings::ChangeAudioClip(const std::string& key, GUID oldSoundGUID, GUID newSoundGUID, std::vector<GUID>& audioClips)
	{

		if (oldSoundGUID == newSoundGUID)
		{
			SLICE_LOG_WARNING("ChangeAudioClip: Old and new GUIDs are identical for key '%s'. No change made.", key.c_str());
			return;
		}

		SFXEntry* entry = GetSFXEntry(key);
		if (!entry || !entry->soundGroup)
		{
			SLICE_LOG_ERROR("ChangeAudioClip: SoundGroup for key '%s' not found.", key.c_str());
			return;
		}

		/*auto resourceManager = Core::GetInstance()->GetResourceManager();
		auto oldAudioHandle = resourceManager->get<SliceEngineTypes::Audio>(oldSoundGUID).get();

		FMOD::SoundGroup* soundGroup = nullptr;

		oldAudioHandle->GetSound()->getSoundGroup(&soundGroup);*/

		bool guidReplaced = false;
		for (auto& clipGUID : audioClips)
		{
			if (clipGUID == oldSoundGUID)
			{
				clipGUID = newSoundGUID;
				guidReplaced = true;
				break;
			}
		}

		if (!guidReplaced)
		{
			SLICE_LOG_WARNING("ChangeAudioClip: Old GUID %llu not found in the audio clips list.", oldSoundGUID.GetGUID());
			return;
		}

		auto resourceManager = Core::GetInstance()->GetResourceManager();
		auto newAudioHandle = resourceManager->get<SliceEngineTypes::Audio>(newSoundGUID);

		if (!newAudioHandle.IsValid())
		{
			SLICE_LOG_ERROR("ChangeAudioClip: Failed to load new audio resource for GUID %llu.", newSoundGUID.GetGUID());
			return;
		}

		FMOD::Sound* newSound = newAudioHandle->GetSound();

		if (!newSound)
		{
			SLICE_LOG_ERROR("ChangeAudioClip: FMOD::Sound is null for new GUID %llu.", newSoundGUID.GetGUID());
			return;
		}

		newSound->setSoundGroup(entry->soundGroup);
		

	}


	void AudioSettings::ReplaceExistingEntry(const std::string oldKey, const std::string newKey)
	{
		SFXEntry* entry = GetSFXEntry(oldKey);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + oldKey + "' not found");
			return;
		}

		entry->key = newKey;

		entry->soundGroup->release();

		FMOD::SoundGroup* newGroup = nullptr;

		FMOD_RESULT newGroupCreation = mSystem->createSoundGroup(newKey.c_str(), &newGroup);

		if (newGroupCreation != FMOD_OK)
		{
			
			SLICE_LOG_ERROR("Failed to create sound group.");
			return;

		}
		
		for (auto& clip : entry->AudioClips)
		{
			auto audioSound = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Audio>(clip).get();
			if (!audioSound)
			{
				SLICE_LOG_ERROR("Failed to get audio clip for GUID.");
				return;
			}

			audioSound->GetSound()->setSoundGroup(newGroup);
		}

		entry->soundGroup = newGroup;
		auto keyToChange = mSFXMap.extract(oldKey);

		keyToChange.key() = newKey;

		mSFXMap.insert(std::move(keyToChange));

		SLICE_LOG("Entry successfully renamed");
	}

	void AudioSettings::RemoveAudioClip(std::vector<GUID>& audioClips)
	{
		
		auto audioClip = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Audio>(audioClips.back()).get();

		//Move to master sound group
		FMOD::SoundGroup* master = nullptr;
			
		mSystem->getMasterSoundGroup(&master);

		audioClip->GetSound()->setSoundGroup(master);

		audioClips.pop_back();
	}

	FMOD::SoundGroup* AudioSettings::GetSoundGroup(const std::string& key)
	{
		auto it = mSFXMap.find(key);

		if (it != mSFXMap.end())
		{
			return it->second.soundGroup;
		}

		return nullptr;
	}

	SFXEntry* AudioSettings::GetSFXEntry(const std::string& key)
	{
		auto it = mSFXMap.find(key);

		if (it != mSFXMap.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	void AudioSettings::SetSoundGroupVolume(const std::string& key, float volume)
	{

		SFXEntry* entry = GetSFXEntry(key);
		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return;
		}

		entry->volume = volume;

		if (entry->soundGroup)
		{
			entry->soundGroup->setVolume(volume);
			SLICE_LOG("Set Sound Group volume to %f",volume);
		}
	}

	const float AudioSettings::GetSoundGroupVolume(const std::string& key)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return 0.0f;
		}

		float volume = 0.0f;

		if (entry->soundGroup)
		{
			entry->soundGroup->getVolume(&volume);
		}

		return volume;
	}

	void AudioSettings::RemoveFromSoundGroup(std::vector<GUID>& audioClips)
	{
		if (!audioClips.empty())
		{
			
			audioClips.pop_back();

		}
	}

	void AudioSettings::SetMaxInstances(const std::string& key, int maxInstances)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return;
		}

		entry->maxInstances = maxInstances;

		if (entry->soundGroup)
		{
			entry->soundGroup->setMaxAudible(maxInstances);
			SLICE_LOG("Set Sound Group max instance");
		}
	}

	const int AudioSettings::GetMaxInstances(const std::string& key)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return 0;
		}

		return entry->maxInstances;
	}	

	void AudioSettings::PlaySFX(const std::string& key, glm::vec3 position)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG_ERROR("PlaySFX: SoundGroup '%s' not found", key.c_str());
			return;
		}

		if (entry->AudioClips.empty())
		{
			SLICE_LOG_WARNING("PlaySFX: SoundGroup '%s' has no audio clips defined.", key.c_str());
			return;
		}

		GUID clipGUID;
		if (entry->AudioClips.size() == 1)
		{
			// Only one clip, use it directly
			clipGUID = entry->AudioClips[0];
		}
		else
		{
			// More than one clip, select one randomly
			int randomIndex = std::rand() % entry->AudioClips.size();
			clipGUID = entry->AudioClips[randomIndex];
		}

		auto audioManagerObject = FactoryInstance.GetGOByName("AudioManager");

		auto audioObject = FactoryInstance.GetGOByName(key);

		if (audioObject.GetEntity() == entt::null)
		{
			auto newAudioObject = FactoryInstance.CreateGO(key);
			newAudioObject.AddComponent<AudioSource>();

			if (FactoryInstance.SetParent(newAudioObject.GetEntity(), audioManagerObject.GetEntity()))
			{
				AudioSource& audioComp = newAudioObject.GetComponent<AudioSource>();

				audioComp.soundGUID = clipGUID;

				// Copy volume/spatial settings from the entry to the component
				audioComp.currentVolume = entry->volume;
				audioComp.spatialBlend = entry->isSpatial ? entry->spatialBlend : 0.0f;
				audioComp.minDistance = entry->minDistance;
				audioComp.maxDistance = entry->maxDistance;
				audioComp.volumeRollOff = entry->volumeRollOff;
				audioComp.playOnAwake = false;

				

				bool isSFXPlaying = false;

				audioComp.channel->isPlaying(&isSFXPlaying);

				if (audioComp.channel == nullptr || !isSFXPlaying)
				{
				
					audioComp.channel =  audioManager->PlaySound(audioComp, position, glm::vec3{ 0.f });

				}

			}
			else
			{
				SLICE_LOG_ERROR("No AudioManager object in scene");
				return;
			}

		}
		else
		{
			AudioSource& audioComp = audioObject.GetComponent<AudioSource>();

			auto& transform = audioObject.GetComponent<Transform>();

			audioComp.soundGUID = clipGUID;

			bool isSFXPlaying = false;

			audioComp.channel->isPlaying(&isSFXPlaying);

			if ((audioComp.channel == nullptr || !isSFXPlaying))
			{

				audioComp.channel = audioManager->PlaySound(audioComp, position, glm::vec3{ 0.f });

			}
		}


		//entry->_lastPlayed = currentTime;
	}

	void to_json(nlohmann::json& j, const SFXEntry& entry)
	{
		j["key"] = entry.key;
		j["volume"] = entry.volume;
		j["maxInstances"] = entry.maxInstances;
		j["isSpatial"] = entry.isSpatial;
		j["spatialBlend"] = entry.spatialBlend;
		j["minDistance"] = entry.minDistance;
		j["maxDistance"] = entry.maxDistance;
		j["minInterval"] = entry.minInterval;
		j["volumeRollOff"] = static_cast<int>(entry.volumeRollOff);

		j["AudioClips"] = nlohmann::json::array();

		for (auto& clips : entry.AudioClips)
		{
			nlohmann::json tempJson;

			to_json(tempJson, clips);

			j["AudioClips"].push_back(tempJson);

		}
	}

	//from_json

	void from_json(const nlohmann::json& j, SFXEntry& entry)
	{
		if (j.contains("key"))
		{
			j.at("key").get_to(entry.key);
		}
		if (j.contains("volume"))
		{
			j.at("volume").get_to(entry.volume);
		}
		if (j.contains("maxInstances"))
		{
			j.at("maxInstances").get_to(entry.maxInstances);
		}
		if (j.contains("isSpatial"))
		{
			j.at("isSpatial").get_to(entry.isSpatial);
		}
		if (j.contains("spatialBlend"))
		{
			j.at("spatialBlend").get_to(entry.spatialBlend);
		}
		if (j.contains("minDistance"))
		{
			j.at("minDistance").get_to(entry.minDistance);
		}
		if (j.contains("maxDistance"))
		{
			j.at("maxDistance").get_to(entry.maxDistance);
		}
		if (j.contains("minInterval"))
		{
			j.at("minInterval").get_to(entry.minInterval);
		}

		if (j.contains("volumeRollOff"))
		{
			int rollOff = 0;
			j.at("volumeRollOff").get_to(rollOff);
			entry.volumeRollOff = static_cast<AudioSource::VolumeRollOff>(rollOff);
		}

		entry.AudioClips.clear();

		const auto& audioClipsJsons = j.at("AudioClips");

		for (const auto& audioClipsJson : audioClipsJsons)
		{
			GUID audioClip = (GUID)audioClipsJson.get<uint64_t>();
			//from_json(audioClipsJsons, audioClip);
			entry.AudioClips.push_back(audioClip);
		}

		/*if (j.contains("AudioClips"))
		{
			j.at("AudioClips").get_to(entry.AudioClips);
		}*/

		// Initialize runtime defaults
		entry.soundGroup = nullptr;
		entry._lastPlayed = -999.0f;
	}
}