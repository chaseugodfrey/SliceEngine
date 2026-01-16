#include <pch.h>
#include "PreferenceManager.h"
#include <Systems/SceneSystem.h>

namespace SliceEditor
{
	void PreferenceManager::Init()
	{
		LoadPreferences();

		// check starting scene path
		// if blank, open default scene
		// if not, set starting scene to path from guid

		auto resourceManager = SliceEngine::Core::GetInstance()->GetResourceManager();
		auto sceneManager = SliceEngine::Core::GetInstance()->GetSceneSystem();

		if (resourceManager->CheckResource(mPreferences->StartingSceneGUID))
		{
			auto sceneHandle = resourceManager->get<SliceEngine::SliceEngineTypes::Scene>(mPreferences->StartingSceneGUID);

			if (sceneHandle.IsValid())
			{
				sceneManager->SetCurrentScenePath(sceneHandle->GetFilePath());
			}
		}
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
		mPreferences->Theme = EditorUtilities::GetThemeTypeFromString(p_theme);

		SliceEngine::GUID p_startingSceneGUID = preferencesJson["StartingSceneGUID"].get<SliceEngine::GUID>();
		mPreferences->StartingSceneGUID = p_startingSceneGUID;

		preferencesFile.close();

		SetPreferences();
	}

	void PreferenceManager::SavePreferences()
	{
		std::string filepath = "preferences.json";
		std::ofstream preferencesFile{ filepath };
		nlohmann::json preferences;

		preferences["Theme"] = EditorThemes[mPreferences->Theme];

		preferencesFile << preferences.dump();
		preferencesFile.close();

		SetPreferences();
	}

	Preferences& PreferenceManager::GetPreferences()
	{
		return *mPreferences;
	}

	void PreferenceManager::SetPreferences()
	{
		EditorUtilities::SetTheme(mPreferences->Theme);
	}

}