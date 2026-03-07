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
		entt::entity tmpEnt;

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

		void CreateNode(std::string newNode);
		void DeleteNode(uint16_t id);
		void SelectNode(uint16_t id);
		void SelectLink(uint16_t id);
		void AddLink(uint16_t id,std::string targetState);

		bool RemoveTransitionFromState(uint16_t id);

		bool CheckStateInput(StateNode* node);
		bool CheckLinkInput(TransitionLinkNode* node);
		void DrawStateNode(StateNode* node);
		void CheckStateNode();
		void DrawTransitionLinkNode(TransitionLinkNode* node);
		void CheckTransitionLinkNode();
		void LoadDataFromAnimator(SliceEngine::Animator* component, entt::entity entity);

	public:
		AnimatorWindow(Registry& reg) : EditorWindow(reg) {};
		~AnimatorWindow();
		void Init() override;
		void Draw() override final;
	};
}

#endif