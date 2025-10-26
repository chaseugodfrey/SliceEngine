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
	//Can be taken out
	enum class SoundCategory
	{
		SFX,
		BGM,
		UI,
		Editor
	};

	//Base SoundTrack struct for sound files
	struct SoundTrack
	{
		FMOD::Sound* sound = nullptr;
		/**************Moved to Audio Source so to takeout *****************/
		FMOD::Channel* channel = nullptr;
		FMOD::Channel* previewChannel = nullptr;
		/*******************************************************************/
		float defaultSoundVolume = 1.0f;
		float currentSoundVolume = 1.0f;
		FMOD_VECTOR soundPos3D = { 0.f,0.f,0.f };
		FMOD_VECTOR vel = { 0.f,0.f,0.f };
		SoundCategory category = SoundCategory::SFX;
		bool is3D = true;
		bool isLooping = false;
		bool isPaused = false;
		bool muffle = false;
		Entity entityID;


		virtual ~SoundTrack() = default;

		virtual void ApplySettings()
		{
			if (channel)
			{
				channel->setVolume(currentSoundVolume);
				channel->setMode(isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
				channel->set3DAttributes(&soundPos3D, &vel);
				channel->set3DMinMaxDistance(0.1f, 0.6f);
			}
		}
	};

	

	class AudioManager
	{


		//FMOD::Studio::System* system;
		FMOD::System* mSoundSystem;
		FMOD::ChannelGroup* master;
		FMOD::ChannelGroup* sfx;
		FMOD::ChannelGroup* bgm;
		FMOD::ChannelGroup* ui;
		FMOD::ChannelGroup* editorSounds;
		
		const int MAX_CHANNELS = 256;
		std::unordered_map<std::string, std::unique_ptr<SoundTrack>> mLoadedSounds;
		//To take out cause redundant
		std::unordered_map<SoundCategory, float> mCategoryVolumes;

		const float defaultVolume = 1.0f;



	public:
		//To take out cause redundant
		enum InternalSound
		{
			SOUND_INGAME,
			SOUND_INMENU,
			SOUND_BGM,
			SOUND_EDITOR,
			SOUND_MAX_SOUNDS
		};

	private:

		std::vector<std::unique_ptr<SoundTrack>> mSound[SOUND_MAX_SOUNDS];
		std::unordered_map<Entity, std::unique_ptr<SoundTrack>> mSounds[SOUND_MAX_SOUNDS];
		float mMasterVolume = 1.0f;


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
		FMOD_VECTOR Vec3ToFMODVec3(glm::vec3& vector)
		{
			return FMOD_VECTOR{ vector.x,vector.y,vector.z };
		}

		/**
		 * @brief Converts an FMOD_VECTOR to a glm::vec3.
		 * @param vector The input FMOD vector.
		 * @return An equivalent glm::vec3.
		 */
		glm::vec3 FMODVec3ToVec3(FMOD_VECTOR vector)
		{
			return glm::vec3{ vector.x, vector.y, vector.z };
		}

		/**
		 * @brief Loads a sound file into memory and stores it for later playback.
		 * @param soundFile The path or name of the sound file to load.
		 */
		void LoadSound(GUID soundFile);

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
		 * @return True if playback started successfully, false otherwise.
		 */
		bool PlaySound(const std::string& soundName, SoundCategory category, InternalSound internalCategory, bool is3D, bool isPaused, bool isLoop, float volume, Entity& id, glm::vec3 soundPos = { 0.f,0.f,0.f });

		/**
		 * @brief Plays a sound preview for the editor without affecting in-game channels.
		 * @param soundName Name of the sound to preview.
		 * @param is3D Whether the preview uses 3D spatialization.
		 * @param id Entity associated with the sound.
		 * @param soundPos Position for 3D preview playback.
		 * @return True if preview playback started successfully, false otherwise.
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
		void SetSound3DPosition(Entity& id, glm::vec3 soundPos);

		/** @brief Sets the global master volume for all sounds. */
		glm::vec3 GetSound3DPosition(Entity& id);

		/** @brief Sets the global master volume for all sounds. */
		void SetMasterVolume(float volume);

		/**
		 * @brief Sets the volume for a specific sound category.
		 * @param category Category to adjust.
		 * @param internalCategory Internal group this category belongs to.
		 * @param volume New volume level.
		 */
		void SetCategoryVolume(SoundCategory category, InternalSound internalCatergory, float volume);

		/**
		 * @brief Gets the current volume for a specific sound category.
		 * @param category Category to query.
		 * @return Volume level of that category.
		 */
		float GetCategoryVolume(SoundCategory category) const;

		/**
		 * @brief Updates the volume of a specific sound instance.
		 * @param id Entity whose sound volume to update.
		 * @param volume New volume value.
		 */
		void UpdateSoundVolume(Entity& id, float volume);

		/**
		 * @brief Retrieves the current volume of a specific sound instance.
		 * @param id Entity whose sound volume to retrieve.
		 * @return Current track volume.
		 */
		float GetCurrentTrackVolume(Entity& id);

		/**
		 * @brief Checks if a sound channel associated with an entity is null.
		 * @param entity Entity to check.
		 * @return True if the channel is null, false otherwise.
		 */
		bool IsChannelNull(Entity& entity);

		/**
		 * @brief Checks if a sound channel is currently playing.
		 * @param entity Entity to check.
		 * @return True if the channel is active and playing.
		 */
		bool IsChannelPlaying(Entity& entity);

		/**
		 * @brief Checks if a preview channel is currently playing in the editor.
		 * @param entity Entity to check.
		 * @return True if the preview channel is active.
		 */
		bool IsPreviewChannelPlaying(Entity& entity);

		/**
		 * @brief Checks if a sound is configured for 3D playback.
		 * @param id Entity to check.
		 * @return True if the sound uses 3D mode.
		 */
		bool IsFMOD3D(Entity& id);

		/**
		 * @brief Updates the FMOD mode (2D/3D) for a sound.
		 * @param id Entity whose mode to change.
		 * @param is3D True for 3D, false for 2D.
		 */
		void UpdateFMODMode(Entity& id, bool is3D);

		/**
		 * @brief Pauses or resumes a sound associated with an entity.
		 * @param id Entity whose sound to pause or resume.
		 * @param isPaused True to pause, false to resume.
		 */
		void UpdatePauseSound(Entity& id, bool isPaused);

		/**
		 * @brief Retrieves whether a sound is currently paused.
		 * @param id Entity to check.
		 * @return True if paused.
		 */
		bool GetPauseState(Entity& id);

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
		void StopAllSound(InternalSound SoundCategory);

		/** @brief Removes any stopped or invalid sounds from memory. */
		//void CleanUpStoppedSounds();

		/** @brief Switches sound context (e.g., between menu and gameplay). */
		//void SwitchSound();

	private:

		/**
		 * @brief Internal helper to update a SoundTrack’s effective volume.
		 * @param track Pointer to the sound track to update.
		 */
		void UpdateSoundVolume(SoundTrack* track);

		/**
		 * @brief Calculates the final effective volume of a track, considering master and category levels.
		 * @param track The sound track to evaluate.
		 * @param category The sound category.
		 * @return Computed effective volume.
		 */
		float CalculateFinalVolume(const SoundTrack* track, SoundCategory category) const;
		

	};
}

#endif