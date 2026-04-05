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

		ImVec2 mouseSelectPos{};

		int uniqueIDCnt{ 0 };
		int newNodeID{ 0 };

		bool tempLoadPos = false;
		bool isSaved = true;

		struct ShaderEditableNode : SelectionNode
		{
			int id{};
			int out_id{};

			std::string name;
			rttr::variant baseData;
			SliceEngine::SliceEngineTypes::CSHAD_T baseDataType = SliceEngine::SliceEngineTypes::CSHAD_T::NIL;
			ShaderEditableNode() : SelectionNode(SelectionType::SHADER_STATE) {}
		};
		struct ShaderStateNode : SelectionNode
		{
			int id{};

			int out_id{};
			std::vector<int> in_ids;

			std::string name{};
			ShaderStateNode() :SelectionNode(SelectionType::SHADER_FUNCTION_STATE) {}
		};
		struct ShaderLinkNode : SelectionNode
		{
			int id{};

			int sourceAttr{};
			int destAttr{};

			ShaderLinkNode() : SelectionNode(SelectionType::SHADER_LINK_STATE) {}
		};

		std::unordered_map<int, ShaderStateNode> mDefaultIns;
		std::unordered_map<int, ShaderEditableNode> mEditableIns;
		std::unordered_map<int, ShaderStateNode> mStateNodes;
		std::unordered_map<int, ShaderLinkNode> mTransitionNodes;

		std::unordered_map<int, int> attrIDToNodeID;
		std::unordered_map<int, int> attrIDToLinkID; // Both Ways
		ShaderStateNode mFinalNode; // 1 Node here, but treated as 4 nodes in saving & Loading
		std::vector<std::string> mFinalNodeOutputNames;

		void create_default();
		void DrawSideBar();
		void DrawNodeEditor();
		void DrawStateNode(ShaderStateNode&);
		void DrawFinalNode();
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
		int CreateNode(std::string);
		int CreateEditable(SliceEngine::SliceEngineTypes::CSHAD_T);
	public:

		CustomShaderWindow(Registry& reg);
		~CustomShaderWindow();

		void DeleteButtonPress();
		void CheckFileData();
		void SaveFileData();
		void Init() override;
		void Draw() override final;
	};
}
#endif