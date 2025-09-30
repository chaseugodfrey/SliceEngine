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

	void HierarchyWindow::DrawNode(TestNode& node)
	{
		bool hasChildren = node.children.size() > 0;
		ImGuiTreeNodeFlags flags = hasChildren ? parentFlags : childFlags;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth;

		if (node.isSelected)
			flags |= ImGuiTreeNodeFlags_Selected;

		std::string name = SliceEngine::FactoryInstance.GetGOByEntity(node.entity).GetName();

		bool isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("entity_popup");
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("gameobject", (void*)&node.entity, sizeof(node.entity));
			ImGui::Text(node.name.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("gameobject"))
			{
				entt::entity child_entity = *static_cast<entt::entity*>(payload->Data);

				auto& factory = SliceEngine::Core::GetInstance()->mFactory;
				auto go = factory.GetGOByEntity(child_entity);
				factory.SetParent(child_entity, node.entity);
				mManager.ParentGameObject(child_entity, node.entity);
				//mManager.SetDirty();
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

		EntityContextPopUp(node);

		if (isOpen)
		{
			for (size_t i = 0; i < node.children.size(); i++)
			{
				auto& child_node = mManager.GetHierarchy().at(node.children[i]);
				DrawNode(child_node);
			}

			ImGui::TreePop();
		}
	}

	void HierarchyWindow::DrawSceneNode(TestNode& node)
	{
		for (size_t i = 0; i < node.children.size(); i++)
		{
			auto& child_node = mManager.GetHierarchy().at(node.children[i]);
			DrawNode(child_node);
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
		bool hasParent = node.parent->entity != SliceEngine::FactoryInstance.GetRootEntity();

		if (ImGui::BeginPopupContextItem("entity_popup"))
		{
			if (!hasParent)
				ImGui::BeginDisabled();

			if (ImGui::Selectable("Unparent"))
			{
				mManager.Unparent(node.entity, node.parent->entity);
			}

			if (!hasParent)
				ImGui::EndDisabled();

			//if (ImGui::Selectable("Add Component"))
			//{

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