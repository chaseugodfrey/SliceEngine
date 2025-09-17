#include "pch.h"
#include "SceneViewManager.h"
#include "SceneViewWindow.h"

namespace SliceEditor
{
	std::unique_ptr<EditorWindow> SceneViewManager::CreateWindow()
	{
		auto window = std::make_unique<SceneViewWindow>(*this);
		//mRenderManager.CreateCamera(windowManager.GetMainWindow());
		return window;
	}
}
