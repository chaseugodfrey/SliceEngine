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
	class Registry;

	class GameViewWindow : public EditorWindow
	{

	public:

		GameViewWindow(Registry& reg) : EditorWindow(reg) {};
		~GameViewWindow() = default;

		void Init() override;
		void Draw() override final;
	};
}


#endif