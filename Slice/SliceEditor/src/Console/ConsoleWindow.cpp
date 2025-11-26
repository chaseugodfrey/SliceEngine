#include <pch.h>
#include "ConsoleWindow.h"

namespace SliceEditor
{
	void ConsoleWindow::Init()
	{

	}

	void ConsoleWindow::Draw()
	{
		auto& logs = Logger::consoleLogs;

		ImGui::Begin("Console");
		
		if (ImGui::BeginTable("console_logs", 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg))
		{
			for (size_t i = 0; i < logs.size(); i++)
			{
				ImGui::TableNextColumn();

				std::string callStackMsg{};
				for (const auto& stack : logs[i].second.callStack)
					callStackMsg += stack;

				ImGui::Text(callStackMsg.c_str());
				ImGui::Text(logs[i].second.mMessage.c_str());
			}

			ImGui::EndTable();
		}

		ImGui::End();

	}
}