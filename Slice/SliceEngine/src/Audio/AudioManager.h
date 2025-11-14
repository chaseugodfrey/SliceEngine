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


namespace SliceEngine
{
	
	//Base SoundTrack struct for sound files
	//struct SoundTrack
	//{
	//	FMOD::Sound* sound = nullptr;
	//	/**************Moved to Audio Source so to takeout *****************/
	//	FMOD::Channel* channel = nullptr;
	//	FMOD::Channel* previewChannel = nullptr;
	//	/*******************************************************************/
	//	float defaultSoundVolume = 1.0f;
	//	float currentSoundVolume = 1.0f;
	//	FMOD_VECTOR soundPos3D = { 0.f,0.f,0.f };
	//	FMOD_VECTOR vel = { 0.f,0.f,0.f };
	//	bool is3D = true;
	//	bool isLooping = false;
	//	bool isPaused = false;
	//	bool muffle = false;
	//	Entity entityID;


	//	virtual ~SoundTrack() = default;
	//};

	

	class AudioManager
	{


		//FMOD::Studio::System* system;
		FMOD::System* mSoundSystem;
		FMOD::ChannelGroup* master;
		FMOD::ChannelGroup* sfx;
		FMOD::ChannelGroup* bgm;
		FMOD::ChannelGroup* ui;
		FMOD::ChannelGroup* editorSounds;

		std::unordered_map<std::string, FMOD::SoundGroup*> mSoundGroups;
		
		const int MAX_CHANNELS = 256;

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
		 * @param vector The input glm vector.
		 * @return An equivalent FMOD_VECTOR.
		 */
		inline FMOD_VECTOR Vec3ToFMODVec3(glm::vec3 vector)
		{
			return { vector.x,vector.y,vector.z };
		}

		/**
		 * @brief Converts an FMOD_VECTOR to a glm::vec3.
		 * @param vector The input FMOD vector.
		 * @return An equivalent glm::vec3.
		 */
		inline glm::vec3 FMODVec3ToVec3(FMOD_VECTOR vector)
		{
			return { vector.x, vector.y, vector.z };
		}

		/**
		 * @brief Plays a sound associated with a specific entity.
		 * @param soundName Name of the loaded sound to play.
		 * @param category Sound category (SFX, BGM, etc.).
		 * @param internalCategory Internal sound grouping for playback management.
		 * @param is3D Whether the sound should use 3D spatialization.
		 * @param isPaused Whether the sound should start paused.
		 * @param isLoop Whether the sound should loop.
		 * @param volume Volume level (0.0–1.0).
		 * @param id The entity owning this sound.
		 * @param soundPos The world position for 3D sounds (default at origin).
		 * @return a pointer to a channel if playback started successfully, nullptr otherwise.
		 */
		FMOD::Channel* PlaySound(GUID soundName, bool isPaused, bool isLoop, float volume, glm::vec3 soundPos, glm::vec3 vel);

		/**
		 * @brief Plays a sound preview for the editor without affecting in-game channels.
		 * @param soundName Name of the sound to preview.
		 * @param is3D Whether the preview uses 3D spatialization.
		 * @param id Entity associated with the sound.
		 * @param soundPos Position for 3D preview playback.
		 * @return a pointer to a channel if preview playback started successfully, nullptr otherwise.
		 */
		FMOD::Channel* PlayEditorPreview(GUID soundName, bool is3D);

		/**
		 * @brief Updates the listener's 3D attributes (position, velocity, orientation).
		 * @param pos Listener position.
		 * @param vel Listener velocity.
		 * @param forward Forward direction vector.
		 * @param up Up direction vector.
		 */
		void SetListenerAttributes(glm::vec3& pos, glm::vec3& vel, glm::vec3& forward, glm::vec3& up);

		//to do : Add in parameter for min and max distance when after it is added to AudioSource Component

		/**
		 * @brief Updates the 3D position of a sound tied to an entity.
		 * @param id The entity whose sound position is being updated.
		 * @param soundPos New 3D position.
		 */
		void SetSound3DPosition(FMOD::Channel* channel, bool is3D, glm::vec3 soundPos, glm::vec3 vel);

		/** @brief Sets the global master volume for all sounds. */
		void GetSound3DPosition(FMOD::Channel* channel);

		/** @brief Sets the global master volume for all sounds. */
		void SetMasterVolume(float volume);

		void SetMinMaxDistance(FMOD::Channel* channel, float minDistance, float maxDistance);
		

		/**
		 * @brief Sets the volume for a specific sound category.
		 * @param category Category to adjust.
		 * @param internalCategory Internal group this category belongs to.
		 * @param volume New volume level.
		 */
		void SetCategoryVolume(GUID soundName, float volume);

		float GetChannelVolume(FMOD::Channel* channel);

		void SetChannelVolume(FMOD::Channel* channel, float volume);

		FMOD::SoundGroup* CreateSoundGroup(std::string& soundGroupName, FMOD::SoundGroup* soundGroup, int maxInstances = -1, FMOD_SOUNDGROUP_BEHAVIOR behaviour = FMOD_SOUNDGROUP_BEHAVIOR_FAIL);

		void SetSoundGroup(GUID soundGUID, std::string soundGroupName);



		/**
		 * @brief Checks if a sound channel is currently playing.
		 * @param entity Entity to check.
		 * @return True if the channel is active and playing.
		 */
		bool IsChannelPlaying(FMOD::Channel* channel);


		/**
		 * @brief Checks if a sound is configured for 3D playback.
		 * @param id Entity to check.
		 * @return True if the sound uses 3D mode.
		 */
		bool IsFMOD3D(FMOD::Channel* channel);

		/**
		 * @brief Updates the FMOD mode (2D/3D) for a sound.
		 * @param id Entity whose mode to change.
		 * @param is3D True for 3D, false for 2D.
		 */
		void UpdateFMODMode(FMOD::Channel* channel, bool is3D);

		/**
		 * @brief Pauses or resumes a sound associated with an entity.
		 * @param id Entity whose sound to pause or resume.
		 * @param isPaused True to pause, false to resume.
		 */
		void UpdatePauseSound(FMOD::Channel* channel, bool isPaused);

		/**
		 * @brief Retrieves whether a sound is currently paused.
		 * @param id Entity to check.
		 * @return True if paused.
		 */
		bool GetPauseState(FMOD::Channel* channel);

		/**
		 * @brief Stops a sound currently playing for a given entity.
		 * @param id Entity whose sound should be stopped.
		 */
		void StopSound(FMOD::Channel* channel);

		/**
		 * @brief Stops an active editor preview sound.
		 * @param id Entity whose preview to stop.
		 */
		void StopEditorPreview(FMOD::Channel* channel);

		/**
		 * @brief Stops all sounds in the specified internal sound group.
		 * @param SoundCategory Internal sound group to stop.
		 */
		void StopAllSound();

		/** @brief Removes any stopped or invalid sounds from memory. */
		//void CleanUpStoppedSounds();

		/** @brief Switches sound context (e.g., between menu and gameplay). */
		//void SwitchSound();
		

	};
}

#endif