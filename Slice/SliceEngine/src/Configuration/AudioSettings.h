/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AudioSettings.h

 author:	  Lee Yong Yee

 email:       l.yongyee@digipen.edu

 brief:		  Holds the SFX list and settings for game

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef AUDIO_SETTINGS_H
#define AUDIO_SETTINGS_H

#include <fmod.hpp>
#include "ECS/ECSTypes.h"
#include <vector>
#include <unordered_map>
#include <cmath>
#include <stdlib.h> // For rand()
#include <time.h>   // For srand()

namespace SliceEngine
{
	//In the future add an ID 
	struct SFXEntry
	{
		std::string key = "Default";
		FMOD::SoundGroup* soundGroup = nullptr;
		std::vector<GUID> AudioClips;
		float volume = 1.0f;
		int maxInstances = 8;
		bool isSpatial = false;
		float spatialBlend = 1.0f;
		float minDistance = 1.0f;
		float maxDistance = 30.0f;
		AudioSource::VolumeRollOff volumeRollOff = AudioSource::VolumeRollOff::Logarithmic;
		float _lastPlayed = -999.f;
		float minInterval = 0.f;
	};

	void to_json(nlohmann::json& j, const SFXEntry& entry);
	void from_json(const nlohmann::json& j, SFXEntry& entry);

	class AudioSettings
	{
		FMOD::System* mSystem = nullptr;

		const std::filesystem::path AUDIO_SETTINGS_PATH = std::filesystem::path("src/ProjectSettings/AudioSettings.asset");

		//std::vector<SFXEntry> mSfxMap;
	public:
		std::unordered_map<std::string, SFXEntry> mSFXMap;
		void Init();
		void Exit();
		void Serialize(const std::filesystem::path& desc_path);
		void Deserialize(const std::filesystem::path& desc_path);
		void CreateSoundGroup(const std::string& key);
		void RemoveSoundGroup();
		void AddAudioClip(FMOD::SoundGroup* soundGroup, GUID soundGUID, std::vector<GUID>& audioClips);
		void RemoveAudioClip(std::vector<GUID>& audioClips);
		void ChangeAudioClip(GUID oldSoundGUID, GUID newSoundGUID, std::vector<GUID>& audioClips);
		FMOD::SoundGroup* GetSoundGroup(const std::string& key);
		SFXEntry* GetSFXEntry(const std::string& key);
		void SetSoundGroupVolume(const std::string& key, float volume);
		const float GetSoundGroupVolume(const std::string& key);
		void RemoveFromSoundGroup(std::vector<GUID>& audioClips);
		void SetMaxInstances(const std::string& key, int maxInstances);
		const int GetMaxInstances(const std::string& key);
		void ReplaceExistingEntry(const std::string oldKey, const std::string newKey);
		void PlaySFX(const std::string& key);
		void Release();
	};
}



#endif