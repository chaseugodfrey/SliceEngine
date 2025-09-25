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
			SLICE_LOG_ERROR("FMOD System creation failed");
			return;
		}

		result = mSoundSystem->init(MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr);
		if (result != FMOD_OK)
		{
			SLICE_LOG_ERROR("FMOD System initialization failed");
			return;
		}
	}

	void AudioManager::LoadSound(const std::string& soundFile)
	{

		FMOD::Sound* sound{ nullptr };

		std::string soundName;
		mSoundSystem->createSound(soundFile.c_str(), FMOD_3D, nullptr, &sound);

		soundName = soundFile.substr(soundFile.find_last_of("/") + 1, soundFile.find_last_of(".") - soundFile.find_last_of("/")-1);

		if (sound)
		{

			/*if (is3D)
			{
				auto track = std::make_unique<SoundTrack3D>();
				track->sound = sound;
				mLoadedSounds3D.try_emplace(soundName, std::move(track));
			}
			else
			{
				auto track = std::make_unique<SoundTrack2D>();
				track->sound = sound;
				mLoadedSounds2D.try_emplace(soundName, std::move(track));
			}*/

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

	bool AudioManager::PlaySound(const std::string& soundName, SoundCategory category, InternalSound internalCategory, bool is3D, bool isLoop, float volume, Entity id)
	{


		//	if (channel)
		//	{
		//		/*channel->setVolume(volume);
		//		channel->setMode(isLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);*/

		//		// Only track the channel, not duplicate the sound

		//		track->channel = channel;
		//		track->category = category;
		//		track->isLooping = isLoop;
		//		track->currentSoundVolume = volume;

		//		track->ApplySettings();

		//		mSound3D[internalCategory].emplace_back(std::move(track));
		//		return true;
		//	}
		//}
		//else
		//{
		//	auto it = mLoadedSounds2D.find(soundName);
		//	if (it == mLoadedSounds2D.end())
		//	{
		//		SLICE_LOG("Sound not loaded");
		//		return false;
		//	}

		//	auto track = std::make_unique<SoundTrack2D>();

		//	FMOD::Channel* channel = nullptr;

		//	FMOD_RESULT result = mSoundSystem->playSound(it->second.get()->sound, nullptr, false, &channel);
		//	if (result != FMOD_OK)
		//	{
		//		SLICE_LOG("Failed to play sound");
		//		return false;
		//	}


		//	if (channel)
		//	{
		//		/*channel->setVolume(volume);
		//		channel->setMode(isLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);*/

		//		// Only track the channel, not duplicate the sound

		//		track->channel = channel;
		//		track->category = category;
		//		track->isLooping = isLoop;
		//		track->currentSoundVolume = volume;

		//		track->ApplySettings();

		//		mSound2D[internalCategory].emplace_back(std::move(track));
		//		return true;
		//	}
		//}

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

		FMOD::Channel* channel = nullptr;

		FMOD_RESULT result = mSoundSystem->playSound(it->second.get()->sound, nullptr, false, &channel);

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
			track->currentSoundVolume = volume;
			track->entityID = id;
			
			track->ApplySettings();

			mSound[internalCategory].emplace_back(std::move(track));
			return true;
		}




		return false;

	}

	void AudioManager::SetMasterVolume(float volume)
	{
		mMasterVolume = std::clamp(volume, 0.0f, 1.0f);

		for (int i{}; i < InternalSound::SOUND_MAX_SOUNDS; ++i)
		{
			for (auto it = mSound[i].begin(); it != mSound[i].end(); ++it)
			{

				UpdateSoundVolume(it->get(), SoundCategory::SFX);

			}
		}
	}

	void AudioManager::SetCategoryVolume(SoundCategory category, InternalSound internalCategory, float volume)
	{
		mCategoryVolumes[category] = std::clamp(volume, 0.0f, 1.0f);
		for (auto it = mSound[internalCategory].begin(); it != mSound[internalCategory].end(); ++it)
		{
			if (it->get()->channel && it->get()->category == category)
			{
				UpdateSoundVolume(it->get(), category);
			}

		}
	}

	float AudioManager::GetCategoryVolume(SoundCategory category)
	{
		return mCategoryVolumes[category];
	}

	float AudioManager::CalculateFinalVolume(const SoundTrack* track, SoundCategory category) const
	{
		return track->currentSoundVolume * mCategoryVolumes.at(category) * mMasterVolume;
	}

	void AudioManager::UpdateSoundVolume(SoundTrack* track, SoundCategory category)
	{
		if (!track->channel)
		{
			return;
		}

		float finalVolume = CalculateFinalVolume(track, category);
		track->channel->setVolume(finalVolume);
	}

	void AudioManager::SetListenerAttributes(glm::vec3& pos, glm::vec3& vel, glm::vec3& forward, glm::vec3& up)
	{
		FMOD_VECTOR positionVec = { pos.x, pos.y, pos.z };
		FMOD_VECTOR forwardVec = { forward.x, forward.y, forward.z };
		FMOD_VECTOR velVec = { vel.x, vel.y, vel.z };
		FMOD_VECTOR upVec = { up.x, up.y, up.z };

		mSoundSystem->set3DListenerAttributes(0, &positionVec, &velVec, &forwardVec, &upVec);
		//mSoundSystem->update();
	}

	void AudioManager::SetSound3DPosition(const std::string& soundName, glm::vec3 soundPos)
	{
		FMOD_VECTOR soundPosition = { soundPos.x, soundPos.y, soundPos.z };
		FMOD_VECTOR velVec = { 0.0f,0.0f,0.0f };

		/*auto it = mLoadedSounds3D.find(soundName);
		if (it == mLoadedSounds3D.end())
		{
			SLICE_LOG("Sound not loaded");
			return;
		}*/

		auto it = mLoadedSounds.find(soundName);
		if (it == mLoadedSounds.end())
		{
			SLICE_LOG_ERROR("Sound Not Loaded");
			return;
		}


		if (it->second->channel)
		{
			it->second->channel->set3DAttributes(&soundPosition, &velVec);
			it->second->channel->set3DMinMaxDistance(0.1f, 0.6f);
		}
	}

	void AudioManager::StopAllSound(InternalSound InternalCategory)
	{
		/*for (auto it = mSound2D[InternalCategory].begin(); it != mSound2D[InternalCategory].end(); ++it)
		{
			if (it->get()->channel)
			{
				it->get()->channel->stop();
			}
		}

		mSound2D[InternalCategory].clear();

		for (auto it = mSound3D[InternalCategory].begin(); it != mSound3D[InternalCategory].end(); ++it)
		{
			if (it->get()->channel)
			{
				it->get()->channel->stop();
			}
		}

		mSound3D[InternalCategory].clear();*/

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

		/*for (auto& pair : mLoadedSounds2D)
		{
			if (pair.second->sound)
			{
				pair.second->sound->release();
				pair.second->sound = nullptr;
			}
		}
		mLoadedSounds2D.clear();

		for (auto& pair : mLoadedSounds3D)
		{
			if (pair.second->sound)
			{
				pair.second->sound->release();
				pair.second->sound = nullptr;
			}
		}
		mLoadedSounds3D.clear();*/
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