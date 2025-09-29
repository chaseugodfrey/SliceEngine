#include <pch.h>
#include "SceneViewManager.h"
#include "SceneViewWindow.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"
#include "../../SliceEngine/src/ECS/ECSTypes.h"

namespace SliceEditor
{
	void SceneViewManager::Init()
	{

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
}
