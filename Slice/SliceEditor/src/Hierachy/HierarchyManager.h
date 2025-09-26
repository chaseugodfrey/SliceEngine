#ifndef HIERARCHY_MANAGER_H
#define HIERARCHY_MANAGER_H

#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"

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

	class Registry;

	class HierarchyManager : public IBaseManager, public ICreateWindow
	{
		// TO DO: replace this with proper scene graph
		std::vector<TestNode> mRootNodes;

	public:

		HierarchyManager(Registry& reg) : IBaseManager(reg) {};
		~HierarchyManager() = default;

		void Test();
		void Init() override;
		void BuildHierarchy();
		void AddGameObject();

		std::unique_ptr<EditorWindow> CreateWindow() override;

		std::vector<TestNode>& GetNodes() { return mRootNodes; }
	};
}

#endif