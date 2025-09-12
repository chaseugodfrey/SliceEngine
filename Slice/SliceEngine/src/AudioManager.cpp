#include <pch.h>
#include "AudioManager.h"

namespace SliceEngine
{
	void AudioManager::Init()
	{
		SLICE_LOG("Initializing FMOD Studio.");
		FMOD_RESULT result = FMOD::System_Create(&mSoundSystem);
		if (result != FMOD_OK)
		{
			SLICE_LOG("FMOD System creation failed");
			return;
		}

		result = mSoundSystem->init(MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr);
		if (result != FMOD_OK)
		{
			SLICE_LOG("FMOD System initialization failed");
			return;
		}
	}

	void AudioManager::LoadSound(const std::string& soundName, const std::string& soundFile, bool is3D, bool loop)
	{
		FMOD_MODE eMode = FMOD_DEFAULT;

		if (is3D)
		{
			eMode |= FMOD_3D;
		}
		else
		{
			eMode |= FMOD_2D;
		}

		if (loop)
		{
			eMode |= FMOD_LOOP_NORMAL;
		}
		else
		{
			eMode |= FMOD_LOOP_OFF;
		}

		FMOD::Sound* sound{ nullptr };
		mSoundSystem->createSound(soundFile.c_str(), eMode, nullptr, &sound);

		if (sound)
		{
			auto track = std::make_unique<SoundTrack>();
			track->sound = sound;
			track->isLooping = loop;

			mLoadedSounds.try_emplace(soundName, std::move(track));
			SLICE_LOG("Sound Loaded");
			return;
		}
	}

	void AudioManager::Update()
	{
		
	}

	bool AudioManager::PlaySound(const std::string& soundName, SoundCategory category, InternalSound internalCategory, bool isLoop, float volume)
	{
		auto it = mLoadedSounds.find(soundName);
		if (it == mLoadedSounds.end())
		{
			SLICE_LOG("Sound not loaded");
			return false;
		}

		SoundTrack* track = it->second.get();
		FMOD::Channel* channel = nullptr;

		FMOD_RESULT result = mSoundSystem->playSound(track->sound, nullptr, false, &channel);
		if (result != FMOD_OK)
		{
			SLICE_LOG("Failed to play sound");
			return false;
		}

		track->channel = channel;
		track->category = category;
		track->isLooping = isLoop;
		track->currentSoundVolume = volume;

		if (channel)
		{
			channel->setVolume(volume);
			channel->setMode(isLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

			// Only track the channel, not duplicate the sound
			auto track = std::make_unique<SoundTrack>();
			track->channel = channel;
			track->category = category;
			track->isLooping = isLoop;
			track->currentSoundVolume = volume;

			mSound[internalCategory].emplace_back(std::move(track));
			return true;
		}

		return false;
		
	}

	void AudioManager::StopSound(InternalSound InternalCategory)
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
		/*for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; ++i)
		{

			StopSound(static_cast<InternalSound>(i));

		}*/

		for (int i = 0; i < InternalSound::SOUND_MAX_SOUNDS; ++i)
		{
			for (auto& track : mSound[static_cast<InternalSound>(i)])
			{
				if (track->channel)
				{
					track->channel->stop();
					track->channel = nullptr;
				}
			}
			mSound[static_cast<InternalSound>(i)].clear();
		}

		// Now release sounds only once, from mLoadedSounds
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