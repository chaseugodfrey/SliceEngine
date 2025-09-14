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
		void Init();
		void Render();

		template <typename T>
		void AddWindow();

	};

	template <typename T>
	void WindowManager::AddWindow()
	{
		list.push_back(std::make_unique<T>());
	}

}



#endif