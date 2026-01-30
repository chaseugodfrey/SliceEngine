#ifndef ANIMATOR_WINDOW_H
#define ANIMATOR_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class Registry;
	class SessionManager;

	class AnimatorWindow : public EditorWindow
	{
		SessionManager* mSessionManager = nullptr;
		SliceEngine::Animator* mCurrentAnimator = nullptr;

		StateNode entryNode;
		StateNode exitNode;

		AnimatorData* mAnimatorData = nullptr;

		void ClearSelectionSubscribe(ClearSelectionEvent e);
		void ClearSelection();
		void DrawMenuBar();
		void DrawParameters();
		void DrawEntryNode();
		void DrawExitNode();
		void DrawNodeEditor();
		void DrawPostEditorElements();

		void CheckForAnimator();
		void SaveAnimatorData();
		void ClearData();

		void CreateNode();
		void DeleteNode(uint16_t id);
		void SelectNode(uint16_t id);

		bool RemoveTransitionFromState(uint16_t id);

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