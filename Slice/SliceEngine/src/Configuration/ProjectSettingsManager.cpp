#include <pch.h>
#include "ProjectSettingsManager.h"

namespace SliceEngine
{
	void ProjectSettingsManager::Init()
	{
		mSettingsList.push_back(std::make_unique<AudioSettings>("AudioSettings"));
		//mSettingsList.push_back(std::make_unique<PhysicsSettings>("PhysicsSettings"));

		for (auto& setting : mSettingsList)
		{
			setting->Init();
			setting->LoadFromFile();
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
		for (auto& setting : mSettingsList)
		{
			setting->Exit();
		}

		mSettingsList.clear();
	}
	
}