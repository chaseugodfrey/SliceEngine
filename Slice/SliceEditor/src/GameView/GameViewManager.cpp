#include <pch.h>
#include "GameViewManager.h"
#include "GameViewWindow.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"

namespace SliceEditor
{
	void GameViewManager::Init()
	{

	}

	std::unique_ptr<EditorWindow> GameViewManager::CreateEditorWindow()
	{
		auto window = std::make_unique<GameViewWindow>(*this);

		return window;
	}
}
