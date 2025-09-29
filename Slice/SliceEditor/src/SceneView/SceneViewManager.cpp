#include <pch.h>
#include "SceneViewManager.h"
#include "SceneViewWindow.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"
#include "../../SliceEngine/src/ECS/ECSTypes.h"

namespace SliceEditor
{
	void SceneViewManager::Init()
	{
		mCameraSpeed = 0.01f;
	}

	std::unique_ptr<EditorWindow> SceneViewManager::CreateWindow()
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

		SLICE_LOG_VALUES("New Camera Speed: ", mCameraSpeed);
	}
}
