#include <pch.h>
#include "SessionManager.h"
#include "Selection/SelectionManager.h"
#include "ContentBrowser/ContentBrowserManager.h"
#include "../../SliceEngine/src/Systems/PrefabSystem.h"
#include <Core/EventManager.h>
#include <WindowManager/WindowManager.h>

#include <Systems/SceneSystem.h>
#include <Systems/PrefabSystem.h>

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
		eventManager->Subscribe<OnSceneSaveEvent, &SessionManager::OnSceneSave>(this);
		eventManager->Subscribe<AssetFileChangedEvent, &SessionManager::OnAssetFileChanged>(this);
		eventManager->Subscribe<PrefabInspectedEvent, &SessionManager::PrefabInspected>(this);

		mAnimatorData = std::make_unique<AnimatorData>();
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

	void SessionManager::OnSceneSave(OnSceneSaveEvent e)
	{
		auto windowManager = registry.GetManager<WindowManager>("Windows");

		if (isSavingScene)
			return;

		isSavingScene = true;
		isSavingDone = false;
		windowManager->OpenSaveScenePopup();

		std::thread([&, windowManager]()
			{
				std::chrono::steady_clock::time_point before = std::chrono::steady_clock::now();
				EditorUtilities::Scene_Save();
				std::chrono::steady_clock::time_point after = std::chrono::steady_clock::now();
				std::chrono::duration<double, std::milli> dur = after - before;
				duration = dur.count();

				SLICE_LOG("Scene saving took (" + std::to_string(duration) + "ms.)");

				isSavingScene = false;
				isSavingDone = true;
				windowManager->CloseSaveScenePopup();

			}).detach();
	}

	void SessionManager::SetNodeAsPrefab(EntityNode* node, bool isPrefab)
	{
		node->isPrefab = isPrefab;
		SliceEngine::GameObject go = SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(node->entity);

		if (go.HasComponent<SliceEngine::SceneGraph>())
		{
			auto& sceneGraph = go.GetComponent<SliceEngine::SceneGraph>();

			auto childEntity = sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN];

			while (childEntity != entt::null)
			{
				//Need to find the EntityNode in the editor's map
				if (mEntityNodes.find(childEntity) == mEntityNodes.end())
				{
					SLICE_LOG_WARNING("De-sync of mEntityNodes!");
					return;
				}
				auto childEntityNode = mEntityNodes[childEntity].get();
				
				SetNodeAsPrefab(childEntityNode, isPrefab);
				//Get SceneGraph component and update
				auto childGO = SliceEngine::FactoryInstance.GetGOByEntity(childEntity);
				auto& childSceneGraph = childGO.GetComponent<SliceEngine::SceneGraph>();
				childEntity = childSceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT];
			}
		}
	}

	void SessionManager::CreateEntityNodes()
	{
		auto view = SliceEngine::Core::GetInstance()->GetRegistry().view<SliceEngine::SceneGraph>();
		auto prefabView = SliceEngine::Core::GetInstance()->GetRegistry().view<SliceEngine::Prefab>();

		if (view.size() != mEntityNodes.size())
		{
			mEntityNodes.clear();
			for (auto entity : view)
			{
				mEntityNodes.emplace(entity, std::make_unique<EntityNode>(entity));
			}

			for (auto entity : prefabView)
			{
				mEntityNodes[entity].get()->isPrefab = true;
			}
		}
		
		if (prefabView.size() != mPrefabNodes.size())
		{
			mPrefabNodes.clear();
			for (auto entity : prefabView)
			{
				mPrefabNodes.emplace(entity, std::make_unique<EntityNode>(entity));
				mPrefabNodes[entity].get()->isPrefab = true;
				mPrefabNodes[entity].get()->type = SelectionType::PREFAB_ENTITY;
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
			//Create the Prefab Instance
			mPrefabRootEntity = SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().CreatePrefab(event.prefabGUID, true).GetEntity();
			//Clear the look-up table just incase
			mPrefabNodes.clear();
			//Build the mPrefabNodes lookup table
			BuildPrefabTree(mPrefabRootEntity);
		}
		//Prefab no longer being inspected
		else
		{
			//Delete the Root Entity from GOFactory
			//SliceEngine::FactoryInstance.Destroy(mPrefabRootEntity);
			
			//Clear Session Manager Variables
			mPrefabRootEntity = entt::null;
			mPrefabNodes.clear();

			registry.GetManager<SelectionManager>("Selection")->ClearSelection();
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
		prefabNode.isPrefab = true;
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
