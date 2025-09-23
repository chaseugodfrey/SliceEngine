#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H
//#include "../ThirdParty/fmod/include/fmod.hpp"
#include <fmod.hpp>
#include "Systems/SoundSystem.h"
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
		SoundCategory category = SoundCategory::SFX;
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
		std::unordered_map<std::string, std::unique_ptr<SoundTrack3D>> mLoadedSounds;
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

		void Init();
		void Update();
		void Exit();

		void LoadSound(const std::string& soundName, const std::string& soundFile);
		bool PlaySound(const std::string& soundName, SoundCategory category, InternalSound internalCategory, bool is3D, bool isLoop, float volume, Entity id);

		void SetListenerAttributes(glm::vec3& pos, glm::vec3& vel, glm::vec3& forward, glm::vec3& up);
		void SetSound3DPosition(const std::string& soundName, glm::vec3 soundPos);

		void SetMasterVolume(float volume);
		void SetCategoryVolume(SoundCategory category, InternalSound internalCatergory, float volume);
		void GetCategoryVolume(SoundCategory category);

		void StopSound(InternalSound internalCategory, Entity id);
		void StopAllSound(InternalSound SoundCategory);

		void CleanUpStoppedSounds();
		void SwitchSound();

	private:
		std::vector<std::unique_ptr<SoundTrack2D>> mSound2D[SOUND_MAX_SOUNDS];
		std::vector<std::unique_ptr<SoundTrack3D>> mSound3D[SOUND_MAX_SOUNDS];
		//	AudioManager() = default;
		//	~AudioManager() = default;

		//	AudioManager(const AudioManager&) = delete;
		//	AudioManager& operator=(const AudioManager&) = delete;

	};
}

#endif