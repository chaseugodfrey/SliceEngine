/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AudioManager.cpp

 author:	  Lee Yong Yee

 email:       l.yongyee@digipen.edu

 brief:		  Defines the AudioManager class and related audio structures for handling sound playback
			  within the engine using the FMOD sound library. This system manages loading, playing,
			  and updating 2D and 3D sounds, maintaining category-based volume control, and handling
			  sound states such as pause, looping, and positional audio.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "../Audio/AudioManager.h"
#include "../src/Core/Core.h"
#include "Input/InputSystem.h"
#include "Resource/ResourceManager.h"
#include "Resource/Audio.h"

namespace SliceEngine
{
	/*
	* Init initialises and creates the FMOD System
	*/
	void AudioManager::Init()
	{
		SLICE_LOG("Initializing FMOD");
		FMOD_RESULT result = FMOD::System_Create(&mSoundSystem);
		if (result != FMOD_OK)
		{
			SLICE_LOG_ERROR("FMOD System creation failed");
			return;
		}

		result = mSoundSystem->init(MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr);
		if (result != FMOD_OK)
		{
			SLICE_LOG_ERROR("FMOD System initialization failed");
			return;
		}

		for (int i{}; i < 4; i++)
		{
			mCategoryVolumes.emplace(static_cast<SoundCategory>(i), 1.0f);
		}
	}

	/*
	* LoadSound loads all sounds in 3D because its easier to make set the FMOD mode to 2D from 3D if need to
	*/
	void AudioManager::LoadSound(const std::string& soundFile)
	{

		FMOD::Sound* sound{ nullptr };

		FMOD::Sound* soundTest{ nullptr };

		auto audioHandle = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Audio>(soundFile);
		SliceEngineTypes::Audio* audio = audioHandle.get();

		if (audio && audio->sound)
		{
			soundTest = audio->sound; 
			// You can now play, stop, or set properties on soundTest
		}


		std::string soundName;
		mSoundSystem->createSound(soundFile.c_str(), FMOD_3D, nullptr, &sound);

		soundName = soundFile.substr(soundFile.find_last_of("/") + 1, soundFile.find_last_of(".") - soundFile.find_last_of("/")-1);

		if (sound)
		{
			auto track = std::make_unique<SoundTrack>();
			track->sound = sound;

			mLoadedSounds.try_emplace(soundName, std::move(track));

			SLICE_LOG("Sound Loaded" + soundName);
			return;
		}
	}

	void AudioManager::Update()
	{

		mSoundSystem->update();

	}

	bool AudioManager::PlaySound(const std::string soundName, SoundCategory category, InternalSound internalCategory, bool is3D, bool isPaused, bool isLoop, float volume, Entity& id, glm::vec3 soundPos)
	{

		auto it = mLoadedSounds.find(soundName);
		if (it == mLoadedSounds.end())
		{
			SLICE_LOG("Sound not loaded");
			return false;
		}

		auto track = std::make_unique<SoundTrack>();

		if (is3D == false)
		{
			track->channel->setMode(FMOD_2D);
		}
		
		track->soundPos3D = Vec3ToFMODVec3(soundPos);

		FMOD::Channel* channel = nullptr;

		FMOD_RESULT result = mSoundSystem->playSound(it->second.get()->sound, nullptr, isPaused, &channel);

		if (result != FMOD_OK)
		{
			SLICE_LOG_ERROR("Failed to play sound");
			return false;
		}

		if (channel)
		{
			track->channel = channel;
			
			track->category = category;
			track->isLooping = isLoop;
			track->isPaused = isPaused;
			track->currentSoundVolume = volume;
			track->entityID = id;
			
			track->ApplySettings();

			mSound[internalCategory].emplace_back(std::move(track));
			return true;
		}




		return false;

	}

