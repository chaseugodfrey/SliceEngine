/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ContentBrowserManager.h

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Declares the Content Browser manager class. It is responsible for handling the data of the Content Browser window in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef CONTENT_BROWSER_MANAGER_H
#define CONTENT_BROWSER_MANAGER_H

#include <memory>
#include <mutex>
#include <queue>
#include <filesystem>
#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"
#include "../AssetManager/AssetManager.h"
#include "../thirdparty/filewatch/FileWatcher.h"

namespace SliceEditor
{
	struct DirectoryNode;
	class Registry;
	using Texture = SliceEngine::SliceEngineTypes::Texture;

	class ContentBrowserManager : public IBaseManager, public ICreateWindow
	{
		// hard coded for now
		const std::vector<std::string> iconNames = 
		{
			"",
			"FolderIcon",
			"",
			"",
			"",
			"AudioIcon",
			"SceneIcon",
			"ShaderIcon",
			"MaterialIcon",
			"PrefabIcon",
			"AnimationIcon",
			"AnimatorIcon",
			"FontIcon",
			"TxtIcon"
		};


		std::unordered_map<int, SliceEngine::Handle<SliceEngine::SliceEngineTypes::Texture>> defaultIconMap;

		void BuildTree();

		void ResetRootDirectory(DirectoryNode& node);

		void CreateDirectoryNode(DirectoryNode& node);

	public:

		ContentBrowserManager(Registry& reg) : IBaseManager(reg), selectedFolder(nullptr) {};
		~ContentBrowserManager() = default;

		std::unique_ptr<DirectoryNode> rootNode;

		DirectoryNode* selectedFolder;

		std::queue<DroppedFile> mPendingDrops;

		std::optional<DroppedFile> mActiveDrop;

		std::vector<DirectoryNode*> mDeleteList;

		bool openRenameFile = false;
		bool mHasDroppedAssets = false;

		void Init() override;
		void Update() override;

		void LoadDefaultIcons();

		std::optional<SliceEngine::Handle<Texture>> GetDefaultIconHandle(SelectionType);

		void RebuildDirectory();

		void SetSelectedFolder(DirectoryNode& node);

		void RenameFile(DirectoryNode& entry, char* newName);

		void OpenFile(DirectoryNode& entry);

		void EditFile(DirectoryNode& entry);

		void DeleteNode(DirectoryNode& entry);

		std::unique_ptr<EditorWindow> CreateEditorWindow() override;
	};
}

#endif