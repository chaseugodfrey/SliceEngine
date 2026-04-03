/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SelectionSystem.cpp

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the SelectionSystem class, which manages the selection of entities in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "SelectionManager.h"
#include "Core/Registry.h"
#include "Session/SessionManager.h"
#include "History/HistoryManager.h"

namespace SliceEditor
{
	//void SelectionManager::UpdateManagers()
	//{
	//	/*for (size_t i = 0; i < mListeners.size(); i++)
	//	{
	//		mListeners[i]->OnUpdateSelected(mSelectedEntities);
	//	}*/
	//}

	void SelectionManager::Init()
	{
		mSelectedNodes.clear();
		EventManager::GetInstance()->Subscribe<ClearSelectionEvent, &SelectionManager::ClearSelectionEventHandler>(this);
		EventManager::GetInstance()->Subscribe<DeleteSelectedEntities, &SelectionManager::DeleteSelectedObjects>(this);
		EventManager::GetInstance()->Subscribe<CloneSelectedEntities, &SelectionManager::CloneSelectedObjects>(this);

	}

	void SelectionManager::Update()
	{
		if (ImGui::IsKeyPressed(ImGuiKey_O))
		{
			SLICE_LOG("mSelectedNode Size: " + std::to_string(mSelectedNodes.size()));
			SLICE_LOG("mSelectedOrder Size: " + std::to_string(mSelectionOrder.size()));
		}
	}

	void SelectionManager::RegisterListener(ISelectionListener* listener)
	{
		mListeners.push_back(listener);
	}

	void SelectionManager::SelectSingle(SelectionNode* node, bool suppressHistory)
	{
		//std::unordered_set<entt::entity> oldSelection = mSelectedEntities;
		std::unordered_set<SelectionNode*> oldSelection = mSelectedNodes;
		if (mSelectedNodes.size() > 0)
		{
			if (node == GetLastSelectedNode())
			{
				return;
			}
		}

		ClearSelection(true);
		//mSelectedEntities.insert(entity);
		mSelectedNodes.insert(node);
		mSelectionOrder.push_back(node);
		node->isSelected = true;

		if (node->type == SelectionType::ENTITY)
		{
			EntityNode* entNode = static_cast<EntityNode*>(node);
			SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity).AddComponent<SliceEngine::SelectedEntity>();
		}
		else if (node->type == SelectionType::PREFAB)
		{
			//Get the PrefabGUID for the event of changing Hierarchy and Inspector to Prefab Inspecting
			DirectoryNode* dirNode = static_cast<DirectoryNode*>(node);
			auto& assetManager = registry.GetAssetManager();
			//auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
			SliceEngine::GUID prefabGUID;
			std::string fileName = std::filesystem::relative(dirNode->fullPath.lexically_normal(), registry.GetAssetManager().mAssetDirectory.lexically_normal()).generic_string();
			//Search for the GUID in the map:
			if (assetManager.mFilenameToGUID.find(fileName) != assetManager.mFilenameToGUID.end())
			{
				prefabGUID = assetManager.mFilenameToGUID[fileName];
			}
			else
			{
				SLICE_LOG_CRITICAL("Prefab Inspected not in AssetManager!");
				return;
			}

			PrefabInspectedEvent event(prefabGUID, true);
			EventManager::GetInstance()->Publish<PrefabInspectedEvent>(event);
		}
		else if (node->type == SelectionType::SHADERGRAPH)
		{
			DirectoryNode* dirNode = static_cast<DirectoryNode*>(node);
			auto& assetManager = registry.GetAssetManager();
			auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
			SliceEngine::GUID shaderGUID;
			std::string fileName = std::filesystem::relative(dirNode->fullPath.lexically_normal(), registry.GetAssetManager().mAssetDirectory.lexically_normal()).generic_string();
			//Search for the GUID in the map:
			if (assetManager.mFilenameToGUID.find(fileName) != assetManager.mFilenameToGUID.end())
			{
				shaderGUID = assetManager.mFilenameToGUID[fileName];
			}
			else
			{
				SLICE_LOG_CRITICAL("Custom Shader Inspected not in AssetManager!");
				return;
			}
			ShaderGraphInspectedEvent event{shaderGUID};
			EventManager::GetInstance()->Publish<ShaderGraphInspectedEvent>(event);
		}

		mSelectionType = node->type;

