/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ProfilerWindow.cpp

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Defines the ProfilerWindow class, which is the window that draws the profiler data of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


#include <pch.h>
#include "ProfilerWindow.h"
#include "ProfilerManager.h"
#include "Core/Registry.h"
#include "Selection/SelectionManager.h"
#include <Systems/FramerateManager.h>

namespace SliceEditor
{
	ProfilerWindow::ProfilerWindow(ProfilerManager& man, Registry& reg) : EditorWindow(reg), mManager(man)
	{
	}

	void ProfilerWindow::Init()
	{
	}

	void ProfilerWindow::Draw()
	{

		ImGui::Begin("Profiler");

		ImGui::BeginTabBar("##Profiler");

		if(ImGui::BeginTabItem("Logger"))
		{
			DrawLoggerTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Profiler"))
		{
			DrawPerformanceTab();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("SceneGraphView"))
		{
			DrawSceneGraphTab();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();


		ImGui::End();
	}

	void ProfilerWindow::DrawLoggerTab()
	{
		ImGui::Checkbox("Auto-Scroll", &mManager.mAutoScroll);

		ImGui::BeginChild("##Logger", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar);
		for (int i = 0; i < Logger::engineLogs.size(); i++)
		{
			std::stringstream ss;

			ss << " [" << Logger::LogLevelToString(Logger::engineLogs[i].first) << "] ";

			ImGui::TextColored(mManager.LogLevelToImVec4(Logger::engineLogs[i].first), ss.str().c_str());

			ImGui::SameLine();

			ImGui::Text("%s", Logger::engineLogs[i].second.c_str());
		}

		if (mManager.mAutoScroll)
		{
			ImGui::SetScrollHereY(1.0);
		}

		if (ImGui::GetScrollY() < ImGui::GetScrollMaxY())
		{
			mManager.mAutoScroll = false;
		}
		else
		{
			mManager.mAutoScroll = true;
		}
		ImGui::EndChild();
	}

	void ProfilerWindow::DrawPerformanceTab()
	{
		if(!mManager.mClearStatistics)
		{
			mManager.UpdateDebugStatistics();
		}
		else
		{
			mManager.ClearDebugStatistics();
		}

		ImGui::Text("FPS: %f", mManager.mCurrFPS);

		DrawSystemTimeline();

		DrawSystemBreakdown();

	}

	void ProfilerWindow::DrawSceneGraphTab()
	{
		ImGui::Text("Scene Graph View");
		ImGui::BeginChild("SceneGraph", ImVec2(0, 0), true);
		{
			DrawSceneGraphComponent(SliceEngine::FactoryInstance.GetRootEntity());

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 5.0f);

			auto& selectedEntities = mManager.GetRegistry().GetManager<SelectionManager>("Selection")->GetSelectedNodes();
			for(auto& entity: selectedEntities)
			{
				if(auto entityNode = static_cast<EntityNode*>(entity))
				{
					DrawSceneGraphComponent(entityNode->entity);
					ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 5.0f);
				}
			}
			ImGui::EndChild();
		}
	}

	void ProfilerWindow::DrawSystemTimeline()
	{
		//const auto& systemPercentages = SliceEngine::Core::GetInstance()->GetFramerateManager()->GetSystemPercentages();

		ImGui::Text("System Timeline");
		ImGui::BeginChild("Timeline", ImVec2(0, 30), ImGuiChildFlags_AutoResizeY);
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
			ImVec2 canvas_size = ImGui::GetContentRegionAvail();
			float xPos = canvas_pos.x;
			const float height = 20.0f;

			// Draw timeline background
			draw_list->AddRectFilled(
				canvas_pos,
				ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + height),
				IM_COL32(30, 30, 30, 255)
			);


