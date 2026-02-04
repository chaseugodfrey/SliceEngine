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
		std::unique_ptr<ImNodesEditorContext*> editor_context_this;
		std::unique_ptr<ImNodesEditorContext*> editor_context_other;

		SelectionManager* mSelectionManager = nullptr;
		SessionManager* mSessionManager = nullptr;
		SliceEngine::GUID mCurrShaderGraphGUID{};

		int uniqueIDCnt{ 0 };

		bool tempLoadPos = false;

		struct ShaderEditableNode : SelectionNode
		{
			int id{};
			int out_id{};

			std::string name;
			std::variant<bool, uint32_t, int32_t, float> baseData;
			ShaderEditableNode()
			{
				type = SelectionType::SHADER_STATE;
			}
		};
		struct ShaderStateNode : SelectionNode
		{
			int id{};

			int out_id{};
			std::vector<int> in_ids;

			std::string name{};
			ShaderStateNode()
			{
				type = SelectionType::SHADER_FUNCTION_STATE;
			}
		};
		struct ShaderLinkNode : SelectionNode
		{
			int id{};

			int sourceAttr{};
			int destAttr{};

			ShaderLinkNode()
			{
				type = SelectionType::SHADER_LINK_STATE;
			}
		};

		int colorExitNodeID{};
		int roughMetExitNodeID{};

		std::unordered_map<int, ShaderStateNode> mDefaultIns;
		std::unordered_map<int, ShaderEditableNode> mEditableIns;
		std::unordered_map<int, ShaderStateNode> mStateNodes;
		std::unordered_map<int, ShaderLinkNode> mTransitionNodes;

		std::unordered_map<int, int> attrIDToNodeID;
		std::unordered_map<int, int> attrIDToLinkID; // Both Ways

		void create_default();
		void DrawStateNode(ShaderStateNode&);
		void DrawDefaultInNode(ShaderStateNode&);
		void DrawEditableInNode(ShaderEditableNode&);
		void DrawTransitionNodes(ShaderLinkNode&);
		void DrawPostEditorElements();
		void TempLoadPosAll();
		void InitNodePos(int id, float xPos, float yPos);
		void PostEditorChecks();

		void SelectNode(SelectionNode*);
		void DeleteLink(int);
		void DeleteLinkFromAttr(int);
	public:

		CustomShaderWindow(Registry& reg) : EditorWindow(reg) {};
		~CustomShaderWindow();

		void CheckFileData();
		void SaveFileData();
		void Init() override;
		void Draw() override final;
	};
}
#endif