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