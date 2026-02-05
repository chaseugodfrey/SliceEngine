#include <pch.h>
#include "CustomShader/CustomShaderWindow.h"
#include "Core/Registry.h"
#include "Selection/SelectionManager.h"
#include "Session/SessionManager.h"

namespace SliceEditor
{
	void CustomShaderWindow::Init()
	{
		mSelectionManager = mRegistry.GetManager<SelectionManager>("Selection");
		mSessionManager = mRegistry.GetManager<SessionManager>("Session");
	}

	void CustomShaderWindow::Draw()
	{
		ImGui::Begin("CustomShader");

		

		if(ImGui::Button("Test Button"))
		{
			SliceEngine::Handle<SliceEngine::SliceEngineTypes::Texture> handle = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Texture>("Textures/Gideon.png");
			auto texture = handle.get();
			ImGui::Image(static_cast<ImU64>(texture->texture_id), ImGui::GetWindowSize());
		}

		ImGui::End();
	}
}