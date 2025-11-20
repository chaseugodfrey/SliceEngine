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

	class AudioSettings
	{
		FMOD::System* mSystem = nullptr;
		//std::vector<SFXEntry> mSfxMap;
	public:
		std::unordered_map<std::string, SFXEntry> mSFXMap;
		void Init(FMOD::System* system);
		void Exit();
		void CreateSoundGroup(const std::string& key);
		void AddAudioClip(const std::string& key);
		void ChangeAudioClip(GUID oldSoundGUID, GUID newSoundGUID, const std::string& key);
		const std::string GetEntryName(const std::string& key);
		void SetEntryName(const std::string& key);
		void RemoveAudioClip(const std::string& key);
		FMOD::SoundGroup* GetSoundGroup(const std::string& key);
		SFXEntry* GetSFXEntry(const std::string& key);
		void SetSoundGroupVolume(const std::string& key, float volume);
		const float GetSoundGroupVolume(const std::string& key);
		void RemoveFromSoundGroup(std::vector<GUID>& audioClips);
		void SetMaxInstances(const std::string& key, int maxInstances);
		const int GetMaxInstances(const std::string& key);
		void SetMinIntervals(const std::string& key, float minIntervals);
		void SetMinDistance(const std::string& key, float minDistance);
		const float GetMinDistance(const std::string& key);
		void SetMaxDistance(const std::string& key, float maxDistance);
		const float GetMaxDistance(const std::string& key);
		void SetSoundGroupSpatialBlend(const std::string& key, float spatialBlend);
		const float GetSoundGroupSpatialBlend(const std::string& key);
		void SetSoundGroupSpatialBlendBool(const std::string& key, bool isSpatial);
		const bool GetSoundGroupSpatialBlendBool(const std::string& key);
		void PlaySFX(const std::string& key);
		void Release();
	};
}



#endif