#include <pch.h>
#include "NavigationWindow.h"
#include "Selection/SelectionManager.h"

namespace SliceEditor
{
	void NavigationWindow::Init()
	{
		mCompiler.Init();
	}

	void NavigationWindow::Draw()
	{
		std::vector<glm::mat4> transformMtxs{};
		std::vector<SliceEngine::SliceEngineTypes::Model *> models{};

		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");

		ImGui::Begin("Navigation");

		if (selectionManager->mSelectionType == SelectionType::ENTITY)
		{
			auto &nodes = selectionManager->GetSelectedNodes();
			if (!nodes.empty())
			{
				auto &reg = SliceEngine::Core::GetInstance()->GetRegistry();
				for (auto &node : nodes)
				{
					EntityNode *entity_node = static_cast<EntityNode *>(node);
					auto renderer = reg.try_get<SliceEngine::Renderer>(entity_node->entity);
					if (renderer)
					{
						if (renderer->modelHandle.IsValid())
						{
							const auto &transform = reg.get<SliceEngine::Transform>(entity_node->entity);
							glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), transform.position)
								* glm::mat4_cast(transform.rotation)
								* glm::scale(glm::mat4(1.0f), transform.scale);

							transformMtxs.push_back(transformMatrix);
							models.push_back(renderer->modelHandle.get());
						}
					}
				}
			}
		}

		ImGui::BeginGroup();
		auto &config = mCompiler.GetConfig();
		auto &height = mCompiler.GetAgentHeight();
		auto &radius = mCompiler.GetAgentRadius();
		auto &maxClimb = mCompiler.GetMaxClimb();
		ImGui::SeparatorText("Rasterization");

		ImGui::Text("Cell Size");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragFloat("##nav_cell_size", &config.cs, 0.01f, 0.01f, FLT_MAX, "%.2f");

		ImGui::Text("Cell Height");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragFloat("##nav_cell_height", &config.ch, 0.01f, 0.01f, FLT_MAX, "%.2f");

		ImGui::SeparatorText("Agent");

		ImGui::Text("Radius");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragFloat("##nav_radius", &radius, 1, 0, FLT_MAX);

		ImGui::Text("Height");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragFloat("##nav_height", &height, 1, 0, FLT_MAX);

		ImGui::Text("Max Climb");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragFloat("##nav_max_climb", &maxClimb, 1, 0, FLT_MAX);

		ImGui::Text("Max Slope");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragFloat("##nav_walkable_slope_angle", &config.walkableSlopeAngle, 1.0f, 0.0f, 90.0f, "%.1f");

		ImGui::SeparatorText("Region");

		ImGui::Text("Min Region Size");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragInt("##nav_min_region", &config.minRegionArea, 1, 0, INT_MAX);

		ImGui::Text("Merged Region Size");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragInt("##nav_merged_region", &config.mergeRegionArea, 1, 0, INT_MAX);

		ImGui::SeparatorText("Polygonization");

		ImGui::Text("Max Edge Length");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragInt("##nav_edge_length", &config.maxEdgeLen, 1, 0, INT_MAX);

		ImGui::Text("Max Edge Error");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragFloat("##nav_edge_error", &config.maxSimplificationError, 0.1f, 0.0f, FLT_MAX, "%.1f");

		ImGui::Text("Verts Per Poly");
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(150.0f);
		ImGui::DragInt("##nav_verts_per_poly", &config.maxVertsPerPoly, 1, 0, INT_MAX);


		ImGui::EndGroup();

		std::string obj_handle = std::to_string(models.size()) + " objects selected.";
		ImGui::Text(obj_handle.c_str());

		if (models.empty())
			ImGui::BeginDisabled();

		if (ImGui::Button("Bake"))
		{
			std::vector<SliceEngine::NavMeshLink> links;

			auto &reg = SliceEngine::Core::GetInstance()->GetRegistry();

			// 2. Iterate using .each() to avoid iterator errors
			reg.view<SliceEngine::NavMeshLink, SliceEngine::Transform>().each([&](auto entity, auto &linkComp, auto &transform)
				{
					SliceEngine::NavMeshLink data;

					// Assuming startLink/endLink in the component are World Space positions 
					// derived from the editor handles/transforms.
					data.startLink = linkComp.startLink;
					data.endLink = linkComp.endLink;

					data.bidirectional = true;
					data.radius = 5.0f;
					std::cout << "Baking Link: " << data.startLink.x << ", " << data.startLink.y << " -> " <<  data.endLink.x << ", " << data.endLink.y  << std::endl;
					links.push_back(data);
				});
			mCompiler.BuildFromModel(models, transformMtxs, links);
		}

		ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{
			mCompiler.Clear();
		}

		if (models.empty())
			ImGui::EndDisabled();

		ImGui::End();
	}
}