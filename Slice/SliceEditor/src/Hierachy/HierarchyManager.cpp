#include <pch.h>
#include "HierarchyManager.h"
#include "HierarchyWindow.h"

namespace SliceEditor
{
	HierarchyManager::HierarchyManager()
	{

	}

	void HierarchyManager::Test()
	{
		TestNode mTestRootNode{};
		mTestRootNode.parent = nullptr;

		for (size_t i = 0; i < 3; i++)
		{
			TestNode mNode{};
			mNode.parent = &mTestRootNode;
			mNode.name = std::to_string(i);

			for (size_t j = 0; j < 2; j++)
			{
				TestNode mNode2{};
				mNode2.parent = &mNode;
				mNode2.name = std::to_string(i) + "-" + std::to_string(j);
				mNode.children.push_back(mNode2);
			}

			mTestRootNode.children.push_back(mNode);
		}

		mRootNodes.push_back(mTestRootNode);
	}

	void HierarchyManager::Init()
	{
		Test();
	}

	std::unique_ptr<EditorWindow> HierarchyManager::CreateWindow()
	{
		auto window = std::make_unique<HierarchyWindow>(*this);
		return window;
	}
}