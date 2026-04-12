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
		EventManager::GetInstance()->Subscribe<OnStopEvent, &GameViewWindow::OnStop>(this);
	}

	void GameViewWindow::OnPlay(OnPlayEvent e)
	{
		mIsPlayMode = true;
		mRequestToFocus = true;
		auto core = SliceEngine::Core::GetInstance();
		mLastCursorState = core->GetInputSystem()->GetCurrCursorState();
		
	}

	void GameViewWindow::OnStop(OnStopEvent e)
	{
		mIsPlayMode = false;
		mLastCursorState = {};
		auto core = SliceEngine::Core::GetInstance();
		core->GetInputSystem()->ResetCursorState();
		core->GetSceneSystem()->SetTimeScale(1.0f);

	}

	void GameViewWindow::Draw()
	{
		ImGui::Begin("Game", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

		if (mRequestToFocus)
		{
			ImGui::SetWindowFocus();
			mRequestToFocus = false;
		}

		position = ImGui::GetWindowPos();
		size = ImGui::GetWindowSize();
		center = { std::floor(position.x + size.x / 2.0f), std::floor(position.y + size.y / 2.0f) };
		

		DrawHeaderBar();
		mWindowScreen.CalculatePositions(ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail());
		DrawCameraView();
		UpdateGameMousePosition();
		CaptureInputs();
		DrawDebugInfo();

		ImGui::End();
	}

	void GameViewWindow::DrawHeaderBar()
	{
		ImGuiSelectableFlags selectable_flags = mIsDebuggingEnabled ? ImGuiSelectableFlags_Highlight : ImGuiSelectableFlags_None;

		ImGui::BeginMenuBar();

		if (ImGui::Selectable("Debug", false, selectable_flags, ImGui::CalcTextSize("Debug")))
		{
			mIsDebuggingEnabled = !mIsDebuggingEnabled;
		}

		ImGui::EndMenuBar();
	}

	void GameViewWindow::DrawCameraView()
	{
		auto core = SliceEngine::Core::GetInstance();

		auto& possibleCam = core->GetRenderManager()->GetGameCamera();

		if (!possibleCam.has_value() || SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Camera>(possibleCam.value()) == nullptr)
		{
			std::string msg = "No Cameras detected.";
			auto middle_pos = ImGui::GetWindowPos() + ImGui::GetWindowSize() / 2 - ImGui::CalcTextSize(msg.c_str()) / 2;
			ImGui::SetCursorScreenPos(middle_pos);
			ImGui::Text(msg.c_str());

			return;
		}

		auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(possibleCam.value());

		ImTextureID tex = static_cast<ImTextureID>(cam.textureID);

		ImVec2 pos = ImGui::GetCursorScreenPos();

		// win as in the scene Window
		ImVec2 winOffset{ 0.f,0.f };
		ImVec2 totalWinScreenDim = mWindowScreen.size;
		ImVec2 winNewScreenSize = totalWinScreenDim;
		float winScreenAR = winNewScreenSize.x / winNewScreenSize.y;
		// worldSpace as in what the camera renders (i'm going to change this val when I crop out parts of the image when streching the height)
		ImVec2 worldSpaceDim{ static_cast<float>(cam.width) , static_cast<float>(cam.height) };
		float worldSpaceAR = worldSpaceDim.x / worldSpaceDim.y;
		float worldSpaceOffsetX{ 0.f };

		ImVec2 uvCropMin{ 0.f, 1.f }; // it's like this cuz it's flipped
		ImVec2 uvCropMax{ 1.f, 0.f };

		if (winScreenAR > worldSpaceAR) // Stretch the width
		{
			winNewScreenSize.x = totalWinScreenDim.y * worldSpaceAR;
			winOffset.x = (totalWinScreenDim.x - winNewScreenSize.x) / 2.f;
		}
		else if (winScreenAR < worldSpaceAR) // Stretch the height
		{
			winNewScreenSize.y = totalWinScreenDim.x / worldSpaceAR;
			winOffset.y = (totalWinScreenDim.y - winNewScreenSize.y) / 2.f;

			// world space: y & x dim stays the same
			float wouldBeWinWidth = totalWinScreenDim.y * worldSpaceAR;
			float percentScreenShown = totalWinScreenDim.x / wouldBeWinWidth;
			uvCropMin.x += 0.5f - 0.5f * percentScreenShown;
			uvCropMax.x -= 0.5f - 0.5f * percentScreenShown;

			worldSpaceOffsetX = uvCropMin.x * worldSpaceDim.x;
			//this ui crop thing seems to be bugged, ignoring this offset stuff for now to get correct mouse pos
			//worldSpaceDim.x *= percentScreenShown;	

		}

		mGameScreen.CalculatePositions(mWindowScreen.topLeft + winOffset, winNewScreenSize);

		ImGui::GetWindowDrawList()->AddImage(
			tex,
			mGameScreen.topLeft,
			mGameScreen.btmRight,
			ImVec2(0, 1),
			ImVec2(1, 0)
		);
	}

	void GameViewWindow::UpdateGameMousePosition()
	{
		if (!mIsPlayMode)
			return;

		auto window = SliceEngine::Core::GetInstance()->GetWindow();

		if (!glfwGetWindowAttrib(window, GLFW_FOCUSED))
			return;

		auto inputSystem = SliceEngine::Core::GetInstance()->GetInputSystem();
		auto cursor_state = inputSystem->GetCurrCursorState();
		auto mouse_pos = ImGui::GetIO().MousePos;

		auto relative_mouse_pos = mouse_pos - mGameScreen.topLeft;
		auto percentage_x = relative_mouse_pos.x / mGameScreen.size.x;
		auto percentage_y = relative_mouse_pos.y / mGameScreen.size.y;

		mGameMouseDelta = { 0.f, 0.f };

		if (cursor_state == SliceEngine::CursorState::DISABLED)
		{
			int window_size_x, window_size_y;

			glfwGetWindowSize(window, &window_size_x, &window_size_y);

			if (mLastCursorState != SliceEngine::CursorState::DISABLED)
			{
				SLICE_LOG_VALUES("Mouse Delta", mGameMouseDelta.x, mGameMouseDelta.y);
			}

			else
			{
				auto delta = mouse_pos - center;
				ImVec2 ratio = { window_size_x / mGameScreen.size.x,  window_size_y / mGameScreen.size.y };

				mGameMouseDelta = { delta.x * ratio.x, delta.y * ratio.y };
			}

			mGameMousePosition = mGameScreen.center;
			mGameMouseNDC = { 0.5f, 0.5f };

			// centering actual mouse position
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
			ImGui::GetIO().MousePos = center;

			int x, y;
			float x_scale, y_scale;
			glfwGetWindowContentScale(window, &x_scale, &y_scale);
			glfwGetWindowPos(window, &x, &y);
			//glfwSetCursorPos(window, (center.x - x) / x_scale, (center.y - y) / y_scale);
			glfwSetCursorPos(window, center.x - x, center.y - y);
		}

		else
		{
			ImVec2 new_mouse_pos = { percentage_x * mGameScreen.size.x, percentage_y * mGameScreen.size.y };

			if (mLastCursorState == SliceEngine::CursorState::DISABLED)
			{
				SLICE_LOG_VALUES("Mouse Delta", mGameMouseDelta.x, mGameMouseDelta.y);
				glfwSetCursorPos(window, center.x, center.y);
			}

			else
			{
				mGameMouseDelta = new_mouse_pos - mGameMousePosition;
			}

			mGameMousePosition = new_mouse_pos;
			mGameMouseNDC = { percentage_x, percentage_y };
		}

		mGameMouseDelta *= -1; // flip y delta so that up is positive
		
		inputSystem->SetMousePosition(mGameMousePosition.x, mGameMousePosition.y);
		inputSystem->SetMouseDelta(mGameMouseDelta.x, mGameMouseDelta.y);
		inputSystem->SetMouseNDC(mGameMouseNDC.x, mGameMouseNDC.y);
		mLastCursorState = cursor_state;

		mIsHoveringGameScreen = false;

		if (mouse_pos.x < mGameScreen.topLeft.x || mouse_pos.y < mGameScreen.topLeft.y ||
			mouse_pos.x > mGameScreen.btmRight.x || mouse_pos.y > mGameScreen.btmRight.y)
			return;

		mIsHoveringGameScreen = true;
	}

	void GameViewWindow::CaptureInputs()
	{
		if (!mIsPlayMode)
			return;

		auto inputSystem = SliceEngine::Core::GetInstance()->GetInputSystem();

		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				ImGui::SetWindowFocus(NULL);
				mLastCursorState = inputSystem->GetCurrCursorState();
				inputSystem->SetCursorState(SliceEngine::CursorState::DEFAULT);
			}

			if (!mIsFocused)
			{
				inputSystem->SetCursorState(mLastCursorState);
			}
		}

		mIsFocused = ImGui::IsWindowFocused();
	}

	void GameViewWindow::DrawDebugInfo()
	{
		if (!mIsDebuggingEnabled)
			return;

		auto window_pos = ImGui::GetWindowPos();
		auto window_size = ImGui::GetWindowSize();
		auto debugger_size = window_size * 0.3f;

		ImGui::SetNextItemAllowOverlap();
		ImGui::SetNextWindowBgAlpha(0.3f);
		ImGui::SetNextWindowPos(window_pos + window_size - debugger_size);

		ImGui::BeginChild("GameDebugInfo", debugger_size, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollWithMouse);

		ImGui::Text("Mouse Position");
		ImGui::Text("%.1f, %.1f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);

		ImGui::Text("Mouse Game Position");
		ImGui::Text("%.1f, %.1f", mGameMousePosition.x, mGameMousePosition.y);

		ImGui::Text("Mouse Delta");
		ImGui::Text("%.1f, %.1f", mGameMouseDelta.x, mGameMouseDelta.y);

		ImGui::Text("Mouse NDC");
		ImGui::Text("%.3f, %.3f", mGameMouseNDC.x, mGameMouseNDC.y);

		ImGui::Text("Window Center Position");
		ImGui::Text("%.1f, %.1f", center.x, center.y);

		ImGui::Text("Game Center Position");
		ImGui::Text("%.1f, %.1f", mGameScreen.center.x, mGameScreen.center.y);

		ImGui::Text("Hovering Game Screen");
		ImGui::Text("%s", mIsHoveringGameScreen ? "yes" : "no");
		
		ImGui::EndChild();
	}

	void GameViewWindow::Screen::CalculatePositions(ImVec2 start, ImVec2 newSize)
	{
		this->size = newSize;
		topLeft = start;
		btmRight = { size.x + start.x, size.y + start.y };
		center = { (btmRight.x - topLeft.x) / 2, (btmRight.y - topLeft.y) / 2 };
	}
}
