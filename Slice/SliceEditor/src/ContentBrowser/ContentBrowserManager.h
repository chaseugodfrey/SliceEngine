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

	enum class FileEventType
	{
		ADDED,
		REMOVED,
		MODIFIED
	};

	struct FileEvent
	{
		std::filesystem::path path;
		FileEventType type;
	};

	class ContentBrowserManager : public IBaseManager, public ICreateWindow
	{

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

		bool openRenameFile = false;
		bool mHasDroppedAssets = false;

		void Init() override;
		void Update() override;

		void RebuildDirectory(DirectoryNode& node);

		void SetSelectedFolder(DirectoryNode& node);

		void RenameFile(DirectoryNode& entry, char* newName);

		void OpenFile(DirectoryNode& entry);

		void DeleteNode(DirectoryNode& entry);
		
		std::unique_ptr<EditorWindow> CreateEditorWindow() override;
	};
}

#endif