#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "WindowTypes.h"
#include "EditorWindow.h"
//#include "EditorState.h"

namespace SliceEditor
{
	class WindowManager
	{
		//std::shared_ptr<EditorState> editorState;
		void DrawMainMenu();
		void DrawDockspace();
		void DrawPreferenceWindow();

	public:
		
		std::vector<std::unique_ptr<EditorWindow>> list;

		//void Init(EditorState& editorState);
		void Render();

		//template <typename T>
		//void AddWindow(EditorState& editorState);

	};

	//template <typename T>
	//void WindowManager::AddWindow(EditorState& editorState)
	//{
	//	list.push_back(std::make_unique<T>(editorState));
	//}

}



#endif