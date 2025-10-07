/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SceneViewManager.cpp

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the SceneViewManager class, which manages the data of the scene view window of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "SceneViewManager.h"
#include "SceneViewWindow.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"
#include "../../SliceEngine/src/ECS/ECSTypes.h"
#include "../Core/Registry.h"

namespace SliceEditor
{
	void SceneViewManager::Init()
	{
		mCameraSpeed = 0.01f;
	}

	std::unique_ptr<EditorWindow> SceneViewManager::CreateEditorWindow()
	{
		// TO DO: replace this with camera creation + attaching to window
		auto mRenderManager = SliceEngine::RenderManagerInstance;
		// Will have error due to not yet loading the Camera 
		//auto id = mRenderManager->GetGameCamera().GetComponent<SliceEngine::Camera>().textureID;
		auto camObj = mRenderManager->CreateCamera();

		auto window = std::make_unique<SceneViewWindow>(*this, camObj);
		//window->SetCameraTexture(id);

		return window;
	}

	float SceneViewManager::GetCameraSpeed()
	{
		return mCameraSpeed;
	}


	void SceneViewManager::ChangeCameraSpeed(float speed)
	{

		mCameraSpeed += speed;

		if (mCameraSpeed < 0.f)
		{
			mCameraSpeed = 0.f;
		}

		//SLICE_LOG_VALUES("New Camera Speed: ", mCameraSpeed);
	}

	void SceneViewManager::SetGizmoOperation(ImGuizmo::OPERATION op)
	{
		mGuizmoOperation = op;
	}

	void SceneViewManager::SetGizmoMode(ImGuizmo::MODE mode)
	{
		mGuizmoMode = mode;
	}

	void SceneViewManager::SelectObject(entt::entity entity)
	{
		registry.GetSelectionSystem().UpdateSelected(entity);
	}

	void SceneViewManager::ClearObject()
	{
		registry.GetSelectionSystem().ClearSelection();
	}

	ImGuizmo::OPERATION SceneViewManager::GetGizmoOperation() const
	{
		return mGuizmoOperation;
	}

	ImGuizmo::MODE SceneViewManager::GetGizmoMode() const
	{
		return mGuizmoMode;
	}
}
