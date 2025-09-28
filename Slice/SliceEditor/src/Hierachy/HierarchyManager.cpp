#include <pch.h>
#include "HierarchyManager.h"
#include "HierarchyWindow.h"
#include "../Core/Registry.h"

namespace SliceEditor
{

	void HierarchyManager::Test()
	{
		//TestNode mTestRootNode{};
		//mTestRootNode.parent = nullptr;

		//for (size_t i = 0; i < 3; i++)
		//{
		//	TestNode mNode{};
		//	mNode.parent = &mTestRootNode;
		//	mNode.name = std::to_string(i);

		//	for (size_t j = 0; j < 2; j++)
		//	{
		//		TestNode mNode2{};
		//		mNode2.parent = &mNode;
		//		mNode2.name = std::to_string(i) + "-" + std::to_string(j);
		//		mNode.children.push_back(mNode2);
		//	}

		//	mTestRootNode.children.push_back(mNode);
		//}

		//mRootNodes.push_back(mTestRootNode);
	}

	void HierarchyManager::Init()
	{
		Test();
		BuildHierarchy();
	}

	void HierarchyManager::SetDirty()
	{
		isDirty = true;
	}

	void HierarchyManager::CheckDirty()
	{
		if (isDirty)
		{
			BuildHierarchy();
		}
	}

	void HierarchyManager::BuildHierarchy()
	{
		auto* core = SliceEngine::Core::GetInstance();
		TestNode node{};
		node.parent = nullptr;
		node.isSelected = false;
		node.name = "scene1";
		mRootNodes.push_back(node);

		auto& root = mRootNodes.back();

		auto entityview = core->GetRegistry().view<SliceEngine::SliceEntity>();

		int i = 1;
		
		for (auto entity: entityview)
		{
			TestNode childNode{};
			childNode.parent = &node;
			childNode.entity = entity;
			childNode.name = "dummy" + std::to_string(i++);
			root.children.push_back(childNode);
		}
	}

	void HierarchyManager::AddGameObject()
	{
		static int i = 10;
		auto go = SliceEngine::Core::GetInstance()->mFactory.CreateGO();
		go.AddComponent<SliceEngine::Renderer>();

		auto& rootNode = mRootNodes[0];
		TestNode node{};
		node.entity = go.GetEntity();
		node.parent = &rootNode;
		node.name = "child" + std::to_string(i++);
		rootNode.children.push_back(node);
	}

	std::unique_ptr<EditorWindow> HierarchyManager::CreateWindow()
	{
		auto& selectionSystem = registry.GetSelectionSystem();
		auto window = std::make_unique<HierarchyWindow>(*this, selectionSystem);
		return window;
	}
}