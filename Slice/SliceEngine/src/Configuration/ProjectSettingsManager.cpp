#include <pch.h>
#include "ProjectSettingsManager.h"

namespace SliceEngine
{
	void ProjectSettingsManager::Init()
	{
		mSettingsList.push_back(std::make_unique<AudioSettings>("AudioSettings"));
		mSettingsList.push_back(std::make_unique<PhysicsSettings>("PhysicsSettings"));

		Load();
	}

	void ProjectSettingsManager::Load()
	{
		for (auto& setting : mSettingsList)
		{
			setting->LoadSettings();
		}
	}

	void ProjectSettingsManager::Save()
	{
		for (auto& setting : mSettingsList)
		{
			setting->SaveSettings();
		}
	}

	void ProjectSettingsManager::Exit()
	{
		mSettingsList.clear();
	}
	
}