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
#include "HierarchyManager.h"
#include "../SelectionSystem/SelectionSystem.h"
//#include "../../SliceEngine/src/Core/Core.h"

namespace SliceEditor
{
	constexpr ImGuiTreeNodeFlags parentFlags = ImGuiTreeNodeFlags_OpenOnArrow;
	constexpr ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_Leaf;


	HierarchyWindow::HierarchyWindow(HierarchyManager& manager, SelectionSystem& selection) : mManager(manager), mSelection(selection)
	{

	}

	void HierarchyWindow::DrawNode(entt::entity entity, SliceEngine::SceneGraph& scene_graph)
	{
		auto core = SliceEngine::Core::GetInstance();
		bool hasChildren = scene_graph.child_count > 0;
		auto& node = mManager.GetHierarchy()[entity];

		ImGuiTreeNodeFlags flags = hasChildren ? parentFlags : childFlags;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;

		if (node.isSelected)
			flags |= ImGuiTreeNodeFlags_Selected;

		std::string name = SliceEngine::FactoryInstance.GetGOByEntity(node.entity).GetName();

		ImGui::InvisibleButton(("##" + name + "_order").c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 1));

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("gameobject"))
			{
				entt::entity dropped = *static_cast<entt::entity*>(payload->Data);
				mManager.SetNewLocation(dropped, node.entity);
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

		EntityContextPopUp(node);

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("gameobject", (void*)&node.entity, sizeof(node.entity));
			ImGui::Text(name.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("gameobject"))
			{
				entt::entity child_entity = *static_cast<entt::entity*>(payload->Data);
				mManager.ParentGameObject(child_entity, node.entity);
			}

			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked())
		{
			if (ImGui::GetIO().KeyCtrl)
			{
				if (node.isSelected)
				{
					node.isSelected = false;
					set.erase(&node);
				}

				else
				{
					mSelection.UpdateSelected(node.entity);
				}
			}

			else
			{
				mSelection.ClearSelection();
				mSelection.UpdateSelected(node.entity);
			}
		}

		if (isNodeOpen)
		{
			auto child_entity = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN];

			while (child_entity != entt::null)
			{
				auto& child_scene_graph = core->GetRegistry().get<SliceEngine::SceneGraph>(child_entity);
				DrawNode(child_entity, child_scene_graph);
				child_entity = child_scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
			}

			ImGui::TreePop();
		}
	}

	void HierarchyWindow::DrawSceneNode(TestNode& node)
	{
		if (ImGui::TreeNodeEx(node.name.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Separator();

			auto& engine_reg = SliceEngine::Core::GetInstance()->GetRegistry();
			auto& scene_graph = engine_reg.get<SliceEngine::SceneGraph>(node.entity);
			auto child_entity = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN];

			while (child_entity != entt::null)
			{
				auto& child_scene_graph = engine_reg.get<SliceEngine::SceneGraph>(child_entity);
				DrawNode(child_entity, child_scene_graph);
				child_entity = child_scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
			}


			ImGui::TreePop();
		}
	}

	void HierarchyWindow::DrawNodeGraph()
	{
		ImGui::BeginGroup();
		auto root_entity = SliceEngine::Core::GetInstance()->mFactory.GetRootEntity();
		auto& hierarchy = mManager.GetHierarchy();
		DrawSceneNode(hierarchy[root_entity]);
		ImGui::EndGroup();
	}

	void HierarchyWindow::EntityContextPopUp(TestNode& node)
	{
		auto& scene_graph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(node.entity);
		bool hasParent = scene_graph.neighbours[SliceEngine::SceneGraph::UP] != SliceEngine::FactoryInstance.GetRootEntity();

		if (ImGui::BeginPopupContextItem())
		{
			if (!hasParent)
				ImGui::BeginDisabled();

			if (ImGui::Selectable("Unparent"))
			{
				mManager.Unparent(node.entity);
			}

			if (!hasParent)
				ImGui::EndDisabled();

			//if (ImGui::Selectable("Add Transform Component"))
			//{
				//gCoordinate->mObjectFactory->AddComponent<Transform>(entity_id);
			//}

			if (ImGui::Selectable("Remove GameObject"))
			{
				mManager.RemoveGameObject(node.entity);
			}

			ImGui::EndPopup();
		}
	}

	void HierarchyWindow::Draw()
	{
		ImGui::Begin("Hierarchy");

		if (ImGui::Button("Reload Hierarchy"))
		{
			mManager.BuildHierarchy();
		}

		DrawNodeGraph();

		ImGui::BeginGroup();
		ImGui::InvisibleButton("##hierarchy_end", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
		ImGui::EndGroup();

		if (ImGui::IsItemClicked())
		{
			mSelection.ClearSelection();
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("window_popup");
		}

		if (ImGui::BeginPopupContextItem("window_popup"))
		{
			if (ImGui::Selectable("Add GameObject"))
			{
				mManager.AddGameObject();
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		mManager.CheckDirty();
	}
}