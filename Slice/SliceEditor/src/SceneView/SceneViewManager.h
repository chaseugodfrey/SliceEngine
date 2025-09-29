#ifndef SCENE_VIEW_MANAGER_H
#define SCENE_VIEW_MANAGER_H

#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"
#include "../SelectionSystem/ISelectionListener.h"

namespace SliceEngine
{
	class RenderManager;
}

namespace SliceEditor
{
	class Registry;

	class SceneViewManager : public IBaseManager, public ICreateWindow
	{

	public:

		SceneViewManager(Registry& reg) : IBaseManager(reg) {};
		~SceneViewManager() = default;

		void Init() override;

		std::unique_ptr<EditorWindow> CreateWindow() override;
	};
}
#endif
