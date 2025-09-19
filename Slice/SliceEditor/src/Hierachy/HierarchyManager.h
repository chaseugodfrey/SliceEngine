#ifndef HIERARCHY_MANAGER_H
#define HIERARCHY_MANAGER_H

#include "../WindowManager/ICreateWindow.h"

namespace SliceEditor
{
	struct TestNode
	{
		int parent;
		int child;
		int previous;
		int next;
	};

	class HierarchyManager : public ICreateWindow
	{
		// TO DO: replace this with proper scene graph
		std::vector<TestNode> mNodes;

	public:

		HierarchyManager();
		~HierarchyManager() = default;

		void Init();

		std::unique_ptr<EditorWindow> CreateWindow() override;

		std::vector<TestNode>& GetNodes() { return mNodes; }
	};
}

#endif