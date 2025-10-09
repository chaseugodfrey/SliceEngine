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
#include "Core/Registry.h"
#include "Selection/SelectionManager.h"

namespace SliceEditor
{
	constexpr ImGuiTreeNodeFlags parentFlags = ImGuiTreeNodeFlags_OpenOnArrow;
	constexpr ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_Leaf;


	HierarchyWindow::HierarchyWindow(HierarchyManager& manager) : mManager(manager)
	{

	}

	void HierarchyWindow::DrawNode(SelectionManager& mSelection, entt::entity entity, SliceEngine::SceneGraph& scene_graph)
	{
		bool hasChildren = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN] != entt::null;
		auto& node = mManager.GetHierarchy()[entity];

		ImGuiTreeNodeFlags flags = hasChildren ? parentFlags : childFlags;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;

		if (mSelection.GetSelectedEntities().find(node.entity) != mSelection.GetSelectedEntities().end())
			flags |= ImGuiTreeNodeFlags_Selected;

		std::string name = SliceEngine::FactoryInstance.GetGOByEntity(node.entity).GetName();

		ImGui::InvisibleButton(("##" + name + "_order").c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 2));

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

		EntityContextPopUp(entity);

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
					//mSelection->UpdateSelected(node.entity);
				}
			}

			else
			{
				mSelection.SelectSingle(node.entity);
			}
		}

		if (isNodeOpen)
		{
			auto child_entity = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN];

			while (child_entity != entt::null)
			{
				auto& child_scene_graph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(child_entity);
				DrawNode(mSelection, child_entity, child_scene_graph);
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
				DrawNode(*mManager.GetRegistry().GetManager<SelectionManager>("Selection"), child_entity, child_scene_graph);
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
				mManager.Unparent(entity);
			}

			if (!hasParent)
				ImGui::EndDisabled();

			//if (ImGui::Selectable("Add Transform Component"))
			//{
				//gCoordinate->mObjectFactory->AddComponent<Transform>(entity_id);
			//}

			if (ImGui::Selectable("Remove GameObject"))
			{
				mManager.RemoveGameObject(entity);
			}

			ImGui::EndPopup();
		}
	}

	void HierarchyWindow::Draw()
	{
		ImGui::Begin("Hierarchy");

		DrawNodeGraph();

		ImGui::BeginGroup();
		ImGui::InvisibleButton("##hierarchy_end", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
		ImGui::EndGroup();

		if (ImGui::IsItemClicked())
		{
			mManager.GetRegistry().GetManager<SelectionManager>("Selection")->ClearSelection();
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