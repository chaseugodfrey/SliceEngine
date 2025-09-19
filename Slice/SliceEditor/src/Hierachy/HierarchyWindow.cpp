#include <pch.h>
#include "HierarchyWindow.h"
#include "HierarchyManager.h"

namespace SliceEditor
{
	HierarchyWindow::HierarchyWindow(HierarchyManager& manager) : mManager(manager)
	{

	}

	void HierarchyWindow::Draw()
	{
		ImGui::Begin("Hierarchy");

		auto& nodes = mManager.GetNodes();

		ImGui::BeginGroup();
		for (size_t i = 0; i < nodes.size(); i++)
		{
			if (ImGui::TreeNodeEx(std::to_string(i).c_str(), ImGuiTreeNodeFlags_Leaf))
			{
				
				ImGui::TreePop();
			}
		}
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