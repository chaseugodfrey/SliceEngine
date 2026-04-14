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
#include <cmath>
#include <stdlib.h> // For rand()
#include <time.h>   // For srand()

#include "ProjectSettings.h"
#include "ECS/ECSTypes.h"

namespace SliceEngine
{
	//In the future add an ID 
	struct SFXEntry
	{
		std::string key = "Default";
		FMOD::SoundGroup* soundGroup = nullptr;
		std::vector<GUID> AudioClips;
		int lastPlayed = 0;
		float volume = 1.0f;
		int maxInstances = 8;
		bool isSpatial = false;
		float spatialBlend = 1.0f;
		float minDistance = 1.0f;
		float maxDistance = 30.0f;
		AudioSource::VolumeRollOff volumeRollOff = AudioSource::VolumeRollOff::Linear;
		float _lastPlayed = -999.f;
		float minInterval = 0.f;
	};

	void to_json(nlohmann::json& j, const SFXEntry& entry);
	void from_json(const nlohmann::json& j, SFXEntry& entry);

	struct AudioSettings : public ProjectSettings
	{
		FMOD::System* mSystem = nullptr;

	public:
		
	
		std::unordered_map<std::string, SFXEntry> mSFXMap{};
	
		AudioSettings(std::string name) : ProjectSettings(name) {};
		~AudioSettings() = default;
		void Init() override;
		void Exit() override;
		void DeleteAM();
		void LoadSettings(nlohmann::json) override;
		void SaveSettings() override;

		void CreateSoundGroup(const std::string& key);
		void RemoveSoundGroup();
		void AddAudioClip(FMOD::SoundGroup* soundGroup, GUID soundGUID, std::vector<GUID>& audioClips);
		void RemoveAudioClip(std::vector<GUID>& audioClips);
		void ChangeAudioClip(const std::string& key, GUID oldSoundGUID, GUID newSoundGUID, std::vector<GUID>& audioClips);
		FMOD::SoundGroup* GetSoundGroup(const std::string& key);
		SFXEntry* GetSFXEntry(const std::string& key);
		void SetSoundGroupVolume(const std::string& key, float volume);
		const float GetSoundGroupVolume(const std::string& key);
		void RemoveFromSoundGroup(std::vector<GUID>& audioClips);
		void SetMaxInstances(const std::string& key, int maxInstances);
		const int GetMaxInstances(const std::string& key);
		void ReplaceExistingEntry(const std::string oldKey, const std::string newKey);
		void PlaySFX(const std::string& key, glm::vec3 position = glm::vec3(0.f), Entity parent = entt::null);
		Entity PlaySFXWithGO(const std::string& key, glm::vec3 position = glm::vec3(0.f), Entity parent = entt::null);
		void Release();


	};
}



#endif