#ifndef HIERARCHY_MANAGER_H
#define HIERARCHY_MANAGER_H

#include "../WindowManager/ICreateWindow.h"
#include "../SelectionSystem/ISelectionService.h"

namespace SliceEditor
{
	struct TestNode
	{
		TestNode* parent;
		entt::entity entity;
		std::string name;
		bool isSelected;
		std::vector<TestNode> children;
	};

	class HierarchyManager : public ICreateWindow, public ISelectionListener
	{
		// TO DO: replace this with proper scene graph
		std::vector<TestNode> mRootNodes;


	public:

		HierarchyManager();
		~HierarchyManager() = default;

		void Test();
		void Init();
		void BuildHierarchy();
		void AddGameObject();

		std::unique_ptr<EditorWindow> CreateWindow() override;

		std::vector<TestNode>& GetNodes() { return mRootNodes; }

		// Inherited via ISelectionListener
		void OnUpdateSelected(std::unordered_set<entt::entity>&) override;
	};
}

#endif