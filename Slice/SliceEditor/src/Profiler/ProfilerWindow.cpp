#include <pch.h>
#include "ProfilerWindow.h"

namespace SliceEditor
{
	ProfilerWindow::ProfilerWindow(ProfilerManager& man) : mManager(man)
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

	void ProfilerWindow::DrawLoggerTab()
	{
		

		ImGui::Checkbox("Auto-Scroll", &mManager.autoScroll);

		ImGui::BeginChild("##Logger", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar);
		for (int i = 0; i < Logger::savedLogs.size(); i++)
		{
			std::stringstream ss;

			ss << " [" << Logger::LogLevelToString(Logger::savedLogs[i].first) << "] ";

			ImGui::TextColored(mManager.LogLevelToImVec4(Logger::savedLogs[i].first), ss.str().c_str());

			ImGui::SameLine();

			ImGui::Text("%s", Logger::savedLogs[i].second.c_str());
		}

		if (mManager.autoScroll)
		{
			ImGui::SetScrollHereY(1.0);
		}

		if (ImGui::GetScrollY() < ImGui::GetScrollMaxY())
		{
			mManager.autoScroll = false;
		}
		else
		{
			mManager.autoScroll = true;
		}
		ImGui::EndChild();
	}

	void ProfilerWindow::DrawPerformanceTab()
	{
		
		for (auto&[system, time] : SliceEngine::Core::GetInstance()->GetFramerateManager()->GetSysDurations())
		{
			ImGui::Text("%s: ", system.c_str());
			ImGui::SameLine();
			ImGui::Text("Duration: %.4f",time);
		}

		ImGui::Text("Total Frame Time: %.4f", SliceEngine::Core::GetInstance()->GetFramerateManager()->GetFrameTime());
	}
}