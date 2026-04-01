/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        HierarchyWindow.cpp

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the HierarchyWindow class, which is responsible for rendering the hierarchy window in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "HierarchyWindow.h"
#include "Core/Registry.h"
#include "Session/SessionManager.h"
#include "History/HistoryManager.h"
#include "Selection/SelectionManager.h"
#include <Systems/SceneSystem.h>

#define IMGUI_DEFINE_MATH_OPERATORS

namespace SliceEditor
{
	constexpr ImGuiTreeNodeFlags parentFlags = ImGuiTreeNodeFlags_OpenOnArrow;
	constexpr ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_Leaf;

	void HierarchyWindow::DrawNode(SelectionManager& mSelection, SessionManager& mSession, entt::entity entity, SliceEngine::SceneGraph& scene_graph, bool isPrefab)
	{
		static bool pendingSelect = false;
		static EntityNode* pendingNode = nullptr;
		bool hasChildren = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN] != entt::null;

		ImGuiTreeNodeFlags flags = hasChildren ? parentFlags : childFlags;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth;
		EntityNode* node = nullptr;

		if (isPrefab)
		{
			auto& map = mSession.GetPrefabNodes();
			if (map.find(entity) == map.end())
			{
				SLICE_LOG_DEBUG("Entity in Scene Graph, isPrefab in the EntityNode but not in prefabMap");
				return;
			}
			node = map[entity].get();
		}
		else
		{
			auto& map = mSession.GetEntityNodes();
			if (map.find(entity) == map.end())
			{
				return;
			}
			node = map[entity].get();
		}

		
		if (node->isSelected)
			flags |= ImGuiTreeNodeFlags_Selected;

		


		//Temporary Change
		std::string name = SliceEngine::FactoryInstance.GetGOByEntity(entity).GetName();

		if (mSession.GetHierarchyEntityIDs())
		{ 
			name = std::to_string(entt::to_integral(entity)) + std::string(" ") + SliceEngine::FactoryInstance.GetGOByEntity(entity).GetName();
		}

		ImVec2 invisButtonSize = ImVec2(ImGui::GetContentRegionAvail().x, 2);
		
