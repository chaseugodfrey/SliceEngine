#include <pch.h>
#include "SceneViewManager.h"
#include "SceneViewWindow.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"

namespace SliceEditor
{
	void SceneViewManager::Init()
	{

	}

	std::unique_ptr<EditorWindow> SceneViewManager::CreateWindow()
	{
		// TO DO: replace this with camera creation + attaching to window
		auto mRenderManager = SliceEngine::RenderManagerInstance;
		auto id = mRenderManager->GetTexture();
		auto camObj = mRenderManager->CreateCamera();

		auto window = std::make_unique<SceneViewWindow>(*this, camObj);
		window->SetCameraTexture(id);

		return window;
	}

	void SceneViewManager::OnUpdateSelected(std::unordered_set<entt::entity>&)
	{

	}
}
