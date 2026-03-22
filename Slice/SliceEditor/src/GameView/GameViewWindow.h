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
#include "Input/InputTypes.h"

namespace SliceEditor
{
	class Registry;

	class GameViewWindow : public EditorWindow
	{
		struct Screen
		{
			ImVec2 topLeft;
			ImVec2 btmRight;
			ImVec2 size;
			ImVec2 center;

			void CalculatePositions(ImVec2 start, ImVec2 size);
		};

		Screen mWindowScreen;
		Screen mGameScreen;
		
		ImVec2 mRelativeCenter;
		ImVec2 mGameMousePosition;
		ImVec2 mGameMouseNDC;
		ImVec2 mGameMouseDelta;

		SliceEngine::CursorState mLastCursorState{};

		bool mIsPlayMode{};
		bool mIsFocused{};
		bool mIsDebuggingEnabled{};
		bool mIsHoveringGameScreen{};
		bool mRequestToFocus{};

		void OnPlay(OnPlayEvent e);
		void OnStop(OnStopEvent e);
		void DrawHeaderBar();
		void DrawCameraView();
		void DrawDebugInfo();
		void UpdateGameMousePosition();

		void CaptureInputs();

	public:

		GameViewWindow(Registry& reg) : EditorWindow(reg) {};
		~GameViewWindow() = default;

		void Init() override;
		void Draw() override final;
	};
}


#endif