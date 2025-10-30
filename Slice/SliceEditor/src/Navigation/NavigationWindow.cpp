#include <pch.h>
#include "NavigationWindow.h"

namespace SliceEditor
{
	void NavigationWindow::Init()
	{

	}

	void NavigationWindow::Draw()
	{
		ImGui::Begin("Navigation");
	
		if (ImGui::Button("Bake"))
		{

		}

		ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{

		}

		ImGui::End();
	}
}