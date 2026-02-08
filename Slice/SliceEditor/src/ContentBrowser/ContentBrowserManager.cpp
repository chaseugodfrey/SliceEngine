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
#include "../../SliceEngine/src/Scripting/ScriptSystem.h"
#include "Core/Registry.h"
#include "../../SliceEngine/src/Systems/SceneSystem.h"
#include "Selection/SelectionManager.h"
#include "Session/SessionManager.h"
#include "../../SliceEngine/src/Systems/PrefabSystem.h"
#include "../History/HistoryManager.h"

namespace SliceEditor
{
	void ContentBrowserManager::Init()
	{
		SLICE_LOG("Initializing Content Browser Data.");
		LoadDefaultIcons();
		BuildTree();
		EventManager::GetInstance()->Subscribe<RefreshContentBrowser, &ContentBrowserManager::RebuildDirectory>(this);
	}

	void ContentBrowserManager::Update()
	{
		if (!mDeleteList.empty())
		{
			for (auto node : mDeleteList)
			{
				DeleteNode(*node);
			}
			mDeleteList.clear();
		}
	}

	void ContentBrowserManager::LoadDefaultIcons()
	{
		auto resouceManager = SliceEngine::Core::GetInstance()->GetResourceManager();
		//auto& assetManager = registry.GetAssetManager();
		for (int i = 1; i < iconNames.size(); i++)
		{
			if (iconNames[i].empty())
				continue;
			
			auto textureHandle = resouceManager->get<SliceEngine::SliceEngineTypes::Texture>("Editor/" + iconNames[i] + ".png");
			if (textureHandle.IsValid())
				defaultIconMap.emplace(i, textureHandle);
			else
				SLICE_LOG_WARNING("Failed to load default icon: " + iconNames[i]);
		}
	}

	std::optional<SliceEngine::Handle<Texture>> ContentBrowserManager::GetDefaultIconHandle(SelectionType type)
	{
		int typeByInt = static_cast<int>(type);
		auto it = defaultIconMap.find(typeByInt);
		if (it == defaultIconMap.end())
			return std::nullopt;

		return it->second;
	}

	std::optional<SliceEngine::Handle<Texture>> ContentBrowserManager::GetTextureIconHandle(std::string path)
	{
		return SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Texture>(path);
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
		rootNode->fullPath = std::filesystem::path("../SliceEditor/Assets");
		rootNode->relativePath = rootNode->fullPath;
		std::cout << rootNode->fullPath.string() << std::endl;
		//rootNode->path = std::filesystem::path(ASSET_DIR);
		rootNode->fileName = "Assets";
		rootNode->isDirectory = true;
		CreateDirectoryNode(*rootNode);
		selectedFolder = &*rootNode;

		auto& assetManager = registry.GetAssetManager();
		// testing categories

		auto sceneNode = std::make_unique<DirectoryNode>();
		auto& sceneList = assetManager.mAssetTypeToGUIDs[AssetType::Scene];
		sceneNode->fileName = "Scenes";
		
		for (const auto& guid : sceneList)
		{
			auto optFilename = assetManager.GetFilenameFromGUID(guid);
			if (optFilename.has_value())
			{
				DirectoryNode child;
				child.fileName = std::filesystem::path(optFilename.value()).filename().string();
				child.fullPath = assetManager.mAssetDirectory / optFilename.value();
				child.relativePath = std::filesystem::relative(child.fullPath, rootNode->fullPath);
				child.parent = sceneNode.get();
				child.isDirectory = false;
				child.type = SelectionType::SCENE;
				sceneNode->children.insert({ child.fileName, child });
			}
		}

		categoryNodes.push_back(std::move(sceneNode));
	}

