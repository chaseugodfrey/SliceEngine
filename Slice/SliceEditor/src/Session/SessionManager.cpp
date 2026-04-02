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
		mShowHierarchyEntityIDs = false;
		mHighlightGOs = false;
		auto* eventManager = EventManager::GetInstance();

		eventManager->Subscribe<OnSceneLoadedEvent, &SessionManager::OnSceneChange>(this);
		eventManager->Subscribe<OnSceneStopEvent, &SessionManager::OnSceneStop>(this);
		eventManager->Subscribe<OnSceneSaveEvent, &SessionManager::OnSceneSave>(this);
		eventManager->Subscribe<AssetFileChangedEvent, &SessionManager::OnAssetFileChanged>(this);
		eventManager->Subscribe<PrefabInspectedEvent, &SessionManager::PrefabInspected>(this);
		eventManager->Subscribe<ShaderGraphInspectedEvent, &SessionManager::ShaderGraphInspected>(this);
		eventManager->Subscribe<GameObjectScriptSelectedUpdate, &SessionManager::HighlightGameObjects>(this);

		mAnimatorData = std::make_unique<AnimatorData>();
		//CreateEntityNodes();
	}

	void SessionManager::Update()
	{
		UpdateEntityNodes();

		if (!mPrefabInspected && !mPrefabNodes.empty())
		{
			mPrefabNodes.clear();
		}

		//if (mHighlightGOs)
		//{
		//	//mGOScriptTimer -= static_cast<float>(SliceEngine::Core::GetInstance()->GetFramerateManager()->getDeltaTime());

		//	for (auto entity : mHighlightedGameObjects)
		//	{
		//		if (mEntityNodes.find(entity) != mEntityNodes.end())
		//		{
		//			mEntityNodes[entity].get()->isScriptSelected = false;
		//		}
		//	}

		//	mHighlightedGameObjects.clear();
		//	mHighlightGOs = false;
		//}
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

	void SessionManager::UpdateEntityNodes()
	{
		//auto selectionMan = registry.GetManager<SelectionManager>("Selection");
		auto view = SliceEngine::Core::GetInstance()->GetRegistry().view<SliceEngine::SliceEntity>();
		auto isPrefabView = SliceEngine::Core::GetInstance()->GetRegistry().view<SliceEngine::Prefab>();
		auto prefabEditorView = SliceEngine::Core::GetInstance()->GetRegistry().view<SliceEngine::PrefabEditingEntity>();

		//SLICE_LOG_DEBUG( "Prefab Editing Entity Size: " + std::to_string(prefabEditorView.size()));
		//Set the nodeOpen to false every frame because i dont really have time to rework the whole thing
		/*for (auto& node : mEntityNodes)
		{
			node.second.get()->nodeOpen = false;
		}*/

		//EntityNode Map for Hierarchy
		if (view.size() != mEntityNodes.size())
		{
			//Set the seen to false for removal checking ltr on
			for (auto& [entity,node] : mEntityNodes)
			{
				node->seen = false;
			}

			for (auto entity : view)
			{
				//Checking for un-added entities
				if (mEntityNodes.find(entity) == mEntityNodes.end())
				{
					AddEntityNode(entity);
				}
				mEntityNodes[entity]->seen = true;
			}

			//Check for prefab Component
			for (auto entity : isPrefabView)
			{
				auto it = mEntityNodes.find(entity);
				if(it != mEntityNodes.end())
				{
					it->second->isPrefab = true;
				}
			}

			//Removal of no longer existing entities (Check for removal after looping thru once to set seen to true
			for (auto it = mEntityNodes.begin(); it != mEntityNodes.end(); )
			{
				//Was not found in the scene
				if (it->second->seen == false)
				{
					it = mEntityNodes.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		//PrefabNode Map for Prefab Editor
		if (prefabEditorView.size() != mPrefabNodes.size())
		{
			for (auto& [entity, node] : mPrefabNodes)
			{
				node->seen = false;
			}

			for (auto entity : prefabEditorView)
			{
				//Checking for un-added entities
				if (mPrefabNodes.find(entity) == mPrefabNodes.end())
				{
					mPrefabNodes.try_emplace(entity, std::make_unique<EntityNode>(entity));
					mPrefabNodes[entity].get()->type = SelectionType::PREFAB_ENTITY;
				}

				mPrefabNodes[entity]->seen = true;
			}

			//Removal of no longer existing entities (Check for removal after looping thru once to set seen to true
			for (auto it = mPrefabNodes.begin(); it != mPrefabNodes.end(); )
			{
				//Was not found in the scene
				if (it->second->seen == false)
				{
					it = mPrefabNodes.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}

	void SessionManager::AddEntityNode(entt::entity entity)
	{
		mEntityNodes.try_emplace(entity, std::make_unique<EntityNode>(entity));
	}

	void SessionManager::RemoveEntityNode(entt::entity entity)
	{
		mEntityNodes.erase(entity);
	}

	void SessionManager::HighlightGameObjects(const GameObjectScriptSelectedUpdate& event)
	{
		//mGOScriptTimer = 5.0f; //TODO Add to Preferences
		mHighlightGOs = true;
		if(event.toAdd) //Adding to the highlighted game objects
		{
			mHighlightedGameObjects.push_back(event.entity);
			if (mEntityNodes.find(event.entity) != mEntityNodes.end())
			{
				mEntityNodes[event.entity].get()->isScriptSelected = true;
			}
		}
		else //Remove from highlighted GOs
		{
			mHighlightedGameObjects.erase
			(
				std::remove(mHighlightedGameObjects.begin(), mHighlightedGameObjects.end(), event.entity), mHighlightedGameObjects.end()
			);

			if (mEntityNodes.find(event.entity) != mEntityNodes.end())
			{
				mEntityNodes[event.entity].get()->isScriptSelected = false;
			}
		}
	}
		
	void SessionManager::OnSceneChange(const OnSceneLoadedEvent& event)
	{
		mEntityNodes.clear();
		UpdateEntityNodes();
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
			//EditorUtilities::ContentBrowser_Refresh(*registry.GetManager<ContentBrowserManager>("ContentBrowser"));
		}
	}

	void SessionManager::PrefabInspected(const PrefabInspectedEvent& event)
	{
		//Set mPrefabInspected
		mPrefabInspected = event.prefabBeingInspected;

		//SceneGraph Building
		//auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
		//Prefab  now being inspected
		if (event.prefabBeingInspected)
		{
			//Create the Prefab Instance
			mPrefabRootEntity = SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().CreatePrefab(event.prefabGUID, true).GetEntity();
			mInspectedPrefabGUID = event.prefabGUID;
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
			
			//Delete the Root Entity and its children mwahahahaa
			EditorUtilities::GameObject_Destroy(mPrefabRootEntity);

			//Clear Session Manager Variables
			mPrefabRootEntity = entt::null;
			mInspectedPrefabGUID = SliceEngine::GUID::null();
			mPrefabNodes.clear();

			registry.GetManager<SelectionManager>("Selection")->ClearSelection();

		}
	}

	void SessionManager::BuildPrefabTree(Entity entity)
	{
		//Get Entity's SceneGraph
		auto& engineRegistry = SliceEngine::Core::GetInstance()->GetRegistry();
		auto& sceneGraph = engineRegistry.get<SliceEngine::SceneGraph>(entity);

		//Add the parent to mPrefabNodes (just a lookup table)
		//auto pair = mPrefabNodes.try_emplace(entity, std::make_unique<EntityNode>());
		//auto& prefabNodePtr = pair.first->second;
		//EntityNode& prefabNode = *prefabNodePtr;
		//prefabNode.entity = entity;
		////prefabNode.isPrefab = true;
		//prefabNode.type = SelectionType::PREFAB_ENTITY;
		//prefabNode.isSelected = false;

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

	Entity SessionManager::GetPrefabEntityInspected()
	{
		return mPrefabRootEntity;
	}

	SliceEngine::GUID SessionManager::GetPrefabGUIDInspected()
	{
		return mInspectedPrefabGUID;
	}

	SliceEngine::GUID SessionManager::GetShaderGraphInspected()
	{
		return mInspectedShaderGraphGUID;
	}

	void SessionManager::ShaderGraphInspected(const ShaderGraphInspectedEvent& event)
	{
		mInspectedShaderGraphGUID = event.shaderGraphGUID;
	}

	void SessionManager::ToggleHierarchyEntityIDs()
	{
		mShowHierarchyEntityIDs = !mShowHierarchyEntityIDs;
	}

	bool SessionManager::GetHierarchyEntityIDs()
	{
		return mShowHierarchyEntityIDs;
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

		std::filesystem::path filepath = registry.GetAssetManager().mAssetDirectory.string() + "/" + filename.value();

		if (mAnimatorData)
		{
			if (!mAnimatorData->empty())
				mAnimatorData->reset();

			if (!mAnimatorData->Load(filepath))
			{
				mAnimatorData.reset();
				SLICE_LOG_ERROR("Animator Data not loaded.");
			}
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
