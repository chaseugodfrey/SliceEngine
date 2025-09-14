#ifndef EDITOR_H
#define EDITOR_H

struct GLFWwindow;

#include "Engine.h"

namespace SliceEditor
{
	class Editor
	{
		SliceEngine::Engine engine;
		//std::unique_ptr<EditorState> editorState;
		//std::unique_ptr<WindowManager> windowManager;

		void InitImGUI(GLFWwindow* window);
		void InitEditorState();
		//void InitWindowManager(EditorState& editorState);

		void Render();

	public:

		void Init();
		void Run();
		void Exit();
	};
}

#endif