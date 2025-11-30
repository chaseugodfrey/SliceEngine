#include <pch.h>
#include "EditorInputs.h"
#include <Systems/SceneSystem.h>
#include <Input/InputSystem.h>
#include <Input/InputTypes.h>
#include <WindowManager/WindowManager.h>
namespace SliceEditor
{
	void EditorInputs::Update()
	{
		if (!isActive)
			return;


		// in order to toggle game input on/off from editor UI w/o restarting, tell inputsystem if imgui is capturing input this frame
		// editor tells inputsystem each frame whether imgui is using keyboard/mouse
		auto core = SliceEngine::Core::GetInstance();
		auto& io = ImGui::GetIO();
		auto input = SliceEngine::Core::GetInstance()->GetInputSystem();
		input->SetImGuiCapture(io.WantCaptureKeyboard, io.WantCaptureMouse); // set imgui capture state in inputsystem to capture input

		if (core->GetSceneSystem()->mCurrentState == SliceEngine::SceneState::PLAY_SCENE)
		{
			auto windowManager = registry.GetManager<WindowManager>("Windows");
			auto game_view = windowManager->GetWindow<GameViewWindow>();
			if (game_view.has_value())
			{
				if (core->GetInputSystem()->GetCursorState() == SliceEngine::CursorState::DISABLED)
				{
					io.MousePos = game_view.value()->center;
				}

			}

		}

		//Normal Inputs
		if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			EventManager::GetInstance()->Publish<DeleteSelectedEntities>();
		}


		// ctrl inputs

		if (io.KeyCtrl)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_S))
			{
				//EditorUtilities::Scene_Save();
				EventManager::GetInstance()->Publish<OnSceneSaveEvent>();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_D))
			{
				EventManager::GetInstance()->Publish<CloneSelectedEntities>();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_C))
			{

			}

			if (ImGui::IsKeyPressed(ImGuiKey_V))
			{

			}

			if (ImGui::IsKeyPressed(ImGuiKey_Z))
			{
				if (io.KeyShift)
				{
					EventManager::GetInstance()->Publish<RedoEvent>();
				}

				else
				{
					EventManager::GetInstance()->Publish<UndoEvent>();
				}
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Y))
			{

			}

		}
	}
}