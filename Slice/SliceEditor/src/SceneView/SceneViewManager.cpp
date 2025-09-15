#include "pch.h"
#include "SceneViewManager.h"

namespace SliceEditor
{
	std::unique_ptr<EditorWindow> SceneViewManager::CreateWindow()
	{
		return std::unique_ptr<EditorWindow>();
	}
}