		/*if (!suppressHistory)
		{
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, oldSelection, mSelectedNodes));
		}*/
	}

	void SelectionManager::SelectSingle(entt::entity entity, bool suppressHistory)
	{
		auto session = registry.GetManager<SessionManager>("Session");
		if (session->IsPrefabInspected())
		{
			auto& node = session->GetPrefabNodes().at(entity);
			SelectSingle(node.get(), suppressHistory);
		}
		else
		{
			auto& node = session->GetEntityNodes().at(entity);
			SelectSingle(node.get(), suppressHistory);
		}
	}

	void SelectionManager::SelectSingleAdd(SelectionNode* node, bool suppressHistory)
	{
		std::unordered_set<SelectionNode*> oldSelection = mSelectedNodes;

		auto it = mSelectedNodes.find(node);
		if (it == std::end(mSelectedNodes))
		{
			node->isSelected = true;
			mSelectedNodes.insert(node);
			mSelectionOrder.push_back(node);
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode* entNode = static_cast<EntityNode*>(node);
				SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity).AddComponent<SliceEngine::SelectedEntity>();
			}
		}
		else
		{
			node->isSelected = false;
			mSelectedNodes.erase(it);
			auto selectIt = std::find(mSelectionOrder.begin(), mSelectionOrder.end(), node);

			if (selectIt != mSelectionOrder.end())
			{
				mSelectionOrder.erase(selectIt);
			}

			if (node->type == SelectionType::ENTITY)
			{
				EntityNode* entNode = static_cast<EntityNode*>(node);
				auto go = SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity);
				if (go.HasComponent<SliceEngine::SelectedEntity>())
					go.RemoveComponent<SliceEngine::SelectedEntity>();
			}
		}

		/*if (!suppressHistory)
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, oldSelection, mSelectedNodes));*/

		registry.GetManager<HistoryManager>("History")->ClearFromCheckpoint();
		registry.GetManager<HistoryManager>("History")->CreateCheckpoint();
	}

	void SelectionManager::SelectSingleAdd(entt::entity entity, bool suppressHistory)
	{
		auto session = registry.GetManager<SessionManager>("Session");
		if(entity == entt::null)
		{
			return;
		}
		auto& node = session->GetEntityNodes().at(entity);
		SelectSingleAdd(node.get(), suppressHistory);

	}

	void SelectionManager::AddBetweenEntities(SelectionNode* otherNode)
	{
		registry.GetManager<HistoryManager>("History")->ClearFromCheckpoint();
		registry.GetManager<HistoryManager>("History")->CreateCheckpoint();
		auto& mainNode = mSelectionOrder.back();
		if (mainNode->type == SelectionType::ENTITY && otherNode->type == SelectionType::ENTITY)
		{
			auto& mainEntity = static_cast<EntityNode*>(mainNode)->entity;
			auto& otherEntity = static_cast<EntityNode*>(otherNode)->entity;
			if (mainEntity == otherEntity)
				return;
			//Check that both have sceneGraph
			if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::SceneGraph>(mainEntity) && SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::SceneGraph>(otherEntity))
			{
				auto& mainSceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(mainEntity);
				auto& otherSceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(otherEntity);

				//DFS to the end of the sceneGraph
				bool onRight = SceneGraphRightTraversal(mainEntity, otherEntity);
				if(onRight)
				{
					SLICE_LOG("Right/Down Shift Selection");
					ShiftAddEntities(otherEntity, SliceEngine::SceneGraph::RIGHT);
				}
				else
				{
					SLICE_LOG("Left/Down Shift Selection");
					ShiftAddEntities(otherEntity, SliceEngine::SceneGraph::LEFT);
				}
				//Making sure the last clicked node was the last to be "selected" IF it should be selected
				if(mSelectedNodes.find(otherNode) != mSelectedNodes.end())
				{
					mSelectionOrder.erase
					(
						std::remove(mSelectionOrder.begin(), mSelectionOrder.end(), otherNode), mSelectionOrder.end()
					);
					mSelectionOrder.push_back(otherNode);
				}
			}
		}
	}

	void SelectionManager::ShiftAddEntities(Entity targetEntity, SliceEngine::SceneGraph::Direction direction)
	{
		//auto& entityNodes = registry.GetManager<SessionManager>("Session")->GetEntityNodes();
		//Get the starting entity:
		auto& mainNode = mSelectionOrder.back();
		auto currentEntity = static_cast<EntityNode*>(mainNode)->entity; //Thjis should be true becasue i checked in the outer function
		//Double check for sceneGraph:
		if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::SceneGraph>(currentEntity))
		{
			SLICE_LOG("No Scene Graph in the last selected object");
			return;
		}
		auto& currentSceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(currentEntity);
		Entity startingEntity = currentSceneGraph.neighbours[direction];
		Entity childEntity = currentSceneGraph.neighbours[SliceEngine::SceneGraph::DOWN];
		if (TraverseAndSelect(childEntity, targetEntity, SliceEngine::SceneGraph::RIGHT)) // The down of the current entity
		{
			return;
		}
		TraverseAndSelect(startingEntity, targetEntity, direction);
	}

	bool SelectionManager::TraverseAndSelect(Entity currentEntity, Entity targetEntity, SliceEngine::SceneGraph::Direction direction)
	{
		auto& entityNodes = registry.GetManager<SessionManager>("Session")->GetEntityNodes();
		//Get the child's EntityNode to see if its even open
		if (entityNodes.find(currentEntity) == entityNodes.end())
		{
			return false;
		}
		auto& currentSceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(currentEntity);
		EntityNode* currentNode = entityNodes[currentEntity].get();

		ProcessNodeSelection(currentNode); //Process Selection
		//Go in if node is open
		if (currentNode->nodeOpen)
		{
			Entity childEntity = currentSceneGraph.neighbours[SliceEngine::SceneGraph::DOWN];
			if (childEntity != entt::null) {
				if (TraverseAndSelect(childEntity, targetEntity, SliceEngine::SceneGraph::RIGHT))
				{
					return true;
				}
			}
		}
		if (targetEntity == currentEntity)
		{
			return true;
		}
		//Continue iterating to the direction:
		Entity nextSibling = currentSceneGraph.neighbours[direction];
		if (nextSibling != entt::null) 
		{
			if (TraverseAndSelect(nextSibling, targetEntity, direction))
			{
				return true;
			}
		}
		return false;
	}

	void SelectionManager::ProcessNodeSelection(EntityNode* currentNode)
	{
		if (!currentNode->isSelected) //Hasnt been selected
		{
			currentNode->isSelected = true;
			mSelectedNodes.insert(currentNode);
			mSelectionOrder.push_back(currentNode);
			SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(currentNode->entity).AddComponent<SliceEngine::SelectedEntity>();
		}
		else
		{
			currentNode->isSelected = false;
			mSelectedNodes.erase(currentNode);
			mSelectionOrder.erase
			(
				std::remove(mSelectionOrder.begin(), mSelectionOrder.end(), currentNode), mSelectionOrder.end()
			);
			auto go = SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(currentNode->entity);
			if (go.HasComponent<SliceEngine::SelectedEntity>())
				go.RemoveComponent<SliceEngine::SelectedEntity>();
		}
	}

	bool SelectionManager::SceneGraphRightTraversal(Entity currentEntity, Entity targetEntity)
	{
		if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::SceneGraph>(currentEntity))
		{
			auto& currentSceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(currentEntity);
			auto childEntity = currentSceneGraph.neighbours[SliceEngine::SceneGraph::DOWN];

			
			//DOWN CHECKS
			while (childEntity != entt::null)
			{
				auto& child_scene_graph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(childEntity);
				
				if (childEntity == targetEntity)
				{
					return true;
				}

				if (SceneGraphRightTraversal(childEntity, targetEntity))
				{
					return true;
				}

				childEntity = child_scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
			}

			auto rightEntity = currentSceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT];
			//MOVING TO THE RIGHT
			while (rightEntity != entt::null)
			{
				auto& rightSceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(rightEntity);
				if (rightEntity == targetEntity)
				{
					return true;
				}
				if (SceneGraphRightTraversal(rightEntity, targetEntity))
				{
					return true;
				}
				rightEntity = rightSceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT];
			}

		}
		return false;
	}

	void SelectionManager::SelectMultiple(std::unordered_set<SelectionNode*> selectedNodes, bool suppressHistory)
	{
		/*if (!suppressHistory)
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, mSelectedNodes, selectedNodes));*/

		ClearSelection(suppressHistory);

		mSelectedNodes = selectedNodes;

		for (auto& node : mSelectedNodes)
		{
			node->isSelected = true;
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode* entNode = static_cast<EntityNode*>(node);
				SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity).AddComponent<SliceEngine::SelectedEntity>();
			}
		}
	}

	/*void SelectionManager::UpdateDeslected(std::unordered_set<entt::entity>& entities, bool suppressHistory)
	{

	}*/

	void SelectionManager::ClearSelectionEventHandler(ClearSelectionEvent& event)
	{
		ClearSelection(event.suppressHistory);
	}

	void SelectionManager::ClearSelection(bool suppressHistory)
	{
		registry.GetManager<HistoryManager>("History")->ClearFromCheckpoint();
		if (mSelectedNodes.empty())
			return;

		/*if (!suppressHistory)
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, mSelectedNodes, std::unordered_set<SelectionNode*>{}));*/

		for (auto& node : mSelectedNodes)
		{
			node->isSelected = false;
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode* entNode = static_cast<EntityNode*>(node);
				auto go = SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity);
				if (go.HasComponent<SliceEngine::SelectedEntity>())
					go.RemoveComponent<SliceEngine::SelectedEntity>();
			}
		}

		mSelectionType = SelectionType::NONE;
		mSelectedNodes.clear();
		mSelectionOrder.clear();
	}

	void SelectionManager::DeleteSelectedObjects()
	{
		std::vector<entt::entity> deleteList;
		for (auto* node : mSelectedNodes)
		{
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode& entityNode = *static_cast<EntityNode*>(node);
				deleteList.push_back(entityNode.entity);
			}
		}

		for(auto entity : deleteList)
		{
			EditorUtilities::GameObject_Destroy(entity);
		}
	}

	void SelectionManager::CloneSelectedObjects()
	{
		for (auto* node : mSelectedNodes)
		{
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode& entityNode = *static_cast<EntityNode*>(node);
				EditorUtilities::GameObject_Clone(entityNode.entity);
			}
		}
	}

	/*std::unordered_set<entt::entity>& SelectionManager::GetSelectedEntities()
	{
		return mSelectedEntities;
	}*/
	std::unordered_set<SelectionNode*>& SelectionManager::GetSelectedNodes()
	{
		return mSelectedNodes;
	}
	SelectionNode* SelectionManager::GetLastSelectedNode()
	{
		return mSelectionOrder.back();
	}
}