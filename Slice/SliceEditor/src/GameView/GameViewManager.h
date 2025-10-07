/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        GameViewManager.h

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the GameViewManager class, which is responsible for managing the game view window in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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
