/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ContentBrowserManager.cpp

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Defines the Content Browser manager class. It is responsible for handling the data of the Content Browser window in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "ContentBrowserManager.h"
#include "ContentBrowserWindow.h"
#include "Core/Registry.h"
#include "../../SliceEngine/src/Systems/SceneSystem.h"

namespace SliceEditor
{
	void ContentBrowserManager::Init()
	{
		SLICE_LOG("Initializing Content Browser Data.");
		BuildTree();

		std::filesystem::path pathToWatch = rootNode->path;

		mFileWatcher = std::make_unique<filewatch::FileWatch>(
			pathToWatch, [this](const std::filesystem::path filePath, const filewatch::Event changeType)
			{
				std::filesystem::path path(filePath);

				if (path.extension() == ".meta")
				{
					return;
				}

				if (path.string().find(rootNode->path.string() + "/Resources") != std::string::npos)
				{
					return;
				}

				FileEvent event;

				event.path = path;

				bool shouldQueue = false;
				switch (changeType)
				{
				case filewatch::Event::added:
				case filewatch::Event::renamed_new:
					event.type = FileEventType::ADDED;
					shouldQueue = true;
					break;
				case filewatch::Event::modified:
					event.type = FileEventType::MODIFIED;
					shouldQueue = true;
					break;
				case filewatch::Event::removed:
				case filewatch::Event::renamed_old:
					event.type = FileEventType::REMOVED;
					shouldQueue = true;
					break;
				}

				if (shouldQueue)
				{
					std::lock_guard<std::mutex> lock(mFileEventQueueMutex);
					mFileEvents.push(event);
				}
			}
		);

	}

	std::unique_ptr<EditorWindow> ContentBrowserManager::CreateEditorWindow()
	{
		SLICE_LOG("Creating Content Browser Window.");
		auto window = std::make_unique<ContentBrowserWindow>(*this, registry);

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

	void ContentBrowserManager::SetSelectedFolder(DirectoryNode& node)
	{
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
		{	//This is where you tell the editor which is the next scene to change to - yy
			SliceEngine::Core::GetInstance()->GetSceneSystem()->LoadSceneIntoQueue(entry.path);
			//registry.GetManager<HierarchyManager>("Hierarchy")->Reset();
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
		std::filesystem::path pathToDelete = entry.path;

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

		auto assetMgr = registry.GetManager<AssetManager>("AssetManager");
		if (assetMgr)
		{
			assetMgr->HandleAssetRemoval(pathToDelete);
		}

	}

	void ContentBrowserManager::Update()
	{
		ProcessFileEvents();
	}

	void ContentBrowserManager::ProcessFileEvents()
	{
		std::queue<FileEvent> eventsToProcess;

		{
			std::lock_guard<std::mutex> lock(mFileEventQueueMutex);
			if (mFileEvents.empty())
			{
				return;
			}

			eventsToProcess.swap(mFileEvents);
		}

		auto assetMgr = registry.GetManager<AssetManager>("AssetManager");
		if (!assetMgr)
		{
			SLICE_LOG_ERROR("AssetManager not found while processing file events");
			return;
		}

		while (!eventsToProcess.empty())
		{
			FileEvent event = eventsToProcess.front();
			eventsToProcess.pop();

			switch (event.type)
			{
				case FileEventType::ADDED:
					SLICE_LOG("File Added: " + event.path.string());
					HandleFileAdded(event.path);
					break;
				case FileEventType::REMOVED:
					SLICE_LOG("File Removed: " + event.path.string());
					HandleFileRemoved(event.path);
					break;
				case FileEventType::MODIFIED:
					SLICE_LOG("File Modified: " + event.path.string());
					HandleFileModified(event.path);
					break;
			}
		}
	}

	void ContentBrowserManager::HandleFileAdded(const std::filesystem::path& path)
	{
		auto assetMgr = registry.GetManager<AssetManager>("AssetManager");
		if (!assetMgr)
		{
			return;
		}

		std::filesystem::path parentPath = path.parent_path();

		DirectoryNode* parent = FindNodeByPath(parentPath);

		if (parent == nullptr)
		{
			SLICE_LOG_WARNING("Could not find parent directory in tree for new file: " + path.string());

			return;
		}

		DirectoryNode child;
		child.fileName = path.filename().string();
		child.path = path;
		child.parent = parent;
		child.isDirectory = std::filesystem::is_directory(path);

		parent->children.insert({ child.fileName, child });

		if (child.isDirectory)
		{
			CreateDirectory(parent->children[child.fileName]);
		}
		else
		{
			assetMgr->CreateDescriptorFile(path);
		}
	}

	void ContentBrowserManager::HandleFileRemoved(const std::filesystem::path& path)
	{
		DirectoryNode* node = FindNodeByPath(path);

		if (node)
		{
			RemoveNodeFromTree(*node);
		}

		auto assetMgr = registry.GetManager<AssetManager>("AssetManager");
		if (assetMgr)
		{
			assetMgr->HandleAssetRemoval(path);
		}
	}

	void ContentBrowserManager::HandleFileModified(const std::filesystem::path& path)
	{
		auto assetMgr = registry.GetManager<AssetManager>("AssetManager");
		if (assetMgr)
		{
			assetMgr->RecompileAsset(path);
		}
	}

	void ContentBrowserManager::RemoveNodeFromTree(DirectoryNode& node)
	{
		if (node.parent)
		{
			node.parent->children.erase(node.fileName);
		}
	}

	DirectoryNode* ContentBrowserManager::FindNodeByPath(const std::filesystem::path& path)
	{
		if (!rootNode || path == rootNode->path)
		{
			return rootNode.get();
		}

		std::filesystem::path relativePath;
		try
		{
			relativePath = std::filesystem::relative(path, rootNode->path);
		}
		catch (const std::exception& e)
		{
			SLICE_LOG_ERROR("Failed to get relative path: " + std::string(e.what()));
			return nullptr;
		}

		if (relativePath.empty() || relativePath == ".")
		{
			return rootNode.get();
		}

		DirectoryNode* current = rootNode.get();
		for (const auto& part : relativePath)
		{
			auto it = current->children.find(part.string());
			if (it == current->children.end())
			{
				return nullptr;
			}

			current = &it->second;
		}

		return current;
	}
}