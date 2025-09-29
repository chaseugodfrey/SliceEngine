#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H
//#include "../ThirdParty/fmod/include/fmod.hpp"
#include <fmod.hpp>
#include "ECS/ECSTypes.h"
#include <vector>
#include <unordered_map>


namespace SliceEngine
{
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
		FMOD::Channel* channel = nullptr;
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

	//struct for 2D sounds
	struct SoundTrack2D : public SoundTrack
	{
		void ApplySettings() override
		{
			SoundTrack::ApplySettings();
		}
	};

	//struct for 3D sounds
	struct SoundTrack3D : public SoundTrack
	{
		/*FMOD_VECTOR position{ 0.0f,0.0f,0.0f };
		FMOD_VECTOR velocity{ 0.0f,0.0f,0.0f };

		void ApplySettings() override
		{
			SoundTrack::ApplySettings();
			if (channel)
			{
				channel->set3DAttributes(&position, &velocity);
				channel->set3DMinMaxDistance(0.1f, 0.6f);
			}
		}*/
		void ApplySettings() override
		{
			SoundTrack::ApplySettings();
		}
	};

	

	class AudioManager
	{


		//FMOD::Studio::System* system;
		FMOD::System* mSoundSystem;
		const int MAX_CHANNELS = 256;



		std::unordered_map<std::string, std::unique_ptr<SoundTrack2D>> mLoadedSounds2D;
		std::unordered_map<std::string, std::unique_ptr<SoundTrack3D>> mLoadedSounds3D;
		std::unordered_map<std::string, std::unique_ptr<SoundTrack>> mLoadedSounds;
		std::unordered_map<SoundCategory, float> mCategoryVolumes;

		const float defaultVolume = 1.0f;



	public:
		/*static AudioManager& Get()
		{
			static AudioManager instance;
			return instance;
		}*/
		enum InternalSound
		{
			SOUND_INGAME,
			SOUND_INMENU,
			SOUND_BGM,
			SOUND_EDITOR,
			SOUND_MAX_SOUNDS
		};

	private:
		std::vector<std::unique_ptr<SoundTrack2D>> mSound2D[SOUND_MAX_SOUNDS];
		std::vector<std::unique_ptr<SoundTrack3D>> mSound3D[SOUND_MAX_SOUNDS];
		std::vector<std::unique_ptr<SoundTrack>> mSound[SOUND_MAX_SOUNDS];
		float mMasterVolume = 1.0f;


	public:
		void Init();
		void Update();
		void Exit();

		FMOD_VECTOR Vec3ToFMODVec3(glm::vec3& vector)
		{
			return FMOD_VECTOR{ vector.x,vector.y,vector.z };
		}

		glm::vec3 FMODVec3ToVec3(FMOD_VECTOR vector)
		{
			return glm::vec3{ vector.x, vector.y, vector.z };
		}

		void LoadSound(const std::string& soundFile);
		bool PlaySound(const std::string soundName, SoundCategory category, InternalSound internalCategory, bool is3D, bool isPaused, bool isLoop, float volume, Entity& id, glm::vec3 soundPos = { 0.f,0.f,0.f });

		void SetListenerAttributes(glm::vec3& pos, glm::vec3& vel, glm::vec3& forward, glm::vec3& up);

		//to do : Add in parameter for min and max distance when after it is added to AudioSource Component
		void SetSound3DPosition(Entity& id, glm::vec3 soundPos);
		glm::vec3 GetSound3DPosition(Entity& id);

		void SetMasterVolume(float volume);
		void SetCategoryVolume(SoundCategory category, InternalSound internalCatergory, float volume);
		float GetCategoryVolume(SoundCategory category) const;
		void UpdateSoundVolume(Entity& id, float volume);
		float GetCurrentTrackVolume(Entity& id);

		bool IsFMOD3D(Entity& id);
		void UpdateFMODMode(Entity& id, bool is3D);

		void UpdatePauseSound(Entity& id, bool isPaused);
		bool GetPauseState(Entity& id);

		void StopSound(Entity& id);
		void StopAllSound(InternalSound SoundCategory);

		void CleanUpStoppedSounds();
		void SwitchSound();

	private:

		void UpdateSoundVolume(SoundTrack* track);
		float CalculateFinalVolume(const SoundTrack* track, SoundCategory category) const;
		

	};
}

#endif