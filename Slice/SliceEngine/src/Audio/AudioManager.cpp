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

	FMOD::Channel* AudioManager::PlaySound(GUID soundName, bool isPaused, bool isLoop, float volume, glm::vec3 soundPos, glm::vec3 vel)
	{

		//This call will handle the loading if the resource hasn't been loaded
		auto audioClip = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Audio>(soundName).get();
		
		FMOD::Channel* channel = nullptr;

		if (audioClip->GetDimension() == false)
		{
			channel->setMode(FMOD_2D);
		}

		FMOD_RESULT result = FMOD_OK;

		switch (audioClip->GetCategory())
		{
		case 0:
		{

			result = mSoundSystem->playSound(audioClip->GetSound(), sfx, true, &channel);
			break;
		};
		case 1:
		{

			result = mSoundSystem->playSound(audioClip->GetSound(), bgm, true, &channel);
			break;
		};
		case 2:
		{
			result = mSoundSystem->playSound(audioClip->GetSound(), ui, true, &channel);
			break;
		};
		case 3:
		{
			result = mSoundSystem->playSound(audioClip->GetSound(), editorSounds, true, &channel);
			break;
		};

		}
		
		channel->setVolume(volume);
		channel->setMode(isLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
		FMOD_VECTOR soundPosition = Vec3ToFMODVec3(soundPos);
		FMOD_VECTOR velocity = Vec3ToFMODVec3(vel);
		channel->set3DAttributes(&soundPosition, &velocity);

		if (result != FMOD_OK)
		{
			SLICE_LOG_ERROR("Failed to play sound");
			return nullptr;
		}



		return channel;

	}

	FMOD::Channel* AudioManager::PlayEditorPreview(GUID soundName, bool is3D)
	{
		//This call will handle the loading if the resource hasn't been loaded
		auto audioClip = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Audio>(soundName).get();

		FMOD::Channel* previewChannel = nullptr;

		if (!audioClip->GetDimension())
		{
			previewChannel->setMode(FMOD_2D);
		}

		FMOD_RESULT result = mSoundSystem->playSound(audioClip->GetSound(), editorSounds, false, &previewChannel);

		

		//track->soundPos3D = Vec3ToFMODVec3(soundPos);


		if (result != FMOD_OK)
		{
			SLICE_LOG_ERROR("Failed to play sound");
			return nullptr;
		}

		return previewChannel;
	}

	void AudioManager::SetMasterVolume(float volume)
	{
		
		master->setVolume(volume);
	}

	void AudioManager::SetCategoryVolume(GUID soundName, float volume)
	{
		auto audioClip = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Audio>(soundName).get();

		switch (audioClip->GetCategory())
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

	float AudioManager::GetChannelVolume(FMOD::Channel* channel)
	{
		float volume;
		return channel->getVolume(&volume);
	}

	void AudioManager::SetChannelVolume(FMOD::Channel* channel, float volume)
	{
		channel->setVolume(volume);
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

	void AudioManager::SetSound3DPosition(FMOD::Channel* channel, bool is3D, glm::vec3 soundPos, glm::vec3 vel)
	{
		//FMOD_VECTOR soundPosition = { soundPos.x, soundPos.y, soundPos.z };
		//FMOD_VECTOR velVec = { 0.0f,0.0f,0.0f };


		/*for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
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
		}*/
		FMOD_VECTOR soundPosition = Vec3ToFMODVec3(soundPos);
		FMOD_VECTOR velocity = Vec3ToFMODVec3(vel);

		channel->set3DAttributes(&soundPosition, &velocity);
		if (!is3D)
		{
			channel->set3DMinMaxDistance(0.0f, 0.0f);
		}
	}


	void AudioManager::UpdatePauseSound(FMOD::Channel* channel,bool isPaused)
	{
		channel->setPaused(isPaused);
	}

	bool AudioManager::GetPauseState(FMOD::Channel* channel)
	{

		bool pauseState = false;
		channel->getPaused(&pauseState);

		return pauseState;
	}

	bool AudioManager::IsFMOD3D(FMOD::Channel* channel)
	{
		FMOD_MODE currentMode = 0;
		channel->getMode(&currentMode);
		if (currentMode & FMOD_3D)
		{
			return true;
		}

		return false;
	}

	void AudioManager::UpdateFMODMode(FMOD::Channel* channel, bool is3D)
	{
		/*for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; i++)
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
		}*/
		if (is3D)
		{
			
			channel->setMode(FMOD_3D);

		}
		else
		{
			channel->setMode(FMOD_2D);
		}
	}

	void AudioManager::StopSound(FMOD::Channel* channel)
	{

		channel->stop();
		channel = nullptr;
	}

	void AudioManager::StopEditorPreview(FMOD::Channel* channel)
	{

		channel->stop();
		channel = nullptr;
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
}