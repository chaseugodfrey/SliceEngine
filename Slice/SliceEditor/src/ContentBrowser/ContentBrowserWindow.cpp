#include <pch.h>
#include "ContentBrowserWindow.h"

namespace SliceEditor
{
	void ContentBrowserWindow::Draw()
	{
		ImGui::Begin("Content Browser");
		ImGui::Text("Content Browser Window");

		ImGui::End();
	}
}