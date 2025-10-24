#ifndef ANIMATOR_WINDOW_H
#define ANIMATOR_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace NodeEditor = ax::NodeEditor;

namespace SliceEditor
{
	class Registry;

	class AnimatorWindow : public EditorWindow
	{
		NodeEditor::EditorContext* m_Context = nullptr;

	public:
		AnimatorWindow(Registry& reg) : EditorWindow(reg) {};
		~AnimatorWindow();
		void Init() override;
		void Draw() override final;
	};
}

#endif