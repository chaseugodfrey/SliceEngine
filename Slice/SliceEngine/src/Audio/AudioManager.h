/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AudioManager.h

 author:	  Lee Yong Yee

 email:       l.yongyee@digipen.edu

 brief:		  Declares the AudioManager class and related audio structures for handling sound playback
              within the engine using the FMOD sound library. This system manages loading, playing,
              and updating 2D and 3D sounds, maintaining category-based volume control, and handling
              sound states such as pause, looping, and positional audio.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <fmod.hpp>
#include "ECS/ECSTypes.h"
#include <vector>
#include <unordered_map>
#include <cmath>
#include <stdlib.h> // For rand()
#include <time.h>   // For srand()

namespace SliceEngine
{
	//struct SFXEntry
	//{
	//	std::string key = "Default";
	//	FMOD::SoundGroup* soundGroup = nullptr;
	//	std::vector<GUID> AudioClips;
	//	float volume = 1.0f;
	//	int maxInstances = 8;
	//	bool isSpatial = false;
	//	float spatialBlend = 1.0f;
	//	float minDistance = 1.0f;
	//	float maxDistance = 30.0f;
	//	AudioSource::VolumeRollOff volumeRollOff = AudioSource::VolumeRollOff::Logarithmic;
	//	float _lastPlayed = -999.f;
	//	float minInterval = 0.f;
	//};

	//class AudioSettings
	//{
	//	FMOD::System* mSystem = nullptr;
	//	//std::vector<SFXEntry> mSfxMap;
	//public:
	//	std::unordered_map<std::string, SFXEntry> mSFXMap;
	//	void Init(FMOD::System* system);
	//	void Exit();
	//	void CreateSoundGroup(const std::string& key);
	//	void SetSoundGroup(std::string soundName, const std::string& key);
	//	FMOD::SoundGroup* GetSoundGroup(const std::string& key);
	//	SFXEntry* GetSFXEntry(const std::string& key);
	//	void SetSoundGroupVolume(const std::string& key, float volume);
	//	const float GetSoundGroupVolume(const std::string& key);
	//	void SetMaxInstances(const std::string& key, int maxInstances);
	//	const int GetMaxInstances(const std::string& key);
	//	void SetMinIntervals(const std::string& key, float minIntervals);
	//	void SetMinDistance(const std::string& key, float minDistance);
	//	void SetMaxDistance(const std::string& key, float maxDistance);
	//	void SetSoundGroupSpatialBlend(const std::string& key, float spatialBlend);
	//	const float GetSoundGroupSpatialBlend(const std::string& key);
	//	void SetSoundGroupSpatialBlendBool(const std::string& key, bool isSpatial);
	//	const bool GetSoundGroupSpatialBlendBool(const std::string& key);
	//	void PlaySFX();
	//	void Release();
	//};
	//

	class AudioManager
	{


		//FMOD::Studio::System* system;
		FMOD::System* mSoundSystem;
		FMOD::ChannelGroup* master;
		FMOD::ChannelGroup* sfx;
		FMOD::ChannelGroup* bgm;
		FMOD::ChannelGroup* ui;
		FMOD::ChannelGroup* editorSounds;
		
		//std::unordered_map<std::string, FMOD::SoundGroup*> mSoundGroups;
		
		const int MAX_CHANNELS = 20;

		const float defaultVolume = 1.0f;
		


	public:
		/** @brief Initializes the FMOD audio system and prepares it for sound playback. */
		void Init();

		/** @brief Updates the audio system each frame, processing sound playback and cleanup. */
		void Update();

		/** @brief Shuts down and releases all FMOD resources. */
		void Exit();

		FMOD::System* GetSoundSystem();

		/**
		 * @brief Converts a glm::vec3 to an FMOD_VECTOR.
		 */
		inline FMOD_VECTOR Vec3ToFMODVec3(glm::vec3 vector)
		{
			return { vector.x,vector.y,vector.z };
		}

		/**
		 * @brief Converts an FMOD_VECTOR to a glm::vec3.
		 */
		inline glm::vec3 FMODVec3ToVec3(FMOD_VECTOR vector)
		{
			return { vector.x, vector.y, vector.z };
		}

