#include <pch.h>
#include "HistoryWindow.h"
#include "HistoryManager.h"

namespace SliceEditor
{

	void HistoryWindow::Init()
	{

	}

	void HistoryWindow::Draw()
	{
		if (!mHistoryManager)
			mHistoryManager = mRegistry.GetManager<HistoryManager>("History");

		ImGui::Begin("Undo History");

		if (ImGui::BeginTabBar("TabBar"))
		{
			if (ImGui::BeginTabItem("Undo"))
			{
				DrawUndoHistory();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Redo"))
			{
				DrawRedoHistory();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	void HistoryWindow::DrawUndoHistory()
	{
		auto& undoStack = mHistoryManager->GetUndoStack();

		if (ImGui::BeginTable("##undo_table", 1, ImGuiTableFlags_RowBg))
		{
			for (int i = (int)undoStack.size() - 1; i >= 0; i--)
			{
				ImGui::TableNextColumn();
				const auto& msg = undoStack[i]->GetCommandMessage();
				ImGui::Text(std::to_string(i).c_str());
				ImGui::TextUnformatted(msg.c_str());
			}

			ImGui::EndTable();
		}
	}

	void HistoryWindow::DrawRedoHistory()
	{

	}
}
