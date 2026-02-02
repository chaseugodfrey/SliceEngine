#ifndef CUSTOMSHADER_WINDOW_H
#define CUSTOMSHADER_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class Registry;
	class SelectionManager;
	class SessionManager;

	struct EntityNode;

	class CustomShaderWindow : public EditorWindow
	{
		SelectionManager* mSelectionManager = nullptr;
		SessionManager* mSessionManager = nullptr;
		std::string mCurrCSPath = "";

		int32_t nodeTransitionCounter = 0;
		int32_t nodeIDCounter = 0;
		int32_t linkIDCounter = 0;

		struct EditableNode : SelectionNode
		{
			int id{};
			int out_id{};

			std::string name;
			std::variant<bool, uint32_t, int32_t, float> baseData;
			EditableNode()
			{
				type = SelectionType::STATE;
			}
		};

		struct CStateNode : SelectionNode
		{
			int id{};
			int out_id{};

			std::vector<int> inIDs;
			std::vector<int> transitionIds;

			std::string name{};
			CStateNode()
			{
				type = SelectionType::SHADER_FUNCTION_STATE;
			}
		};
		CStateNode colorExitNode;
		CStateNode roughMetExitNode;

		std::unordered_map<int, StateNode> mDefaultIns;
		std::unordered_map<int, EditableNode> mEditableIns;
		std::unordered_map<int, StateNode> mDefaultOuts;
		std::unordered_map<int, CStateNode> mStateNodes;
		std::unordered_map<int, TransitionLinkNode> mTransitionNodes;

		void create_default();
		void DrawStateNode(CStateNode&);
		void DrawDefaultInNode(StateNode&);
		void DrawDefaultOutNode(StateNode&);
		void DrawEditableInNode(EditableNode&);
		void DrawTransitionNodes(TransitionLinkNode&);
		void DrawPostEditorElements();
		void InitNodePos(int);
	public:

		CustomShaderWindow(Registry& reg) : EditorWindow(reg) {};
		~CustomShaderWindow() = default;

		void CheckFileData();
		void Init() override;
		void Draw() override final;
	};
}
#endif