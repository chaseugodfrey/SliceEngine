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
		auto id = mRenderManager.GetTexture();

		auto window = std::make_unique<SceneViewWindow>(*this);
		window->SetCameraTexture(id);

		return window;
	}

	void SceneViewManager::OnUpdateSelected(std::unordered_set<entt::entity>&)
	{

	}
}