	void ContentBrowserManager::RebuildDirectory()
	{
		ResetRootDirectory(*rootNode);
		CreateDirectoryNode(*rootNode);
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

	void ContentBrowserManager::CreateDirectoryNode(DirectoryNode& node)
	{
		if (node.fullPath.has_extension())
		{
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(node.fullPath))
		{
			const auto extension = entry.path().extension().string();
			if (extension == ".meta")
			{
				continue; //Ignore
			}

			DirectoryNode child;
			child.fileName = entry.path().filename().string();

			child.fullPath = entry.path();
			child.relativePath = std::filesystem::relative(child.fullPath, rootNode->fullPath);
			child.parent = &node;
			child.isDirectory = entry.is_directory();

			SelectionType type{};

			auto it = mExtensionToSelectionType.find(extension);
			if (it != mExtensionToSelectionType.end())
				type = it->second;
			else
				type = SelectionType::UNSUPPORTED;

			child.type = type;

			node.children.insert({ child.fileName, child });

			CreateDirectoryNode(node.children[child.fileName]);


		}
		return;
	}

	void ContentBrowserManager::RenameFile(DirectoryNode& entry, char* newName)
	{
		std::filesystem::path extension;
		std::filesystem::path newPath = entry.fullPath.parent_path() / newName;
		DirectoryNode& parent = *entry.parent;
		if (entry.fullPath.has_extension())
		{
			extension = entry.fullPath.extension();
		}
		newPath += extension;
		//std::filesystem::directory_entry actualEntry = std::filesystem::directory_entry(entry.path);
		try
		{
			std::filesystem::rename(entry.fullPath, newPath);

			//Resetting the Key in the Map
			auto key = parent.children.extract(entry.fileName);
			if (!key.empty())
			{
				entry.fileName = newName;
				entry.fullPath = newPath;

				key.key() = newName;
				parent.children.insert(std::move(key));
			}


		}
		catch (const std::exception& e)
		{
			ImGui::Text("Failed!", e.what());
		}
	}

	void ContentBrowserManager::OpenFile(DirectoryNode& entry)
	{
		auto& assetMan = registry.GetAssetManager();
		auto sessionMan = registry.GetManager<SessionManager>("Session");
		//Loading a Scene
		if (entry.fullPath.extension() == ".scene")
		{	//This is where you tell the editor which is the next scene to change to - yy
			//SliceEngine::Core::GetInstance()->GetSceneSystem()->LoadSceneIntoQueue(entry.path);
			SliceEngine::gScriptSystem->OnEnd();
			EditorUtilities::Scene_Load(entry.fullPath, *registry.GetManager<SelectionManager>("Selection"), *registry.GetManager<SessionManager>("Session"));
			EditorUtilities::Scene_CleanTempFiles(registry);
			//registry.GetManager<SelectionManager>("Selection Manager")->ClearSelection();
			//registry.GetManager<HierarchyManager>("Hierarchy")->Reset();
		}
		//Currently Open will Create a Prefab
		else if (entry.fullPath.extension() == ".prefab")
		{
			//auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
			//DOUBLE CHECK THE RM IF THEIR MAPS ARE BEING UPDATED CORRECTLY.
			std::string stem = entry.relativePath.generic_string();
				if (registry.GetAssetManager().mFilenameToGUID.find(stem) != registry.GetAssetManager().mFilenameToGUID.end())
				{
					SliceEngine::GUID guid = registry.GetAssetManager().mFilenameToGUID[stem];
					EditorUtilities::GameObject_CreatePrefab(guid, entt::null, registry.GetManager<HistoryManager>("History"));
				}
				else
				{
					SLICE_LOG("GUID NOT FOUND FOR PREFAB CREATION");
				}
		}
		//Currently Open will Create a Model
		else if (entry.fullPath.extension() == ".fbx")
		{
			//auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
			//DOUBLE CHECK THE RM IF THEIR MAPS ARE BEING UPDATED CORRECTLY.
			std::string stem = entry.relativePath.generic_string();
			if (assetMan.mFilenameToGUID.find(stem) != assetMan.mFilenameToGUID.end())
			{
				SliceEngine::GUID guid = registry.GetAssetManager().mFilenameToGUID[stem];
				if (sessionMan->IsPrefabInspected())
				{
					assetMan.CreateModelGO(guid, *registry.GetManager<HistoryManager>("History"), sessionMan->GetPrefabEntityInspected(), true);
				}
				else
				{
					assetMan.CreateModelGO(guid, *registry.GetManager<HistoryManager>("History"));
				}
				
				//EditorUtilities::GameObject_CreateModel(guid, entt::null, registry.GetManager<HistoryManager>("History"));
			}
			else
			{
				SLICE_LOG("GUID NOT FOUND FOR PREFAB CREATION");
			}
		}
		//No functionality
		else
		{
			SLICE_LOG("Open this file WIP!");

		}
	}

	void ContentBrowserManager::EditFile(DirectoryNode& entry)
	{
		//Editing a Prefab
		if (entry.fullPath.extension() == ".prefab")
		{
			registry.GetManager<SelectionManager>("Selection")->SelectSingle(&entry, true);
		}
		else if (entry.fullPath.extension() == ".mat")
		{
			registry.GetManager<SelectionManager>("Selection")->SelectSingle(&entry, true);
		}

		registry.GetManager<HistoryManager>("History")->CreateCheckpoint();
	}

	void ContentBrowserManager::DeleteNode(DirectoryNode& entry)
	{
		//SLICE_LOG_VALUES("Within DeleteFile Filename: " + entry.fileName);
		//SLICE_LOG_VALUES("Within DeleteFile Path: " + entry.path.string());
		DirectoryNode& parent = *entry.parent;
		//SLICE_LOG_VALUES("Entry Parent: " + (*entry.parent).fileName);
		//SLICE_LOG_VALUES("Copied Entry Parent: " + parent.fileName);
		std::string fileName = entry.fileName;
		try
		{
			if (std::filesystem::remove_all(entry.fullPath))
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