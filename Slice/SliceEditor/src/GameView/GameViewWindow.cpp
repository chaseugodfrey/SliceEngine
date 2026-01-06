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
#include <Graphics/RenderManager.h>
#include <Graphics/CameraSystem.h>
#include <Input/InputSystem.h>
#include <Systems/SceneSystem.h>

namespace SliceEditor
{
	void GameViewWindow::Init()
	{
		EventManager::GetInstance()->Subscribe<OnPlayEvent, &GameViewWindow::OnPlay>(this);
	}

	void GameViewWindow::OnPlay(OnPlayEvent e)
	{
		mRequestToFocus = true;
	}

	void GameViewWindow::Draw()
	{
		ImGui::Begin("Game");

		if (mRequestToFocus)
		{
			ImGui::SetWindowFocus();
			mRequestToFocus = false;
		}

		auto& io = ImGui::GetIO();

#pragma region IO Calculation

		auto size = ImGui::GetContentRegionAvail();
		ImVec2 pos = ImGui::GetCursorScreenPos();

		ImVec2 window_pos = ImGui::GetWindowPos();
		ImVec2 window_size = ImGui::GetWindowSize(); // I actually dk what this is, I'm guessing the Whole region available for my scene to draw??

		int screen_width, screen_height;
		GLFWwindow* mWindow = SliceEngine::Core::GetInstance()->GetWindow();
		glfwGetWindowSize(mWindow, &screen_width, &screen_height); // When in editor, refers to the whole window (including the spaces occupied by inspector window etc

		ImVec2 scene_window_pos = ImGui::GetCursorScreenPos();
		ImVec2 scene_window_size = { window_size.x, window_size.y - (scene_window_pos.y - window_pos.y) };

		float mouse_relative_x = io.MousePos.x - scene_window_pos.x; // Correct, refers to the mouse position (in screen space), starting with (0,0) at the top left of the section you want
		float mouse_relative_y = io.MousePos.y - scene_window_pos.y;

#pragma endregion
	
		auto core = SliceEngine::Core::GetInstance();
		auto view = core->GetRegistry().view<SliceEngine::Camera>(entt::exclude<SliceEngine::EngineEntity>);


		// todo : push this to gameview manager
		//std::vector<SliceEngine::GameObject> camObjs{};
		//
		//for (auto& cam_entt : view)
		//{
		//	auto go = core->mFactory.GetGOByEntity(cam_entt);
		//	if (auto scene_graph_comp = core->GetRegistry().try_get<SliceEngine::SceneGraph>(cam_entt))
		//	{
		//		camObjs.push_back(core->mFactory.GetGOByEntity(cam_entt));
		//	}
		//}
		auto& possibleCam = SliceEngine::Core::GetInstance()->GetRenderManager()->GetGameCamera();
		//if (camObjs.size() > 0 && SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(camObjs[0].GetEntity()).componentEnabled)
		if(possibleCam.has_value() && SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Camera>(possibleCam.value()) != NULL)
		{
			auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(possibleCam.value());

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
				//this ui crop thing seems to be bugged, ignoring this offset stuff for now to get correct mouse pos
				//worldSpaceDim.x *= percentScreenShown;	
				
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

			/*
			* Somehow linking editor window mouse stuff to engine input system
			*/

#pragma region Mouse Click
			// mouse position should always be relative to top left, so that it is consistent with the glfwgetcursorpos
			// also removing the worldspace offset thingy for now
			//ImVec2 worldSpaceMouse{ (mouse_relative_x - winOffset.x) / winScreenDim.x * worldSpaceDim.x + worldSpaceOffsetX, worldSpaceDim.y - ((mouse_relative_y - winOffset.y) / winScreenDim.y * worldSpaceDim.y) };
			
			static ImVec2 lastWorldSpaceMouse;
			ImVec2 worldSpaceMouse{ (mouse_relative_x - winOffset.x) / winScreenDim.x * worldSpaceDim.x, ((mouse_relative_y - winOffset.y) / winScreenDim.y * worldSpaceDim.y) };

			auto* input = SliceEngine::Core::GetInstance()->GetInputSystem();

			static SliceEngine::CursorState game_cursor_state;
			static bool onFocus{ false };
			static bool isFocused{ false };

			if (ImGui::IsWindowFocused())
			{
				if (!isFocused)
					onFocus = true;

				isFocused = true;

				if (core->GetSceneSystem()->mCurrentState == SliceEngine::SceneState::PLAY_SCENE)
				{
					if (onFocus)
					{
						input->SetCursorState(game_cursor_state);
					}

					else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					{
						game_cursor_state = input->GetCursorState();
						input->SetCursorState(SliceEngine::CursorState::DEFAULT);
						ImGui::SetWindowFocus(NULL);
						isFocused = false;
					}

					input->SetMousePosition(worldSpaceMouse.x, worldSpaceMouse.y);
					if (fabs(worldSpaceMouse.x - lastWorldSpaceMouse.x) < 100.f && fabs(worldSpaceMouse.y - lastWorldSpaceMouse.y) < 100.f)
						input->SetMouseDelta(lastWorldSpaceMouse.x - worldSpaceMouse.x,lastWorldSpaceMouse.y - worldSpaceMouse.y);
					else
						input->SetMouseDelta(0.0, 0.0);
					lastWorldSpaceMouse = worldSpaceMouse;
				}

				onFocus = false;
			}

#pragma endregion

		}

		else
		{
			std::string msg = "No Cameras detected.";
			auto middle_pos = ImGui::GetWindowPos() + ImGui::GetWindowSize() / 2 - ImGui::CalcTextSize(msg.c_str()) / 2;
			ImGui::SetCursorScreenPos(middle_pos);
			ImGui::Text(msg.c_str());
		}

		position = ImGui::GetWindowPos();
		size = ImGui::GetWindowSize();
		center = { position.x + size.x / 2.0f, position.y + size.y / 2.0f };

		ImGui::End();
	}
}
