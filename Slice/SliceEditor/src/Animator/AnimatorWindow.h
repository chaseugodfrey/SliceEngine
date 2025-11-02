#ifndef ANIMATOR_WINDOW_H
#define ANIMATOR_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace NodeEditor = ax::NodeEditor;

namespace SliceEditor
{
	class Registry;

	struct NodeInfo
	{
		NodeEditor::NodeId   Id;
		NodeEditor::PinId	inputPinId;
		NodeEditor::PinId	outputPinId;
		std::string         Name;
	};

	struct LinkInfo
	{
		NodeEditor::LinkId Id;
		NodeEditor::PinId  sourceId;
		NodeEditor::PinId  targetId;
	};

	class AnimatorWindow : public EditorWindow
	{
		NodeEditor::EditorContext* m_Context = nullptr;

		ImVector<NodeInfo> m_Nodes;
		ImVector<LinkInfo> m_Links;

	public:
		AnimatorWindow(Registry& reg) : EditorWindow(reg) {};
		~AnimatorWindow();
		void Init() override;
		void Draw() override final;
	};
}

#endif