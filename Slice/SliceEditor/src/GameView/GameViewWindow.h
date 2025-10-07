/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        GameViewWindow.h

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the GameViewWindow class, which is responsible for rendering the game view window in the editor, using the camera entity.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef GAME_VIEW_WINDOW_H
#define GAME_VIEW_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class GameViewManager;

	class GameViewWindow : public EditorWindow
	{
		GameViewManager& mManager;

	public:

		GameViewWindow(GameViewManager& manager);
		~GameViewWindow() = default;

		void Draw() override final;
	};
}


#endif