		/**
		 * @brief Plays a sound associated with a specific entity.
		 */
		FMOD::Channel* PlaySound(const AudioSource& audioComp, glm::vec3 soundPos, glm::vec3 vel);

		/**
		 * @brief Plays a sound preview for the editor without affecting in-game channels.
		 */
		FMOD::Channel* PlayEditorPreview(const AudioSource& audioComp);

		/**
		 * @brief Updates the listener's 3D attributes (position, velocity, orientation).
		 */
		void SetListenerAttributes(glm::vec3& pos, glm::vec3& vel, glm::vec3& forward, glm::vec3& up);

		void SetChannelPriority(FMOD::Channel* channel, int priorityNumber);

		void SetOcclusion(FMOD::Channel* channel, float direct, float reverb);
		std::pair<float, float> GetOcclusion(FMOD::Channel* channel);

		void SetSpread(FMOD::Channel* channel, float spread);
		float GetSpread(FMOD::Channel* channel);

		void SetDopplerLevel(FMOD::Channel* channel, float dopplerLevel);
		float GetDopplerLevel(FMOD::Channel* channel);

		void SetLoopCount(FMOD::Channel* channel, int loopCount);
		int GetLoopCount(FMOD::Channel* channel);

		int GetChannelPriority(FMOD::Channel* channel);

		void SetMute(FMOD::Channel* channel, bool mute);
		bool GetMute(FMOD::Channel* channel);

		void SetPitch(FMOD::Channel* channel, float pitch);
		float GetPitch(FMOD::Channel* channel);

		void SetPan(FMOD::Channel* channel, float pan);

		//to do : Add in parameter for min and max distance when after it is added to AudioSource Component

		/**
		 * @brief Updates the 3D position of a sound tied to an entity.
		 */
		void SetSound3DPosition(FMOD::Channel* channel, bool is3D, glm::vec3 soundPos, glm::vec3 vel);

		
		void GetSound3DPosition(FMOD::Channel* channel);

		void Get3DListenerAttributes(glm::vec3& pos, glm::vec3& vel, glm::vec3& forward, glm::vec3& up);

		/** @brief Sets the global master volume for all sounds. */
		void SetMasterVolume(float volume);
		float GetMasterVolume();

		//float GetMasterVolume();

		void SetMinMaxDistance(FMOD::Channel* channel, float minDistance, float maxDistance);
		std::pair<float, float> GetMinMaxDistance(FMOD::Channel* channel);
		
		
		/**
		 * @brief Sets the volume for a specific sound category.
		 */
		void SetCategoryVolume(int categoryType, float volume);

		const float GetCategoryVolume(int categoryType);

		float GetChannelVolume(FMOD::Channel* channel);

		void SetChannelVolume(FMOD::Channel* channel, float volume);

		void SetSpatialBlend(FMOD::Channel* channel, float blend);

		float GetSpatialBlend(FMOD::Channel* channel);

		/**
		 * @brief Syncs all properties of a live channel to match the state of an AudioSource component.
		 */
		void UpdateChannelFromComponent(FMOD::Channel* channel, const AudioSource& audioComp);

		/**
		 * @brief Checks if a sound channel is currently playing.
		 */
		bool IsChannelPlaying(FMOD::Channel* channel);

		/**
		 * @brief Retrieves whether a sound is currently paused.
		 */
		bool GetPauseState(FMOD::Channel* channel);

		/**
		 * @brief Retrieves whether a sound is currently paused.
		 */
		void SetPauseState(FMOD::Channel* channel, bool pauseState);

		/** * @brief Pauses or unpauses a specific channel group (Category).
		 * 0 = SFX, 1 = BGM, 2 = UI, 3 = Editor
		 */
		void SetCategoryPause(int categoryType, bool pauseState);

		void SetPauseStateAllSound(bool pauseState);

		/**
		 * @brief Stops a sound currently playing for a given entity.
		 */
		void StopSound(FMOD::Channel* channel);

		/**
		 * @brief Stops all sounds in the specified internal sound group.
		 */
		void StopAllSound();


		/** @brief Removes any stopped or invalid sounds from memory. */
		//void CleanUpStoppedSounds();

		/** @brief Switches sound context (e.g., between menu and gameplay). */
		//void SwitchSound();
		

	};
}

#endif