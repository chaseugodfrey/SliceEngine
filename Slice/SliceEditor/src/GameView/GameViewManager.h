#ifndef GAME_VIEW_MANAGER_H
#define GAME_VIEW_MANAGER_H

#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"

namespace SliceEngine
{
	class RenderManager;
}

namespace SliceEditor
{
	class Registry;

	class GameViewManager : public IBaseManager, public ICreateWindow
	{

	public:

		GameViewManager(Registry& reg) : IBaseManager(reg) {};
		~GameViewManager() = default;

		void Init() override;

		std::unique_ptr<EditorWindow> CreateEditorWindow() override;
	};
}
#endif