	bool AudioManager::PlayEditorPreview(const std::string soundName, bool is3D, Entity& id, glm::vec3 soundPos)
	{
		auto it = mLoadedSounds.find(soundName);
		if (it == mLoadedSounds.end())
		{
			SLICE_LOG("Sound not loaded");
			return false;
		}

		auto track = std::make_unique<SoundTrack>();

		if (is3D == false)
		{
			track->channel->setMode(FMOD_2D);
		}

		track->soundPos3D = Vec3ToFMODVec3(soundPos);

		FMOD::Channel* channel = nullptr;

		FMOD_RESULT result = mSoundSystem->playSound(it->second.get()->sound, nullptr, false, &channel);

		if (result != FMOD_OK)
		{
			SLICE_LOG_ERROR("Failed to play sound");
			return false;
		}

		if (channel)
		{
			track->previewChannel = channel;
			track->category = SoundCategory::Editor;
			track->currentSoundVolume = 0.3f;
			track->isLooping = false;
			track->entityID = id;

			track->ApplySettings();

			mSound[SOUND_EDITOR].emplace_back(std::move(track));
			return true;

		}

		return false;
	}

	void AudioManager::SetMasterVolume(float volume)
	{
		mMasterVolume = std::clamp(volume, 0.0f, 1.0f);

		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; ++i)
		{
			for (auto& track : mSound[i])
			{

				UpdateSoundVolume(track.get());

			}
		}
	}

	void AudioManager::SetCategoryVolume(SoundCategory category, InternalSound internalCategory, float volume)
	{
		mCategoryVolumes[category] = std::clamp(volume, 0.0f, 1.0f);

		for (auto& it : mSound[internalCategory])
		{
			if (it->category == category)
			{
				UpdateSoundVolume(it.get());
			}

		}
	}

	float AudioManager::GetCategoryVolume(SoundCategory category) const
	{
		return mCategoryVolumes.at(category);
	}

	float AudioManager::CalculateFinalVolume(const SoundTrack* track, SoundCategory category) const
	{

		return track->currentSoundVolume * GetCategoryVolume(category) * mMasterVolume;
	}

	void AudioManager::UpdateSoundVolume(SoundTrack* track)
	{
		

		float finalVolume = CalculateFinalVolume(track, track->category);
		track->channel->setVolume(finalVolume);
		
	}

	float AudioManager::GetCurrentTrackVolume(Entity& id)
	{
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; ++i)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == id && track->channel)
				{
					return track->currentSoundVolume;
				}
			}
		}

		return 0.0f;
	}

	bool AudioManager::IsChannelNull(Entity& entity)
	{
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; ++i)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == entity)
				{
					return (track->channel == nullptr);
				}
			}
		}

		return true;
	}

	bool AudioManager::IsChannelPlaying(Entity& entity)
	{
		// Loop through all internal sound categories
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; ++i)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == entity && track->channel)
				{
					bool isPlaying = false;
					if (track->channel->isPlaying(&isPlaying) == FMOD_OK)
					{
						return isPlaying;
					}
				}
			}
		}
		return false;
	}

	bool AudioManager::IsPreviewChannelPlaying(Entity& entity)
	{
		// Loop through all internal sound categories
		
		for (auto& track : mSound[SOUND_EDITOR])
		{
			if (track->entityID == entity && track->previewChannel)
			{
				bool isPlaying = false;
				if (track->previewChannel->isPlaying(&isPlaying) == FMOD_OK)
				{
					return isPlaying;
				}
			}
		}
		
		return false;
	}

	void AudioManager::UpdateSoundVolume(Entity& id, float volume)
	{
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == id)
				{
					track->currentSoundVolume = volume;
					UpdateSoundVolume(track.get());
					break;
				}
			}
		}

		
	}

	void AudioManager::SetListenerAttributes(glm::vec3& pos, glm::vec3& vel, glm::vec3& forward, glm::vec3& up)
	{
		FMOD_VECTOR positionVec = Vec3ToFMODVec3(pos);
		FMOD_VECTOR forwardVec = Vec3ToFMODVec3(forward);
		FMOD_VECTOR velVec = Vec3ToFMODVec3(vel);
		FMOD_VECTOR upVec = Vec3ToFMODVec3(up);

		mSoundSystem->set3DListenerAttributes(0, &positionVec, &velVec, &forwardVec, &upVec);
		//mSoundSystem->update();
	}

	void AudioManager::SetSound3DPosition(Entity& id, glm::vec3 soundPos)
	{
		//FMOD_VECTOR soundPosition = { soundPos.x, soundPos.y, soundPos.z };
		//FMOD_VECTOR velVec = { 0.0f,0.0f,0.0f };


		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == id && track->channel)
				{
					track->soundPos3D = Vec3ToFMODVec3(soundPos);
					track->channel->set3DAttributes(&track->soundPos3D, &track->vel);
					if (track->is3D == false && soundPos.x == 0.f && soundPos.y == 0.f && soundPos.z == 0.f)
					{
						track->channel->set3DMinMaxDistance(1.0f, 1.0f);
					}
					else
					{
						track->channel->set3DMinMaxDistance(0.1f, 0.6f);
					}
					break;
				}
			}
		}
	}

	glm::vec3 AudioManager::GetSound3DPosition(Entity& id)
	{
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == id && track->channel)
				{
					return FMODVec3ToVec3(track->soundPos3D);
				}
			}
		}

		return FMODVec3ToVec3(FMOD_VECTOR{0.f,0.f,0.f});
	}

	void AudioManager::UpdatePauseSound(Entity& id, bool isPaused)
	{
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == id && track->channel)
				{
					track->isPaused = isPaused;
					track->channel->setPaused(isPaused);
				}
			}
		}
	}

	bool AudioManager::GetPauseState(Entity& id)
	{

		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == id && track->channel)
				{
					return track->isPaused;
				}
			}
		}

		return false;
	}

	bool AudioManager::IsFMOD3D(Entity& id)
	{
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == id && track->channel)
				{
					return track->is3D;
				}
			}
		}

		return false;
	}

	void AudioManager::UpdateFMODMode(Entity& id, bool is3D)
	{
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == id && track->channel)
				{
					track->is3D = is3D;
					if (is3D)
					{
						track->channel->setMode(FMOD_3D);
					}
					else
					{
						track->channel->setMode(FMOD_2D);
					}
				}
			}
		}
	}

	void AudioManager::StopSound(Entity& id)
	{
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
		{
			for (auto& track : mSound[i])
			{
				if (track->entityID == id && track->channel)
				{
					track->channel->stop();
					break;
				}
			}
		}
	}

	void AudioManager::StopEditorPreview(Entity& id)
	{
		for (auto& track : mSound[SOUND_EDITOR])
		{
			if (track->entityID == id && track->previewChannel)
			{
				track->previewChannel->stop();
				track->previewChannel = nullptr;
				break;
			}
		}
	}

	void AudioManager::StopAllSound(InternalSound InternalCategory)
	{
		

		for (auto it = mSound[InternalCategory].begin(); it != mSound[InternalCategory].end(); ++it)
		{
			if (it->get()->channel)
			{
				it->get()->channel->stop();
			}

		}

		mSound[InternalCategory].clear();
	}


	void AudioManager::Exit()
	{
		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; ++i)
		{

			StopAllSound(static_cast<InternalSound>(i));

		}


		for (auto& pair : mLoadedSounds)
		{
			if (pair.second->sound)
			{
				pair.second->sound->release();
				pair.second->sound = nullptr;
			}
		}

		mLoadedSounds.clear();


		if (mSoundSystem)
		{
			mSoundSystem->close();
			mSoundSystem->release();
			mSoundSystem = nullptr;
		}
		SLICE_LOG("Shutting down FMOD Studio.");
	}
}