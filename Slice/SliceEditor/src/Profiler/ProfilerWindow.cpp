#include <pch.h>
#include "ProfilerWindow.h"

namespace SliceEditor
{
	ProfilerWindow::ProfilerWindow(ProfilerManager& man) : manager(man)
	{
	}

	void ProfilerWindow::Draw()
	{
		ImGui::Begin("Profiler");

		ImGui::BeginTabBar("##Profiler");

		if(ImGui::BeginTabItem("Logger"))
		{

			ImGui::Checkbox("Auto-Scroll", &manager.autoScroll);

			ImGui::BeginChild("##Logger", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar);
			for (int i = 0; i < Logger::savedLogs.size(); i++)
			{
				std::stringstream ss;

				ss << " [" << Logger::LogLevelToString(Logger::savedLogs[i].first) << "] ";

				ImGui::TextColored(manager.LogLevelToImVec4(Logger::savedLogs[i].first), ss.str().c_str());

				ImGui::SameLine();

				ImGui::Text("%s", Logger::savedLogs[i].second.c_str());
			}

			if (manager.autoScroll)
			{
				ImGui::SetScrollHereY(1.0);
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();


		ImGui::End();

		if (ImGui::IsKeyPressed(ImGuiKey_C))
		{
			SLICE_LOG_CRITICAL("Critical Message!");
		}
		if (ImGui::IsKeyPressed(ImGuiKey_Z))
		{
			SLICE_LOG_VALUES("Values Message!");
		}
		if (ImGui::IsKeyPressed(ImGuiKey_X))
		{
			SLICE_LOG_WARNING("Warning Message!");
		}
		if (ImGui::IsKeyPressed(ImGuiKey_V))
		{
			SLICE_LOG_ERROR("Error Message!");
		}
	}
}