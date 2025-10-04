#include <pch.h>
#include "ContentBrowserManager.h"
#include "ContentBrowserWindow.h"
#include "Core/Registry.h"
#include "Hierachy/HierarchyManager.h"

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
		auto window = std::make_unique<ContentBrowserWindow>(*this);

		return window;
	}

	void ContentBrowserManager::BuildTree()
	{
		SLICE_LOG("Building Content Browser Tree.");
		rootNode = std::make_unique<DirectoryNode>();
		//std::string dir = ASSET_DIR;
		rootNode->path = std::filesystem::path("../SliceEditor/Assets");
		std::cout << rootNode->path.string() << std::endl;
		//rootNode->path = std::filesystem::path(ASSET_DIR);
		rootNode->fileName = "Assets";
		rootNode->isDirectory = true;
		CreateDirectory(*rootNode);
		selectedFolder = &*rootNode;
	}

	void ContentBrowserManager::RebuildDirectory(DirectoryNode& node)
	{
		ResetRootDirectory(node);
		CreateDirectory(node);
	}

	void ContentBrowserManager::ResetRootDirectory(DirectoryNode& node)
	{
		node.children.clear();
		selectedFolder = &node;
	}

	void ContentBrowserManager::CreateDirectory(DirectoryNode& node)
	{
		if (node.path.has_extension())
		{
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(node.path))
		{
			if (entry.path().extension().string() == ".meta")
			{
				continue; //Ignore
			}

			DirectoryNode child;
			child.fileName = entry.path().filename().string();

			child.path = entry.path();
			child.parent = &node;
			child.isDirectory = entry.is_directory();

			node.children.insert({ child.fileName, child });

			CreateDirectory(node.children[child.fileName]);


		}
		return;
	}

	void ContentBrowserManager::RenameFile(DirectoryNode& entry, char* newName)
	{
		std::filesystem::path extension;
		std::filesystem::path newPath = entry.path.parent_path() / newName;
		DirectoryNode& parent = *entry.parent;
		if (entry.path.has_extension())
		{
			extension = entry.path.extension();
		}
		newPath += extension;
		//std::filesystem::directory_entry actualEntry = std::filesystem::directory_entry(entry.path);
		try
		{
			std::filesystem::rename(entry.path, newPath);

			//Resetting the Key in the Map
			auto key = parent.children.extract(entry.fileName);
			if (!key.empty())
			{
				entry.fileName = newName;
				entry.path = newPath;

				key.key() = newName;
				parent.children.insert(std::move(key));
			}


			//actualEntry = std::filesystem::directory_entry(newPath);
		}
		catch (const std::exception& e)
		{
			ImGui::Text("Failed!", e.what());
		}
	}

	void ContentBrowserManager::OpenFile(DirectoryNode& entry)
	{
		if (entry.path.extension() == ".scene")
		{
			SliceEngine::Core::GetInstance()->GetSceneSystem()->LoadScene(entry.path);
			registry.GetManager<HierarchyManager>("Hierarchy")->Reset();
		}
		
		else
		{
			SLICE_LOG("Open this file WIP!");

		}
	}

	void ContentBrowserManager::DeleteFile(DirectoryNode& entry)
	{
		SLICE_LOG_VALUES("Within DeleteFile Filename: " + entry.fileName);
		SLICE_LOG_VALUES("Within DeleteFile Path: " + entry.path.string());
		DirectoryNode& parent = *entry.parent;
		SLICE_LOG_VALUES("Entry Parent: " + (*entry.parent).fileName);
		SLICE_LOG_VALUES("Copied Entry Parent: " + parent.fileName);
		std::string fileName = entry.fileName;
		try
		{
			if (std::filesystem::remove_all(entry.path))
			{
				SLICE_LOG("Deleted File: " + entry.fileName);
			}
			else
			{
				SLICE_LOG_WARNING("No such file found!");
			}
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			std::string error = e.what();
			SLICE_LOG_ERROR("Error: " + error);
		}

		SLICE_LOG_VALUES("Entry Parent Aft Delete: " + (*entry.parent).fileName);
		SLICE_LOG_VALUES("Copied Entry Parent Aft Delete: " + parent.fileName);

		parent.children.erase(fileName);

	}
}