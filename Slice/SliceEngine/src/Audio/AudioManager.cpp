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
#include "AudioManager.h"
#include "../src/Core/Core.h"
#include "Input/InputSystem.h"
#include "Resource/ResourceManager.h"
#include "Resource/Audio.h"


namespace SliceEngine
{
#pragma region AUDIO MANAGER
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

		mSoundSystem->set3DSettings(1.0f, 0.01f, 1.0f);

		/*for (int i{}; i < 4; i++)
		{
			mCategoryVolumes.emplace(static_cast<SoundCategory>(i), 1.0f);
		}*/

		//master now points to master channel group
		mSoundSystem->getMasterChannelGroup(&master);
		master->setVolume(1.0f);

		//Create categories for the different sounds
		mSoundSystem->createChannelGroup("SFX", &sfx);
		mSoundSystem->createChannelGroup("BGM", &bgm);
		mSoundSystem->createChannelGroup("UI", &ui);
		mSoundSystem->createChannelGroup("Editor", &editorSounds);
		editorSounds->setVolume(1.0f);

		
	}

	FMOD::System* AudioManager::GetSoundSystem()
	{
		return mSoundSystem;
	}


	void AudioManager::Update()
	{

		mSoundSystem->update();

	}

	FMOD::Channel* AudioManager::PlaySound(const AudioSource& audioComp, glm::vec3 soundPos, glm::vec3 vel)
	{
		if (!audioComp.componentEnabled)
		{
			return nullptr;
		}

		auto audioClip = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Audio>(audioComp.soundGUID).get();
		if (!audioClip)
		{
			SLICE_LOG_ERROR("PlaySound: Failed to get audio clip for GUID.");
			return nullptr;
		}
		
		
		FMOD::Channel* channel = nullptr;

		FMOD_RESULT result = FMOD_OK;

		switch (audioComp.category)
		{
		case 0:
		{

			result = mSoundSystem->playSound(audioClip->GetSound(), sfx, audioComp.isPaused, &channel);
			break;
		};
		case 1:
		{

			result = mSoundSystem->playSound(audioClip->GetSound(), bgm, audioComp.isPaused, &channel);
			break;
		};
		case 2:
		{
			result = mSoundSystem->playSound(audioClip->GetSound(), ui, audioComp.isPaused, &channel);
			break;
		};
		case 3:
		{
			result = mSoundSystem->playSound(audioClip->GetSound(), editorSounds, audioComp.isPaused, &channel);
			break;
		};

		}

		if (result != FMOD_OK)
		{
			SLICE_LOG_ERROR("Failed to play sound");
			return nullptr;
		}

		FMOD_MODE loopMode = audioComp.isLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		FMOD_MODE finalMode;
		const float epsilon = 1e-5f;

		
		if (audioComp.spatialBlend < epsilon)
		{
			finalMode = FMOD_2D | loopMode;
			channel->setMode(finalMode);
		}
		else
		{
			finalMode = FMOD_3D | loopMode | FMOD_3D_LINEARROLLOFF;
			/*if (audioComp.volumeRollOff == 1)
			{
				finalMode |= FMOD_3D_LINEARROLLOFF;
			}*/
			channel->setMode(finalMode);
			SetSpatialBlend(channel, audioComp.spatialBlend);
			FMOD_VECTOR soundPosition = Vec3ToFMODVec3(soundPos);
			FMOD_VECTOR velocity = Vec3ToFMODVec3(vel);
			channel->set3DAttributes(&soundPosition, &velocity);
			channel->set3DMinMaxDistance(audioComp.minDistance, audioComp.maxDistance);
			channel->set3DDopplerLevel(audioComp.dopplerLevel);
			channel->set3DSpread(audioComp.spread);
			channel->setPan(audioComp.stereoPan);
		}

		channel->setPriority(audioComp.priority);
		channel->setVolume(audioComp.currentVolume);
		channel->setPitch(audioComp.pitch);
		channel->setMute(audioComp.isMute);


		


		return channel;

	}

	FMOD::Channel* AudioManager::PlayEditorPreview(const AudioSource& audioComp)
	{
		auto audioClip = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Audio>(audioComp.soundGUID).get();
		if (!audioClip)
		{
			SLICE_LOG_ERROR("PlaySound: Failed to get audio clip for GUID.");
			return nullptr;
		}

		FMOD::Channel* previewChannel = nullptr;

		FMOD_RESULT result = mSoundSystem->playSound(audioClip->GetSound(), editorSounds, false, &previewChannel);

		FMOD_MODE finalMode = FMOD_2D;

		previewChannel->setMode(finalMode);

		float spatialBlend = 0.0f;

		SetSpatialBlend(previewChannel, spatialBlend);
		//track->soundPos3D = Vec3ToFMODVec3(soundPos);

		previewChannel->setPriority(audioComp.priority);
		previewChannel->setMute(audioComp.isMute);
		previewChannel->setPitch(audioComp.pitch);
		previewChannel->setPan(audioComp.stereoPan);

		if (result != FMOD_OK)
		{
			SLICE_LOG_ERROR("Failed to play sound");
			return nullptr;
		}

		return previewChannel;
	}

	void AudioManager::Get3DListenerAttributes(glm::vec3& pos, glm::vec3& vel, glm::vec3& forward, glm::vec3& up)
	{
		FMOD_VECTOR fPos, fVel, fForward, fUp;

		// 0 is the index for the first listener
		FMOD_RESULT result = mSoundSystem->get3DListenerAttributes(0, &fPos, &fVel, &fForward, &fUp);

		if (result == FMOD_OK)
		{
			pos = FMODVec3ToVec3(fPos);
			vel = FMODVec3ToVec3(fVel);
			forward = FMODVec3ToVec3(fForward);
			up = FMODVec3ToVec3(fUp);
		}
		else
		{
			SLICE_LOG_ERROR("FMOD: Failed to get listener attributes. Error: " + std::to_string(result));
		}
	}

	void AudioManager::SetMasterVolume(float volume)
	{
		
		master->setVolume(volume);
	}

	float AudioManager::GetMasterVolume()
	{
		float volume = 0.0f;
		master->getVolume(&volume);
		return volume;
	}

	void AudioManager::SetMinMaxDistance(FMOD::Channel* channel, float minDistance, float maxDistance)
	{
		channel->set3DMinMaxDistance(minDistance, maxDistance);
	}

	std::pair<float, float> AudioManager::GetMinMaxDistance(FMOD::Channel* channel)
	{
		if (!channel)
		{
			return { 0.0f,0.0f };
		}

		float minDist, maxDist;

		channel->get3DMinMaxDistance(&minDist, &maxDist);

		return { minDist,maxDist };
	}

	void AudioManager::SetCategoryVolume(int categoryType, float volume)
	{
		
		switch (categoryType)
		{
			case 0:
			{
				sfx->setVolume(volume);
				break;
			}
			case 1:
			{
				bgm->setVolume(volume);
				break;
			}
			case 2:
			{
				ui->setVolume(volume);
				break;
			}
			case 3:
			{
				editorSounds->setVolume(volume);
				break;
			}

		}
	}

	const float AudioManager::GetCategoryVolume(int categoryType)
	{
		float volume = 0.0f;
		switch (categoryType)
		{
			case 0:
			{
				sfx->getVolume(&volume);
				break;
			}
			case 1:
			{
				bgm->getVolume(&volume);
				break;
			}
			case 2:
			{
				ui->getVolume(&volume);
				break;
			}
			case 3:
			{
				editorSounds->getVolume(&volume);
				break;
			}

		}

		return volume;
	}

	

	float AudioManager::GetChannelVolume(FMOD::Channel* channel)
	{
		float volume;

		channel->getVolume(&volume);

		return volume;
	}

	void AudioManager::SetChannelVolume(FMOD::Channel* channel, float volume)
	{
		channel->setVolume(volume);
	}



	void AudioManager::SetSpatialBlend(FMOD::Channel* channel, float blend)
	{
		if (!channel)
		{
			return;
		}

		channel->set3DLevel(blend);
	}

	float AudioManager::GetSpatialBlend(FMOD::Channel* channel)
	{
		if (!channel)
		{
			return 0.0f;
		}

		float blend = 0.0f;

		channel->get3DLevel(&blend);

		return blend;
	}

	void AudioManager::UpdateChannelFromComponent(FMOD::Channel* channel, const AudioSource& audioComp)
	{
		if (!channel) return;

		//Handle volume
		if (std::abs(audioComp.currentVolume - GetChannelVolume(channel)) > 0.001f)
		{
			SetChannelVolume(channel, audioComp.currentVolume);
		}

		//Handle Pause
		if (audioComp.isPaused != GetPauseState(channel))
		{
			SetPauseState(channel, audioComp.isPaused);
		}

		//Handle Spatial Blend
		if (std::abs(audioComp.spatialBlend - GetSpatialBlend(channel)) > 0.001f)
		{
			SetSpatialBlend(channel, audioComp.spatialBlend);
		}

		//Handle Mute
		if (audioComp.isMute != GetMute(channel))
		{
			SetMute(channel, audioComp.isMute);
		}

		//Handle Pitch
		if (std::abs(audioComp.pitch - GetPitch(channel)) > 0.001f)
		{
			SetPitch(channel, audioComp.pitch);
		}

		//Handle Stereo Pan (Write-only)
		if (audioComp.spatialBlend < 1.0f)
		{
			SetPan(channel, audioComp.stereoPan);
		}

		//Handle Priority for channel
		if (audioComp.priority != GetChannelPriority(channel))
		{
			SetChannelPriority(channel, audioComp.priority);
		}

		//Handle Min and Max Distance
		std::pair<float, float> distance = GetMinMaxDistance(channel);
		if (std::abs(audioComp.minDistance - distance.first) > 0.001f || std::abs(audioComp.maxDistance - distance.second) > 0.001f)
		{
			SetMinMaxDistance(channel, audioComp.minDistance, audioComp.maxDistance);
		}

		//Handle Doppler
		float doppler = GetDopplerLevel(channel);
		if (std::abs(audioComp.dopplerLevel - doppler) > 0.001f)
		{
			SetDopplerLevel(channel, audioComp.dopplerLevel);
		}

		//Handle Spread
		float spread = GetSpread(channel);
		if (std::abs(audioComp.spread - spread) > 0.001f)
		{
			SetSpread(channel, audioComp.spread);
		}

		//Handle Occlusion
		std::pair<float, float> occlusion = GetOcclusion(channel);
		if (std::abs(audioComp.directOcclusion - occlusion.first) > 0.001f || std::abs(audioComp.reverbOcclusion - occlusion.second) > 0.001f)
		{
			SetOcclusion(channel, audioComp.directOcclusion, audioComp.reverbOcclusion);
		}

		//Handle Volume Rolloff
		FMOD_MODE currentMode;
		channel->getMode(&currentMode);

		bool isFmodLinear = (currentMode & FMOD_3D_LINEARROLLOFF);
		bool isCompLinear = (audioComp.volumeRollOff == AudioSource::VolumeRollOff::Linear);

		if (isFmodLinear != isCompLinear && audioComp.spatialBlend > 0.0f)
		{
			bool isLooping = (currentMode & FMOD_LOOP_NORMAL);
			FMOD_MODE newMode = FMOD_3D | (isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

			if (isCompLinear)
			{
				newMode |= FMOD_3D_LINEARROLLOFF;
			}

			channel->setMode(newMode);
		}
	}

	bool AudioManager::IsChannelPlaying(FMOD::Channel* channel)
	{
		bool channelFlag = false;
		channel->isPlaying(&channelFlag);

		return channelFlag;
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

	void AudioManager::SetChannelPriority(FMOD::Channel* channel, int priorityNumber)
	{
		if (channel != nullptr)
		{
			channel->setPriority(priorityNumber);
		}
	}

	void AudioManager::SetOcclusion(FMOD::Channel* channel, float direct, float reverb)
	{
		if (channel)
		{
			channel->set3DOcclusion(direct, reverb);
		}
	}

	std::pair<float, float> AudioManager::GetOcclusion(FMOD::Channel* channel)
	{
		if (!channel)
		{
			return { 0.0f, 0.0f };
		}
		float direct, reverb;
		channel->get3DOcclusion(&direct, &reverb);
		return { direct, reverb };
	}

	void AudioManager::SetSpread(FMOD::Channel* channel, float spread)
	{
		if (channel)
		{
			channel->set3DSpread(spread);
		}
	}

	float AudioManager::GetSpread(FMOD::Channel* channel)
	{
		if (!channel)
		{
			return 0.0f;
		}
		float spread;
		channel->get3DSpread(&spread);
		
		return spread;
	}

	void AudioManager::SetDopplerLevel(FMOD::Channel* channel, float dopplerLevel)
	{
		if (channel)
		{
			channel->set3DDopplerLevel(dopplerLevel);
		}
	}

	float AudioManager::GetDopplerLevel(FMOD::Channel* channel)
	{
		if (!channel)
		{
			return 0.0f;
		}
		float dopplerLevel;

		channel->get3DDopplerLevel(&dopplerLevel);

		return dopplerLevel;
	}

	void AudioManager::SetLoopCount(FMOD::Channel* channel, int loopCount)
	{
		if (channel)
		{
			channel->setLoopCount(loopCount);
		}

	}

	int AudioManager::GetLoopCount(FMOD::Channel* channel)
	{
		if (channel)
		{
			int currentLoopCount = 0;
			channel->getLoopCount(&currentLoopCount);
			return currentLoopCount;
		}
	}

	int AudioManager::GetChannelPriority(FMOD::Channel* channel)
	{
		if (!channel)
		{
			return 128;
		}
		int priority;
		channel->getPriority(&priority);

		return priority;
	}

	void AudioManager::SetMute(FMOD::Channel* channel, bool mute)
	{
		if (channel)
		{
			channel->setMute(mute);
		}
	}

	bool AudioManager::GetMute(FMOD::Channel* channel)
	{
		if (!channel)
		{
			return false;
		}

		bool mute;
		channel->getMute(&mute);

		return mute;
	}

	void AudioManager::SetPitch(FMOD::Channel* channel, float pitch)
	{
		if (channel)
		{
			channel->setPitch(pitch);
		}
	}

	float AudioManager::GetPitch(FMOD::Channel* channel)
	{
		if (!channel)
		{
			return 1.0f;
		}
		float pitch;
		channel->getPitch(&pitch);

		return pitch;
	}

	void AudioManager::SetPan(FMOD::Channel* channel, float pan)
	{
		if (channel)
		{
			channel->setPan(pan);
		}
	}


	void AudioManager::SetSound3DPosition(FMOD::Channel* channel, bool is3D, glm::vec3 soundPos, glm::vec3 vel)
	{
		FMOD_VECTOR soundPosition = Vec3ToFMODVec3(soundPos);
		FMOD_VECTOR velocity = Vec3ToFMODVec3(vel);

		channel->set3DAttributes(&soundPosition, &velocity);
	}

	bool AudioManager::GetPauseState(FMOD::Channel* channel)
	{
		if (!channel)
		{
			SLICE_LOG_ERROR("Unable to get pause state");
			return false;
		}
		bool pauseState = false;
		channel->getPaused(&pauseState);

		return pauseState;
	}

	void AudioManager::SetPauseState(FMOD::Channel* channel, bool pauseState)
	{
		if (channel)
		{
			channel->setPaused(pauseState);
		}
	}

	void AudioManager::SetCategoryPause(int categoryType, bool pauseState)
	{
		switch (categoryType)
		{
		case 0: // SFX
			if (sfx)
			{
				sfx->setPaused(pauseState);
			}
			break;
		case 1: // BGM
			if (bgm)
			{
				bgm->setPaused(pauseState);
			}
			break;
		case 2: // UI
			if (ui)
			{
				ui->setPaused(pauseState);
			}
			break;
		case 3: // Editor
			if (editorSounds)
			{
				editorSounds->setPaused(pauseState);
			}
			break;
		}
	}

	void AudioManager::SetPauseStateAllSound(bool pauseState)
	{
		master->setPaused(pauseState);
	}

	void AudioManager::StopSound(FMOD::Channel* channel)
	{
		if (channel)
		{
			channel->stop();
			channel = nullptr;

		}
	}

	void AudioManager::StopAllSound()
	{
		
		master->stop();
	}


	void AudioManager::Exit()
	{

		if (mSoundSystem)
		{
			mSoundSystem->close();
			mSoundSystem->release();
			mSoundSystem = nullptr;
		}
		SLICE_LOG("Shutting down FMOD Studio.");
	}
#pragma endregion
}