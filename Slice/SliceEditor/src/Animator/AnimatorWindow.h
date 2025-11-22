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

		SliceEngine::Animator* mCurrentAnimator = nullptr;
		SliceEngine::SliceEngineTypes::StateMachine* mStateMachine = nullptr;

		std::unordered_map<std::string, StateNode> mNameToNodeMap;
		std::vector<TransitionLinkNode> mLinkList;

		SelectionNode* mSelectedNode;

		bool CheckForAnimator();
		void ClearData();

		bool CheckStateInput(StateNode* node);
		bool CheckLinkInput(TransitionLinkNode* node);
		void DrawStateNode(StateNode* node);
		void DrawTransitionLinkNode(TransitionLinkNode* node);
		void LoadDataFromAnimator(SliceEngine::Animator* component, entt::entity entity);

	public:
		AnimatorWindow(Registry& reg) : EditorWindow(reg) {};
		~AnimatorWindow();
		void Init() override;
		void Draw() override final;
	};
}

#endif