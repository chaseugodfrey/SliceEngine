#include <pch.h>
#include "ContentBrowserManager.h"
#include "ContentBrowserWindow.h"

namespace SliceEditor
{
	void ContentBrowserManager::Init()
	{
		SLICE_LOG("Initializing Content Browser Data.");
		BuildTree();
	}

	std::unique_ptr<EditorWindow> ContentBrowserManager::CreateWindow()
	{
		SLICE_LOG("Creating Content Browser Window.");
		auto window = std::make_unique<ContentBrowserWindow>();

		return window;
	}

	void ContentBrowserManager::BuildTree()
	{
		SLICE_LOG("Building Content Browser Tree.");
		rootNode = std::make_unique<TreeNode>();
	}
}