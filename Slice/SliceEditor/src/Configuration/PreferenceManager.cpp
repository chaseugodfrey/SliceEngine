#include <pch.h>
#include "PreferenceManager.h"
#include <Systems/SceneSystem.h>

namespace SliceEditor
{
	void PreferenceManager::Init()
	{

	}

	void PreferenceManager::Update()
	{

	}

	void PreferenceManager::CreateDefaultPreferenceFile()
	{
		std::string filepath = "preferences.json";
		std::ofstream preferencesFile{ filepath };
		nlohmann::json preferences;

		preferences["Theme"] = EditorThemes[0];
		preferences["StartingSceneGUID"] = SliceEngine::GUID::null();

		preferencesFile << preferences.dump();
		preferencesFile.close();
	}

	void PreferenceManager::LoadPreferences()
	{
		std::string filepath = "preferences.json";
		std::ifstream preferencesFile{ filepath };

		if (preferencesFile.fail())
		{
			CreateDefaultPreferenceFile();
			preferencesFile.open(filepath);
		}

		mPreferences = std::make_unique<Preferences>();

		nlohmann::json preferencesJson;
		preferencesFile >> preferencesJson; 

		std::string p_theme = preferencesJson["Theme"].get<std::string>();
		mPreferences->theme.ID = EditorUtilities::GetThemeTypeFromString(p_theme);

		uint64_t p_startingSceneGUID = preferencesJson["Starting Scene GUID"].get<uint64_t>();
		mPreferences->scene.startingID = SliceEngine::GUID(p_startingSceneGUID);

		preferencesFile.close();
	}

	void PreferenceManager::SavePreferences()
	{
		std::string filepath = "preferences.json";
		std::ofstream preferencesFile{ filepath };
		nlohmann::json preferences;

		preferences["Theme"] = EditorThemes[mPreferences->theme.ID];
		preferences["Starting Scene GUID"] = mPreferences->scene.startingID;

		preferencesFile << preferences.dump(4);
		preferencesFile.close();

		SetPreferences();
	}

	Preferences& PreferenceManager::GetPreferences()
	{
		return *mPreferences;
	}

	void PreferenceManager::SetPreferences()
	{
		// Theme
		auto& theme = mPreferences->theme;
		EditorUtilities::SetTheme(theme.ID);

		// Scene
		auto& scene = mPreferences->scene;
		auto scene_to_load = scene.startingID != SliceEngine::GUID::null() ? scene.startingID : scene.lastID;
		SliceEngine::Core::GetInstance()->GetSceneSystem()->SetCurrentScenePath(scene_to_load);
	}

}