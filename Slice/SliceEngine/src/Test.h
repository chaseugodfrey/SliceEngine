#pragma once
namespace SliceEngine
{
	extern AudioSettings* mAudioSettings;

	void TestInit(FMOD::System* system);

	void TestCreate();

	void TestAddSound();

	void TestVolume(const std::string& key, float volume);

	void TestMaxInstances(const std::string& key, int maxInstance);

	void TestMinMaxDistance(const std::string& key, float minDist, float maxDist);

	void TestSpatialBlend(const std::string& key, float spatialBlend);

	void TestPlaySFX();

}
