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
#include "../../SliceEngine/src/Graphics/RenderManager.h"
#include "../../SliceEngine/src/Graphics/CameraSystem.h"


namespace SliceEditor
{
	void GameViewWindow::Init()
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

			ImTextureID tex = static_cast<ImTextureID>(cam.textureID);

			// win as in the scene Window
			ImVec2 winScreenTL{ pos.x, pos.y };
			ImVec2 winOffset{ 0.f,0.f };
			ImVec2 totalWinScreenDim = ImGui::GetContentRegionAvail();
			ImVec2 winScreenDim = totalWinScreenDim;
			float winScreenAR = winScreenDim.x / winScreenDim.y;
			// worldSpace as in what the camera renders (i'm going to change this val when I crop out parts of the image when streching the height)
			ImVec2 worldSpaceDim{ static_cast<float>(cam.width) , static_cast<float>(cam.height) };
			float worldSpaceAR = worldSpaceDim.x / worldSpaceDim.y;
			float worldSpaceOffsetX{ 0.f };

			ImVec2 uvCropMin{ 0.f, 1.f }; // it's like this cuz it's flipped
			ImVec2 uvCropMax{ 1.f, 0.f };

			if (winScreenAR > worldSpaceAR) // Stretch the width
			{
				winScreenDim.x = totalWinScreenDim.y * worldSpaceAR;
				winOffset.x = (totalWinScreenDim.x - winScreenDim.x) / 2.f;
			}
			else if (winScreenAR < worldSpaceAR) // Stretch the height
			{
				winScreenDim.y = totalWinScreenDim.x / worldSpaceAR;
				winOffset.y = (totalWinScreenDim.y - winScreenDim.y) / 2.f;

				// world space: y & x dim stays the same
				float wouldBeWinWidth = totalWinScreenDim.y * worldSpaceAR;
				float percentScreenShown = totalWinScreenDim.x / wouldBeWinWidth;
				uvCropMin.x += 0.5f - 0.5f * percentScreenShown;
				uvCropMax.x -= 0.5f - 0.5f * percentScreenShown;

				worldSpaceOffsetX = uvCropMin.x * worldSpaceDim.x;
				worldSpaceDim.x *= percentScreenShown;
			}

			pos += winOffset;
			float scene_x = pos.x + winScreenDim.x; // Refers to the bottom right point of the scene window in screen space
			float scene_y = pos.y + winScreenDim.y;
			// Drawing cam texture
			ImGui::GetWindowDrawList()->AddImage(
				tex,
				ImVec2(pos.x, pos.y),
				ImVec2(scene_x, scene_y),
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
