/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        GameViewWindow.cpp

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the GameViewWindow class, which is responsible for rendering the game view window in the editor, using the camera entity.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "GameViewWindow.h"
#include "GameViewManager.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"
#include "../../SliceEngine/src/Graphics/CameraSystem.h"


namespace SliceEditor
{
	GameViewWindow::GameViewWindow(GameViewManager& manager) : mManager(manager)
	{

	}

	void GameViewWindow::Draw()
	{
		ImGui::Begin("Game");

		auto size = ImGui::GetContentRegionAvail();
		ImVec2 pos = ImGui::GetCursorScreenPos();

		auto core = SliceEngine::Core::GetInstance();
		auto view = core->GetRegistry().view<SliceEngine::Camera>();


		// todo : push this to gameview manager
		std::vector<SliceEngine::GameObject> camObjs{};

		for (auto& cam_entt : view)
		{
			auto go = core->mFactory.GetGOByEntity(cam_entt);
			if (auto scene_graph_comp = core->GetRegistry().try_get<SliceEngine::SceneGraph>(cam_entt))
			{
				camObjs.push_back(core->mFactory.GetGOByEntity(cam_entt));
			}
		}

		if (camObjs.size() > 0)
		{
			auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(camObjs[0].GetEntity());

			ImTextureID tex = reinterpret_cast<ImTextureID>(static_cast<intptr_t>(cam.textureID));

			ImGui::GetWindowDrawList()->AddImage(
				tex,
				ImVec2(pos.x, pos.y),
				ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + ImGui::GetContentRegionAvail().y),
				ImVec2(0, 1),
				ImVec2(1, 0)
			);
		}

		else
		{
			std::string msg = "No Cameras detected.";
			auto middle_pos = ImGui::GetWindowPos() + ImGui::GetWindowSize() / 2 - ImGui::CalcTextSize(msg.c_str()) / 2;
			ImGui::SetCursorScreenPos(middle_pos);
			ImGui::Text(msg.c_str());
		}

		ImGui::End();
	}
}
