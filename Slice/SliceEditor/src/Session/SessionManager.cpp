#include <pch.h>
#include "SessionManager.h"
#include "Selection/SelectionManager.h"
#include "ContentBrowser/ContentBrowserManager.h"
#include <Core/EventManager.h>

namespace SliceEditor
{
	SessionManager::SessionManager(Registry& reg) : IBaseManager(reg) {}

	SessionManager::~SessionManager() {}

	void SessionManager::Init()
	{
		auto* eventManager = EventManager::GetInstance();

		eventManager->Subscribe<OnSceneLoadedEvent, &SessionManager::OnSceneChange>(this);
		eventManager->Subscribe<OnSceneStopEvent, &SessionManager::OnSceneStop>(this);
		eventManager->Subscribe<AssetFileChangedEvent, &SessionManager::OnAssetFileChanged>(this);

		mAnimatorData = std::make_unique<AnimatorData>();
		OpenPreferences();
	}

	void SessionManager::Update()
	{
		CreateEntityNodes();
	}

	void SessionManager::OpenPreferences()
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
		preferencesFile >> preferencesJson; // here aloy
		std::string theme = preferencesJson["Theme"].get<std::string>();
		mPreferences->Theme = EditorUtilities::GetThemeTypeFromString(theme);

		preferencesFile.close();

		SetPreferences();
	}

	void SessionManager::SetPreferences()
	{
		EditorUtilities::SetTheme(mPreferences->Theme);
	}

	void SessionManager::CreateDefaultPreferenceFile()
	{
		std::string filepath = "preferences.json";
		std::ofstream preferencesFile{ filepath };
		nlohmann::json preferences;

		preferences["Theme"] = EditorThemes[0];

		preferencesFile << preferences.dump();
		preferencesFile.close();
	}

	void SessionManager::SavePreferences()
	{
		std::string filepath = "preferences.json";
		std::ofstream preferencesFile{ filepath };
		nlohmann::json preferences;

		preferences["Theme"] = EditorThemes[mPreferences->Theme];

		preferencesFile << preferences.dump();
		preferencesFile.close();

		SetPreferences();
	}

	Preferences& SessionManager::GetPreferences()
	{
		return *mPreferences.get();
	}

	void SessionManager::CreateEntityNodes()
	{
		auto view = SliceEngine::Core::GetInstance()->GetRegistry().view<SliceEngine::SceneGraph>();

		if (view.size() != mEntityNodes.size())
		{
			mEntityNodes.clear();
			for (auto entity : view)
			{
				mEntityNodes.emplace(entity, std::make_unique<EntityNode>(entity));
			}
		}
	}

	void SessionManager::OnSceneChange(const OnSceneLoadedEvent& event)
	{
		if (event.isSceneLoaded)
		{
			mEntityNodes.clear();
			CreateEntityNodes();
		}
	}

	void SessionManager::OnSceneStop(const OnSceneStopEvent& event)
	{
		if (event.isSceneStopped)
		{
			EditorUtilities::Scene_Stop(*registry.GetManager<SelectionManager>("Selection"));
		}
	}

	void SessionManager::OnAssetFileChanged(const AssetFileChangedEvent& event)
	{
		if (event.assetChangeSuccess == true)
		{
			EditorUtilities::ContentBrowser_Refresh(*registry.GetManager<ContentBrowserManager>("ContentBrowser"));
		}
	}

	std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& SessionManager::GetEntityNodes()
	{
		return mEntityNodes;
	}

	void SessionManager::LoadAnimatorData(SliceEngine::GUID guid)
	{
		auto filename = registry.GetAssetManager().GetFilenameFromGUID(guid);

		if (!filename.has_value())
			return SLICE_LOG_ERROR(".controller filename is wrong!");

		std::filesystem::path filepath = registry.GetAssetManager().mAssetDirectory.string() + "/" + filename.value() + ".controller";

		if (!mAnimatorData->empty())
			mAnimatorData->reset();

		if (!mAnimatorData->Load(filepath))
		{
			mAnimatorData.reset();
			SLICE_LOG_ERROR("Animator Data not loaded.");
		}
	}

	void SessionManager::ClearAnimatorData()
	{
		mAnimatorData->reset();
	}

	AnimatorData* SessionManager::GetAnimatorData()
	{
		return mAnimatorData.get();
	}
}
