#ifndef SCENE_VIEW_MANAGER_H
#define SCENE_VIEW_MANAGER_H

#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"
#include "../SelectionSystem/ISelectionService.h"

namespace SliceEngine
{
	class RenderManager;
}

namespace SliceEditor
{
	class Registry;

	class SceneViewManager : public IBaseManager, public ICreateWindow, public ISelectionListener
	{

	public:

		SceneViewManager(Registry& reg) : IBaseManager(reg) {};
		~SceneViewManager() = default;

		void Init() override;

		std::unique_ptr<EditorWindow> CreateWindow() override;


		// Inherited via ISelectionListener
		void OnUpdateSelected(std::unordered_set<entt::entity>&) override;

	};
}
#endif