			// Draw each system's time slice
			for (const auto& pair : mManager.mDebugStats)
			{
				ImU32 sysColor = mManager.GetSystemColor(pair.first);

				draw_list->AddRectFilled(
					ImVec2(xPos, canvas_pos.y),
					ImVec2(xPos + mManager.mDebugStats[pair.first].width, canvas_pos.y + height),
					sysColor
				);

				// System label if there's enough space
				float luminance = mManager.LuminanceCalculation(sysColor);
				ImU32 textColor = IM_COL32(255, 255, 255, 255);

				if (luminance > 0.5f)
				{
					textColor = IM_COL32(0, 0, 0, 255);
				}

				if (mManager.mDebugStats[pair.first].width > 50.0f) {
					draw_list->AddText(
						ImVec2(xPos + 2, canvas_pos.y + 2),
						textColor,
						pair.first.c_str()
					);
				}

				// Tooltip on hover
				if (ImGui::IsMouseHoveringRect(
					ImVec2(xPos, canvas_pos.y),
					ImVec2(xPos + mManager.mDebugStats[pair.first].width, canvas_pos.y + height)))
				{
					ImGui::BeginTooltip();
					ImGui::Text("%s\nTime: %.3f ms\nPercentage: %.1f%%",
						pair.first.c_str(),
						mManager.mDebugStats[pair.first].timeTaken,
						mManager.mDebugStats[pair.first].loadPercentage);
					ImGui::EndTooltip();
				}

				xPos += mManager.mDebugStats[pair.first].width;
			}
		}
		ImGui::EndChild();
	}

	void ProfilerWindow::DrawSystemBreakdown()
	{
		auto frm = SliceEngine::Core::GetInstance()->GetFramerateManager();
		//const auto& systemPercentages = frm->GetSystemPercentages();
		ImGui::Text("System Statistics");
		ImGui::BeginChild("System Stats", ImVec2(0, 0), true);
		{
			ImGui::Text("Total Frame Time: %.4f", mManager.mTotalFrameTime);
			ImGui::Text("%.2f %% of Frame Time Untracked", mManager.mUntrackedFrameTimePercentage);
			ImGui::Text("Untracked Frame Time: %.4f", mManager.mUntrackedFrameTime);
			ImGui::Text("Delta Time: %.4f", mManager.mDeltaTime);
			//ImGui::Text("Fixed Delta Time: %.4f", frm->getFixedDeltaTime());
			for (const auto& pair : mManager.mDebugStats) {
				// Create a bordered box for each system
				ImGui::BeginChild(pair.first.c_str(), ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);
				{
					// System Name with colored indicator

					ImU32 color = mManager.GetSystemColor(pair.first);
					ImGui::ColorButton("##color", ImColor(color), ImGuiColorEditFlags_NoTooltip, ImVec2(16, 16));
					ImGui::SameLine();
					ImGui::Text("%s", pair.first.c_str());

					// Performance metrics in two columns
					ImGui::Columns(2, nullptr, false);

					ImGui::Text("Time:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%.3f ms",
						mManager.mDebugStats[pair.first].timeTaken);

					ImGui::NextColumn();

					ImGui::Text("Load:");
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "%.1f%%",
						mManager.mDebugStats[pair.first].loadPercentage);

					ImGui::Columns(1);

					// Progress bar showing system load
					ImGui::ProgressBar((float)mManager.mDebugStats[pair.first].loadPercentage / 100.0f, ImVec2(-1, 4), "");
				}
				ImGui::EndChild();
				ImGui::Spacing();
			}
		}
		ImGui::EndChild();

	}

	//Move this function to Inspector in future/Keep it here but restructure.
	void ProfilerWindow::DrawSceneGraphComponent(entt::entity entity)
	{
		if (entity == SliceEngine::FactoryInstance.GetRootEntity())
		{
			auto& registry = SliceEngine::Core::GetInstance()->GetRegistry();
			auto& sceneGraph = registry.get<SliceEngine::SceneGraph>(entity);

			if (sceneGraph.neighbours[SliceEngine::SceneGraph::UP] != entt::null)
			{
				if (sceneGraph.neighbours[SliceEngine::SceneGraph::UP] == SliceEngine::FactoryInstance.GetRootEntity())
				{
					ImGui::Text("Parent: Root Entity");
				}
				else
				{
					auto parentGO = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::UP]);
					ImGui::Text("Parent: %s", parentGO.GetName().c_str());
				}
			}
			else
			{
				ImGui::Text("Parent: --");
			}

			if (sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT] != entt::null)
			{
				auto leftSibling = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);
				ImGui::Text("Left: %s", leftSibling.GetName().c_str());
			}
			else
			{
				ImGui::Text("Left: --");
			}

			if (sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT] != entt::null)
			{
				auto rightSibling = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT]);
				ImGui::Text("Right: %s", rightSibling.GetName().c_str());
			}
			else
			{
				ImGui::Text("Right: --");
			}
			//Scene Graph Down is first child
			if (sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN] != entt::null)
			{
				auto firstChild = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN]);
				ImGui::Text("First Child: %s", firstChild.GetName().c_str());
			}
			else
			{
				ImGui::Text("First Child: --");
			}
		}
		auto go = SliceEngine::FactoryInstance.GetGOByEntity(entity);
		if (go.HasComponent<SliceEngine::SceneGraph>())
		{
			auto& sceneGraph = go.GetComponent<SliceEngine::SceneGraph>();
			ImGui::Text("Entity: %s", go.GetName().c_str());
			ImGui::Text("Entity ID: %d", (uint32_t)entity);
			if (sceneGraph.neighbours[SliceEngine::SceneGraph::UP] != entt::null)
			{
				if(sceneGraph.neighbours[SliceEngine::SceneGraph::UP] == SliceEngine::FactoryInstance.GetRootEntity())
				{
					ImGui::Text("Parent: Root Entity");
				}
				else
				{
					auto parentGO = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::UP]);
					ImGui::Text("Parent: %s", parentGO.GetName().c_str());
					ImGui::SameLine();
					ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::UP]);
				}
			}
			else
			{
				ImGui::Text("Parent: --");
			}

			if(sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT] != entt::null)
			{
				auto leftSibling = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);
				ImGui::Text("Left: %s", leftSibling.GetName().c_str());
				ImGui::SameLine();
				ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);
			}
			else
			{
				ImGui::Text("Left: --");
			}

			if(sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT] != entt::null)
			{
				auto rightSibling = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT]);
				ImGui::Text("Right: %s", rightSibling.GetName().c_str());
				ImGui::SameLine();
				ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT]);
			}
			else
			{
				ImGui::Text("Right: --");
			}
			//Scene Graph Down is first child
			if(sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN] != entt::null)
			{
				auto firstChild = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN]);
				ImGui::Text("First Child: %s", firstChild.GetName().c_str());
				ImGui::SameLine();
				ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN]);
			}
			else
			{
				ImGui::Text("First Child: --");
			}
		}
	}
}