#include <pch.h>
#include "HierarchyWindow.h"
#include "HierarchyManager.h"

namespace SliceEditor

{
	constexpr ImGuiTreeNodeFlags parentFlags = ImGuiTreeNodeFlags_OpenOnArrow;
	constexpr ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_Leaf;

	HierarchyWindow::HierarchyWindow(HierarchyManager& manager) : mManager(manager)
	{

	}

	void HierarchyWindow::DrawNode(TestNode const& node)
	{
		bool hasChildren = node.children.size() > 0;
		ImGuiTreeNodeFlags flags = hasChildren ? parentFlags : childFlags;

		if (ImGui::TreeNodeEx(node.name.c_str(), flags))
		{
			for (size_t i = 0; i < node.children.size(); i++)
			{
				DrawNode(node.children[i]);
			}

			ImGui::TreePop();
		}
	}

	void HierarchyWindow::DrawSceneNode(TestNode const& node)
	{
		for (size_t i = 0; i < node.children.size(); i++)
		{
			DrawNode(node.children[i]);
		}
	}

	void HierarchyWindow::DrawNodeGraph()
	{
		auto& rootNodes = mManager.GetNodes();

		for (size_t i = 0; i < rootNodes.size(); i++)
		{
			DrawSceneNode(rootNodes[i]);
		}
	}


	void HierarchyWindow::Draw()
	{
		ImGui::Begin("Hierarchy");


		ImGui::BeginGroup();

		DrawNodeGraph();

		ImGui::EndGroup();

		ImGui::BeginGroup();

		ImGui::InvisibleButton("##hierarchy empty space", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));

		ImGui::EndGroup();

		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("right click");
		}

		if (ImGui::BeginPopupContextItem("right click"))
		{
			if (ImGui::MenuItem("Add GameObject"))
			{

			}

			ImGui::EndPopup();
		}
		ImGui::End();
	}
}