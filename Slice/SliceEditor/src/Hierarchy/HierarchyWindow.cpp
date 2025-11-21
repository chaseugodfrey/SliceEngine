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

	void HierarchyWindow::DrawNode(SelectionManager& mSelection, SessionManager& mSession, entt::entity entity, SliceEngine::SceneGraph& scene_graph)
	{
		bool hasChildren = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN] != entt::null;

		ImGuiTreeNodeFlags flags = hasChildren ? parentFlags : childFlags;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth;

		auto& map = mSession.GetEntityNodes();
		if (map.find(entity) == map.end())
		{
			return;
		}
		auto node = map[entity].get();
		if (node->isSelected)
			flags |= ImGuiTreeNodeFlags_Selected;


		//Temporary Change
		std::string name = SliceEngine::FactoryInstance.GetGOByEntity(entity).GetName();

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

		bool isNodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);

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

		if (ImGui::IsItemClicked())
		{
			if (ImGui::GetIO().KeyCtrl)
			{
				mSelection.SelectSingleAdd(node);
			}

			else
			{
				mSelection.SelectSingle(node);
			}
		}

		if (isNodeOpen)
		{
			auto child_entity = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN];

			while (child_entity != entt::null)
			{
				auto& child_scene_graph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(child_entity);
				DrawNode(mSelection, mSession, child_entity, child_scene_graph);
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
				auto& child_scene_graph = engine_reg.get<SliceEngine::SceneGraph>(child_entity);
				DrawNode(*mRegistry.GetManager<SelectionManager>("Selection"), *mRegistry.GetManager<SessionManager>("Session"), child_entity, child_scene_graph);
				child_entity = child_scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
			}

			ImGui::TreePop();
		}
	}

	void HierarchyWindow::DrawNodeGraph()
	{
		ImGui::BeginGroup();
		DrawSceneNode();
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
			if (ImGui::AcceptDragDropPayload("Model"))
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Model"))
				{
					SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
					EditorUtilities::GameObject_CreateModel(recievedPayload, entt::null, mRegistry.GetManager<HistoryManager>("History"));
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
				if (ImGui::BeginMenu("Create"))
				{
					EditorUtilities::MenuList_CreateGameObjects(mRegistry.GetManager<HistoryManager>("History"), entt::null);
					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

		}


		ImGui::End();
	}
}