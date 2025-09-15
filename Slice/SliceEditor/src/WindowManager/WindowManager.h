#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "WindowTypes.h"
#include "EditorWindow.h"
//#include "EditorState.h"

namespace SliceEditor
{
	struct ICreateWindow;

	class WindowManager
	{
		std::vector<std::unique_ptr<EditorWindow>> list;
		std::unordered_map<std::string, ICreateWindow*> windowFactoryMap;

		//std::shared_ptr<EditorState> editorState;
		void DrawMainMenu();
		void DrawDockspace();
		void DrawPreferenceWindow();

	public:
		

		//void Init(EditorState& editorState);
		void AddWindow(const std::string& name);
		void Init();
		void RegisterInterface(const std::string& name, ICreateWindow* interfaceInstance);
		void Render();


	};
}



#endif