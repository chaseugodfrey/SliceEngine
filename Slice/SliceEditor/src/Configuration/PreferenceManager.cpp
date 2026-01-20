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

	void PreferenceManager::UpdateVersion(nlohmann::json& preferences)
	{

	}

	void PreferenceManager::LoadPreferences()
	{
		std::string filepath = "preferences.json";
		std::ifstream preferencesFile{ filepath };

		mPreferences = std::make_unique<Preferences>();

		if (preferencesFile.fail())
		{
			SavePreferences();
			preferencesFile.open(filepath);
		}

		nlohmann::json preferencesJson;
		preferencesFile >> preferencesJson; 
		
		unsigned int version = preferencesJson["Version"].get<unsigned int>();
		if (version != PreferenceManager::CURRENT_VERSION)
		{
			// to fill upon version updates
			UpdateVersion(preferencesJson);
		}

		// Theme
		auto& p_theme = preferencesJson["Theme"];

		std::string p_theme_id = p_theme["ID"].get<std::string>();
		mPreferences->theme.ID = EditorUtilities::GetThemeTypeFromString(p_theme_id);

		// Scene
		auto& p_scene = preferencesJson["Scene"];

		uint64_t p_startingSceneGUID = p_scene["Starting Scene"].get<uint64_t>();
		mPreferences->scene.startingID = SliceEngine::GUID(p_startingSceneGUID);

		uint64_t p_lastSceneGUID = p_scene["Last Scene"].get<uint64_t>();
		mPreferences->scene.lastID = SliceEngine::GUID(p_lastSceneGUID);

		preferencesFile.close();
	}

	void PreferenceManager::SavePreferences()
	{
		std::string filepath = "preferences.json";
		std::ofstream preferencesFile{ filepath };
		nlohmann::json preferences;

		preferences["Version"] = PreferenceManager::CURRENT_VERSION;
		preferences["Theme"] =
		{
			{ "ID", EditorThemes[mPreferences->theme.ID] }
		};

		preferences["Scene"] =
		{
			{ "Starting Scene", mPreferences->scene.startingID },
			{ "Last Scene", mPreferences->scene.lastID }
		};

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
		auto& assetManager = registry.GetAssetManager();
		auto& scene = mPreferences->scene;
		auto scene_to_load_guid = scene.startingID != SliceEngine::GUID::null() ? scene.startingID : scene.lastID;
		auto scene_metadata_filename = assetManager.GetFilenameFromGUID(scene_to_load_guid);

		std::filesystem::path scene_filepath_to_load = "";
		if (scene_metadata_filename.has_value())
		{
			scene_filepath_to_load = assetManager.GetMetaDataFromFilename(scene_metadata_filename.value());
			scene_filepath_to_load.replace_extension("");
		}

		SliceEngine::Core::GetInstance()->GetSceneSystem()->LoadSceneIntoQueue(scene_filepath_to_load);
	}

}