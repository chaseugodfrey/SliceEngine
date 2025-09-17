#include <pch.h>
#include "InspectorManager.h"
#include "InspectorWindow.h"

namespace SliceEditor
{
	InspectorWindow::InspectorWindow(InspectorManager& manager) : mManager(manager)
	{

	}

	void InspectorWindow::Draw()
	{
		ImGui::Begin("Inspector");

		ImGui::End();
	}
}