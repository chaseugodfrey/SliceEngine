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

namespace SliceEngine
{
	void AudioSettings::Init(FMOD::System* system)
	{
		mSystem = system;

		srand((unsigned int)time(NULL)); // Initialize random seed
	}

	void AudioSettings::Exit()
	{
		mSystem = nullptr;
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
		mSFXMap.erase(mSFXMap.end());
	}

	void AudioSettings::AddAudioClip(const std::string& key)
	{

		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return;
		}

		
		entry->AudioClips.push_back(entry->AudioClips.back());

	}

	void AudioSettings::ChangeAudioClip(GUID oldSoundGUID, GUID newSoundGUID, std::vector<GUID>& audioClips)
	{

		if (oldSoundGUID == newSoundGUID)
		{
			SLICE_LOG_WARNING("ChangeAudioClip: Old and new GUIDs are identical for key '%s'. No change made.", key.c_str());
			return;
		}

		

		for (auto& it : audioClips)
		{
			if (it == oldSoundGUID)
			{
				it = newSoundGUID;
			}
		}
	
	}

	const std::string AudioSettings::GetEntryName(const std::string& key)
	{
		
	}

	void AudioSettings::SetEntryName(const std::string& key)
	{
	}

	void AudioSettings::ReplaceExistingEntry(const std::string oldKey, const std::string newKey)
	{
		auto keyToChange = mSFXMap.extract(oldKey);
		SFXEntry* entry = GetSFXEntry(oldKey);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + oldKey + "' not found");
			return;
		}

		entry->key = newKey;

		keyToChange.key() = newKey;

		mSFXMap.insert(std::move(keyToChange));

	}

	void AudioSettings::RemoveAudioClip(const std::string& key)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return;
		}

		entry->AudioClips.pop_back();
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
			SLICE_LOG("Set Sound Group max instance to %s", maxInstances);
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

	void AudioSettings::SetMinIntervals(const std::string& key, float minIntervals)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return;
		}

		entry->minInterval = minIntervals;
	}

	void AudioSettings::SetMinDistance(const std::string& key, float minDistance)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return;
		}

		entry->minDistance = minDistance;
		SLICE_LOG("Set Sound Group min distance to %f", minDistance);
	}

	const float AudioSettings::GetMinDistance(const std::string& key)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return 0.0f;
		}

		return entry->minDistance;
	}

	void AudioSettings::SetMaxDistance(const std::string& key, float maxDistance)
	{

		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return;
		}

		entry->maxDistance = maxDistance;
		SLICE_LOG("Set Sound Group max distance to %f", maxDistance);
	}

	const float AudioSettings::GetMaxDistance(const std::string& key)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return 0.0f;
		}

		return entry->maxDistance;
	}

	void AudioSettings::SetSoundGroupSpatialBlend(const std::string& key, float spatialBlend)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return;
		}

		entry->spatialBlend = spatialBlend;
		SLICE_LOG("Set Sound Group spatial blend to %f", spatialBlend);
	}

	const float AudioSettings::GetSoundGroupSpatialBlend(const std::string& key)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return 0.0f;
		}

		return entry->spatialBlend;
	}

	void AudioSettings::SetSoundGroupSpatialBlendBool(const std::string& key, bool isSpatial)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return;
		}

		entry->isSpatial = isSpatial;
	}

	const bool AudioSettings::GetSoundGroupSpatialBlendBool(const std::string& key)
	{
		SFXEntry* entry = GetSFXEntry(key);

		if (!entry)
		{
			SLICE_LOG("SoundGroup '" + key + "' not found");
			return false;
		}

		return entry->isSpatial;
	}

	void AudioSettings::PlaySFX(const std::string& key)
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

		auto audioObject = Core::FactoryInstance.CreateGO(key);
		audioObject.AddComponent<AudioSource>();
		AudioSource& audioComp = audioObject.GetComponent<AudioSource>();

		// Find the Transform component
		auto& transform = audioObject.GetComponent<Transform>();

		audioComp.soundGUID = clipGUID;

		// Copy volume/spatial settings from the entry to the component
		audioComp.currentVolume = entry->volume;
		audioComp.spatialBlend = entry->isSpatial ? entry->spatialBlend : 0.0f;
		audioComp.minDistance = entry->minDistance;
		audioComp.maxDistance = entry->maxDistance;
		audioComp.volumeRollOff = entry->volumeRollOff;
		audioComp.playOnAwake = true;

		//audioComp.channel =  audioManager->PlaySound(audioComp, transform.position, glm::vec3{ 0.f });

		//entry->_lastPlayed = currentTime;
	}
}