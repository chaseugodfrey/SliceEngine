#include <pch.h>
#include "Configuration/AudioSettings.h"
#include "../src/Core/Core.h"
#include "Input/InputSystem.h"
#include "Resource/ResourceManager.h"
#include "Resource/Audio.h"
#include "Test.h"

namespace SliceEngine
{
	AudioSettings* mAudioSettings = nullptr;

	void TestInit(FMOD::System* system)
	{
		mAudioSettings->Init(system);
	}

	void TestCreate()
	{
		mAudioSettings->CreateSoundGroup("Hit_Slime.Single");
		mAudioSettings->CreateSoundGroup("Land");
	}

	void TestAddSound()
	{
		//mAudioSettings->SetSoundGroup("A1", "Hit_Slime.Single");
		//mAudioSettings->SetSoundGroup("A2", "Hit_Slime.Single");
	}

	void TestVolume(const std::string& key, float volume)
	{
		
		if (std::abs(volume - mAudioSettings->GetSoundGroupVolume(key)) > 0.001f)
		{
			mAudioSettings->SetSoundGroupVolume(key, volume);
		}
	}

	void TestMaxInstances(const std::string& key, int maxInstance)
	{

		if (mAudioSettings->GetMaxInstances(key) != maxInstance)
		{
			mAudioSettings->SetMaxInstances(key, maxInstance);
		}
	}

	void TestMinMaxDistance(const std::string& key, float minDist, float maxDist)
	{
		if (std::abs(minDist - mAudioSettings->GetMinDistance(key)) > 0.001f)
		{
			mAudioSettings->SetMinDistance(key, minDist);
		}

		if (std::abs(maxDist - mAudioSettings->GetMinDistance(key)) > 0.001f)
		{
			mAudioSettings->SetMaxDistance(key, maxDist);
		}
	}

	void TestSpatialBlend(const std::string& key, float spatialBlend)
	{
		if (std::abs(spatialBlend - mAudioSettings->GetSoundGroupSpatialBlend(key)) > 0.001f)
		{
			mAudioSettings->SetSoundGroupSpatialBlend(key, spatialBlend);
		}

	}

	void TestPlaySFX()
	{
		mAudioSettings->PlaySFX("Land");
		mAudioSettings->PlaySFX("Hit_Slime.Single");
	}

}

