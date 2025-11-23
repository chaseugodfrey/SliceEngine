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
		mPrefabInspected = false;
		auto* eventManager = EventManager::GetInstance();

		eventManager->Subscribe<OnSceneLoadedEvent, &SessionManager::OnSceneChange>(this);
		eventManager->Subscribe<OnSceneStopEvent, &SessionManager::OnSceneStop>(this);
		eventManager->Subscribe<AssetFileChangedEvent, &SessionManager::OnAssetFileChanged>(this);
		eventManager->Subscribe<PrefabInspectedEvent, &SessionManager::PrefabInspected>(this);
		OpenPreferences();
	}

	void SessionManager::Update()
	{
		CreateEntityNodes();

		if (mPrefabInspected)
		{

		}
		else
		{
			mPrefabNodes.clear();
		}
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

	void SessionManager::CreatePrefabNodes()
	{
		//auto& sceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(mPrefabParent.get()->entity);
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

	void SessionManager::PrefabInspected(const PrefabInspectedEvent& event)
	{
		//Set mPrefabInspected
		mPrefabInspected = event.prefabBeingInspected;

		//SceneGraph Building
		auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
		//Prefab  now being inspected
		if (event.prefabBeingInspected)
		{
			//Get the Prefab Handle
			SliceEngine::Handle<SliceEngine::SliceEngineTypes::Prefab> prefab = rm->get<SliceEngine::SliceEngineTypes::Prefab>(event.prefabGUID);
			//Set the rootEntity of the prefab
			mPrefabRootEntity = SliceEngine::JSONSerializer::DeserializePrefab(prefab.get()->filePath);
			//Clear the look-up table just incase
			mPrefabNodes.clear();
			//Build the mPrefabNodes lookup table
			BuildPrefabTree(mPrefabRootEntity);
		}
		//Prefab no longer being inspected
		else
		{
			//Delete the Root Entity from GOFactory
			SliceEngine::Core::GetInstance()->GetRegistry().destroy(mPrefabRootEntity);
			
			//Clear Session Manager Variables
			mPrefabRootEntity = entt::null;
			mPrefabNodes.clear();


		}
	}

	void SessionManager::BuildPrefabTree(Entity entity)
	{
		//Get Entity's SceneGraph
		auto& registry = SliceEngine::Core::GetInstance()->GetRegistry();
		auto& sceneGraph = registry.get<SliceEngine::SceneGraph>(entity);

		//Add the parent to mPrefabNodes (just a lookup table)
		auto pair = mPrefabNodes.try_emplace(entity, std::make_unique<EntityNode>());
		auto& prefabNodePtr = pair.first->second;
		EntityNode& prefabNode = *prefabNodePtr;
		prefabNode.entity = entity;
		prefabNode.type = SelectionType::PREFAB_ENTITY;
		prefabNode.isSelected = false;

		//First child of this entity
		Entity childEntity = sceneGraph.neighbours[SliceEngine::SceneGraph::Direction::DOWN];

		while (childEntity != entt::null)
		{

			//Recursively build the child's subtree
			BuildPrefabTree(childEntity);

			// Move to next sibling via RIGHT
			auto& childSceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(childEntity);

			childEntity = childSceneGraph.neighbours[SliceEngine::SceneGraph::Direction::RIGHT];
		}
	}

	bool SessionManager::IsPrefabInspected()
	{
		return mPrefabInspected;
	}

	Entity SessionManager::GetPrefabInspected()
	{
		return mPrefabRootEntity;
	}

	std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& SessionManager::GetEntityNodes()
	{
		return mEntityNodes;
	}

	std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& SessionManager::GetPrefabNodes()
	{
		return mPrefabNodes;
	}




}