		if (invisButtonSize.x <= 0)
		{
			invisButtonSize.x = 50;
		}
		ImGui::InvisibleButton(("##" + name + "_order").c_str(), invisButtonSize);

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("gameobject"))
			{
				entt::entity dropped = *static_cast<entt::entity*>(payload->Data);
				EditorUtilities::GameObject_SetSibling(dropped, entity, mRegistry.GetManager<HistoryManager>("History"));
			}

			ImGui::EndDragDropTarget();
		}

		// tree node creation
		if (node->isPrefab)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 182, 193, 255)); // custom text color for prefabs
		}

		if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::InactiveEntity>(node->entity))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
		}

		if (node->isScriptSelected)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
		}

		bool isNodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);

		//Set the corresponding element's nodeOpen to true (because its being drawn)
		//TO NOTE: does this happen before or after sessionManager update/selectionManager stuff (to check if its an issue)
		if (mSession.GetEntityNodes().find(entity) != mSession.GetEntityNodes().end())
		{
			auto& entityNodes = mSession.GetEntityNodes();
			entityNodes[entity].get()->nodeOpen = isNodeOpen;
		}

		bool itemHovered = ImGui::IsItemHovered();
		//Set Pending Select when clicked
		if (itemHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			pendingSelect = true;
			pendingNode = node;
		}

		//Disable select when dragging off the threshold
		if (pendingSelect && ImGui::IsItemActive() && ImGui::IsMouseDragPastThreshold(ImGuiMouseButton_Left))
		{
			pendingSelect = false;
		}
		

		if (node->isPrefab)
		{
			ImGui::PopStyleColor();
		}

		if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::InactiveEntity>(node->entity))
		{
			ImGui::PopStyleVar();
		}

		if (node->isScriptSelected)
		{
			ImGui::PopStyleColor();
		}
		// check inputs

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup(name.c_str());
		}

		EntityContextPopUp(entity);

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("gameobject", &entity, sizeof(entity));
			ImGui::Text(name.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("gameobject"))
			{
				entt::entity child_entity = *static_cast<entt::entity*>(payload->Data);
				EditorUtilities::GameObject_Parent(child_entity, entity, mRegistry.GetManager<HistoryManager>("History"));
			}

			ImGui::EndDragDropTarget();
		}

		//Commit to the selection only if its released on the object
		if (pendingSelect && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			pendingSelect = false;

			if (ImGui::GetIO().KeyCtrl)
			{
				mSelection.SelectSingleAdd(pendingNode);
			}

			else if (ImGui::GetIO().KeyShift)
			{
				mSelection.AddBetweenEntities(pendingNode);
			}

			else
			{
				mSelection.SelectSingle(pendingNode);
			}
		}

		if (isNodeOpen)
		{
			auto child_entity = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN];

			while (child_entity != entt::null)
			{
				auto& child_scene_graph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(child_entity);
				DrawNode(mSelection, mSession, child_entity, child_scene_graph,isPrefab);
				child_entity = child_scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
			}

			ImGui::TreePop();
		}
	}

	void HierarchyWindow::DrawSceneNode()
	{
		auto scene = SliceEngine::Core::GetInstance()->GetSceneSystem()->GetCurrentSceneName();

		if (ImGui::TreeNodeEx(scene.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Separator();

			auto& engine_reg = SliceEngine::Core::GetInstance()->GetRegistry();
			auto& scene_graph = engine_reg.get<SliceEngine::SceneGraph>(entt::entity(0));
			auto child_entity = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN];

			while (child_entity != entt::null)
			{
				//auto& child_scene_graph = engine_reg.get<SliceEngine::SceneGraph>(child_entity);
				//DrawNode(*mRegistry.GetManager<SelectionManager>("Selection"), *mRegistry.GetManager<SessionManager>("Session"), child_entity, child_scene_graph, false);
				//child_entity = child_scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
				if (engine_reg.any_of<SliceEngine::SceneGraph>(child_entity))
				{
					auto& child_scene_graph = engine_reg.get<SliceEngine::SceneGraph>(child_entity);
					DrawNode(*mRegistry.GetManager<SelectionManager>("Selection"), *mRegistry.GetManager<SessionManager>("Session"), child_entity, child_scene_graph, false);
					child_entity = child_scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
				}
				else
				{
					//std::cout << "Unable to get scene graph of: " << int(child_entity) << std::endl;
					break;
				}
			}

			ImGui::TreePop();
		}
	}

	void HierarchyWindow::DrawPrefabNode()
	{
		auto sessionManager = mRegistry.GetManager<SessionManager>("Session");
		Entity parentEntity = sessionManager->GetPrefabEntityInspected();
		auto& sceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(parentEntity);
		std::string parentName = SliceEngine::FactoryInstance.GetGOByEntity(parentEntity).GetName();
		auto treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;

		auto childEntity = sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN];

		if (childEntity == entt::null)
		{
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
		}

		//Get the parent entity node
		if (sessionManager->GetPrefabNodes().find(parentEntity) == sessionManager->GetPrefabNodes().end())
		{
			SLICE_LOG_WARNING("This should not trigger!");
			return;
		}
		
		//Selection Check for PrefabNodes
		if (sessionManager->GetPrefabNodes()[parentEntity]->isSelected)
		{
			treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
		}
		
		//ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 182, 193, 255)); // custom text color for prefabs
		if (ImGui::TreeNodeEx(parentName.c_str(), treeNodeFlags))
		{
			if (ImGui::IsItemHovered()&&ImGui::IsItemClicked())
			{
				if (!sessionManager->GetPrefabNodes()[parentEntity])
				{
					SLICE_LOG_ERROR("Parent Prefab Node not in the mPrefabNodes!");
				}
				else
				{
					mRegistry.GetManager<SelectionManager>("Selection")->SelectSingle(sessionManager->GetPrefabNodes()[parentEntity].get());
				}
			}

			while(childEntity != entt::null)
			{
				auto& childSceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(childEntity);
				DrawNode(*mRegistry.GetManager<SelectionManager>("Selection"), *mRegistry.GetManager<SessionManager>("Session"), childEntity, childSceneGraph, true);
				childEntity = childSceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT];
			}
			ImGui::TreePop();
		}
		//ImGui::PopStyleColor();
		
	}

	void HierarchyWindow::DrawNodeGraph()
	{
		ImGui::BeginGroup();
		if (mRegistry.GetManager<SessionManager>("Session")->IsPrefabInspected())
		{
			DrawPrefabNode();
		}
		else
		{
			DrawSceneNode();
		}
		ImGui::EndGroup();
	}

	void HierarchyWindow::EntityContextPopUp(entt::entity entity)
	{
		auto& scene_graph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(entity);
		bool hasParent = scene_graph.neighbours[SliceEngine::SceneGraph::UP] != SliceEngine::FactoryInstance.GetRootEntity();

		if (ImGui::BeginPopupContextItem())
		{
			if (!hasParent)
				ImGui::BeginDisabled();

			if (ImGui::Selectable("Unparent"))
			{
				EditorUtilities::GameObject_Unparent(entity);
			}

			if (!hasParent)
				ImGui::EndDisabled();

			//if (ImGui::Selectable("Add Transform Component"))
			//{
				//gCoordinate->mObjectFactory->AddComponent<Transform>(entity_id);
			//}

			if (ImGui::Selectable("Remove GameObject"))
			{
				EditorUtilities::GameObject_Destroy(entity, mRegistry.GetManager<HistoryManager>("History"));
			}

			ImGui::EndPopup();
		}
	}

	void HierarchyWindow::Init()
	{

	}

	void HierarchyWindow::Draw()
	{
		ImGui::Begin("Hierarchy");

		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 p1 = p0 + ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y + ImGui::GetContentRegionAvail().y);
		ImGuiID id = ImGui::GetCurrentWindow()->GetID("HierarchyDrop");
		ImRect rect(p0, p1);


		if (ImGui::BeginDragDropTargetCustom(rect, id))
		{
			//if (ImGui::AcceptDragDropPayload("Model"))
			//{
			//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Model"))
			//	{
			//		SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
			//		//EditorUtilities::GameObject_CreateModel(recievedPayload, entt::null, mRegistry.GetManager<HistoryManager>("History"));
			//	}
			//}

			if (ImGui::AcceptDragDropPayload("Prefab"))
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab"))
				{
					SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
					EditorUtilities::GameObject_CreatePrefab(recievedPayload, entt::null, mRegistry.GetManager<HistoryManager>("History"));
				}
			}
			ImGui::EndDragDropTarget();
		}


		DrawNodeGraph();

		ImVec2 hierarchy_empty_space_size = ImGui::GetContentRegionAvail();

		if (hierarchy_empty_space_size.x > 0 && hierarchy_empty_space_size.y > 0)
		{
			ImGui::BeginGroup();
			ImGui::InvisibleButton("##hierarchy_end", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
			ImGui::EndGroup();

			if (ImGui::IsItemClicked())
			{
				mRegistry.GetManager<SelectionManager>("Selection")->ClearSelection();
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("window_popup");
			}

			if (ImGui::BeginPopupContextItem("window_popup"))
			{
				auto sessionManager = mRegistry.GetManager<SessionManager>("Session");
				if (ImGui::BeginMenu("Create"))
				{
					if(sessionManager->IsPrefabInspected())
					{
						EditorUtilities::MenuList_CreateGameObjects(mRegistry.GetManager<HistoryManager>("History"), sessionManager->GetPrefabEntityInspected(), sessionManager->IsPrefabInspected());
					}
					else
					{
						EditorUtilities::MenuList_CreateGameObjects(mRegistry.GetManager<HistoryManager>("History"));
					}
					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

		}


		ImGui::End();
	}
}