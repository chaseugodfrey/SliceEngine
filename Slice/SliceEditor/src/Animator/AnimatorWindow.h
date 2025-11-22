#ifndef ANIMATOR_WINDOW_H
#define ANIMATOR_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace NodeEditor = ax::NodeEditor;

namespace SliceEditor
{
	class Registry;

	struct AnimatorData
	{
		SliceEngine::Animator* animator = nullptr;
		std::unique_ptr<StateMachineData> stateMachine;
		std::unordered_map<std::string, StateNode> nodeMap;
	};

	class AnimatorWindow : public EditorWindow
	{

		SliceEngine::Animator* mCurrentAnimator = nullptr;
		std::unique_ptr<StateMachineData> mStateMachineAsset;

		std::unordered_map<std::string, StateNode> mNameToNodeMap;
		std::unordered_map<int, TransitionLinkNode> mIndexToLinkMap;

		std::string mSelectedState;
		int mSelectedLink;

		bool CheckForAnimator();
		void ClearData();

		bool RemoveTransitionFromState(std::string stateName, int id);

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