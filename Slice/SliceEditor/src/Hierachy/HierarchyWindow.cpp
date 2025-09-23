#include <pch.h>
#include "HierarchyWindow.h"
#include "HierarchyManager.h"
//#include "../../SliceEngine/src/Core/Core.h"

namespace SliceEditor
{
	constexpr ImGuiTreeNodeFlags parentFlags = ImGuiTreeNodeFlags_OpenOnArrow;
	constexpr ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_Leaf;

	std::unordered_set<TestNode*> set;

	HierarchyWindow::HierarchyWindow(HierarchyManager& manager) : mManager(manager)
	{
		set = std::unordered_set<TestNode*>();
	}

	void HierarchyWindow::DrawNode(TestNode& node)
	{
		bool hasChildren = node.children.size() > 0;
		ImGuiTreeNodeFlags flags = hasChildren ? parentFlags : childFlags;
		flags |= ImGuiTreeNodeFlags_SpanFullWidth;

		if (node.isSelected)
			flags |= ImGuiTreeNodeFlags_Selected;

		bool isOpen = ImGui::TreeNodeEx(node.name.c_str(), flags);

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
					node.isSelected = true;
					set.insert(&node);
				}
			}

			else
			{
				std::for_each(set.begin(), set.end(), [](auto* item) {item->isSelected = false; });
				set.clear();
				node.isSelected = true;
				set.insert(&node);
			}
		}

		if (isOpen)
		{
			for (size_t i = 0; i < node.children.size(); i++)
			{
				DrawNode(node.children[i]);
			}

			ImGui::TreePop();
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("entity_pop_up");
		}

		//if (ImGui::TreeNodeEx(node.name.c_str(), flags))
		//{
		//	for (size_t i = 0; i < node.children.size(); i++)
		//	{
		//		DrawNode(node.children[i]);
		//	}

		//	ImGui::TreePop();
		//}

		EntityContextPopUp(node);
	}

	void HierarchyWindow::DrawSceneNode(TestNode& node)
	{
		for (size_t i = 0; i < node.children.size(); i++)
		{
			DrawNode(node.children[i]);
		}
	}

	void HierarchyWindow::DrawNodeGraph()
	{
		ImGui::BeginGroup();

		auto& rootNodes = mManager.GetNodes();

		for (size_t i = 0; i < rootNodes.size(); i++)
		{
			DrawSceneNode(rootNodes[i]);
		}

		ImGui::EndGroup();
	}

	void HierarchyWindow::EntityContextPopUp(TestNode& node)
	{
		if (ImGui::BeginPopupContextItem("entity_pop_up"))
		{
			if (ImGui::Selectable("Add Component"))
			{

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

		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("right click");
		}

		if (ImGui::BeginPopupContextItem("right click"))
		{
			if (ImGui::MenuItem("Add GameObject"))
			{
				mManager.AddGameObject();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}
}