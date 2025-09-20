#ifndef SCENE_VIEW_MANAGER_H
#define SCENE_VIEW_MANAGER_H

#include "../WindowManager/ICreateWindow.h"

namespace SliceEngine
{
	class RenderManager;
}

namespace SliceEditor
{
	class SceneViewManager : public ICreateWindow
	{

	public:
		SliceEngine::RenderManager& mRenderManager;

		SceneViewManager(SliceEngine::RenderManager& renderManager) : mRenderManager(renderManager) {}
		~SceneViewManager() = default;

		void Init();

		std::unique_ptr<EditorWindow> CreateWindow() override;

	};
}
#endif
