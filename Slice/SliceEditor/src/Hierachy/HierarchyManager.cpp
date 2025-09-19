#include <pch.h>
#include "HierarchyManager.h"
#include "HierarchyWindow.h"

#define NODE_NULL = (int)-1;

namespace SliceEditor
{
	HierarchyManager::HierarchyManager()
	{

	}

	void HierarchyManager::Init()
	{
		int count = 10;
		mNodes = std::vector<TestNode>(count);

		for (int i = 0; i < count; ++i) {
			mNodes[i].parent = -1;   // or set as needed
			mNodes[i].child = -1;
			mNodes[i].previous = (i == 0) ? -1 : i - 1;
			mNodes[i].next = (i == count - 1) ? -1 : i + 1;
		}
	}

	std::unique_ptr<EditorWindow> HierarchyManager::CreateWindow()
	{
		auto window = std::make_unique<HierarchyWindow>(*this);
		return window;
	}
}