/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        GameViewManager.cpp

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the GameViewManager class, which is responsible for managing the game view window in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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
