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
	
		ImGui::Button("Bake");
		ImGui::SameLine();
		ImGui::Button("Clear");
		ImGui::End();
	}
}