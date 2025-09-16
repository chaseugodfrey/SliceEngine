#ifndef CONTENT_BROWSER_MANAGER_H
#define CONTENT_BROWSER_MANAGER_H

#include <memory>
#include "../WindowManager/ICreateWindow.h"

namespace SliceEditor
{
	struct DirectoryNode;

	class ContentBrowserManager : public ICreateWindow
	{


		void BuildTree();

		void ResetRootDirectory(DirectoryNode& node);

		void CreateDirectory(DirectoryNode& node);

	
	public:
		std::unique_ptr<DirectoryNode> rootNode;

		DirectoryNode* selectedFolder;

		bool openRenameFile = false;

		void Init();
		
		void RebuildDirectory(DirectoryNode& node);

		void RenameFile(DirectoryNode& entry, char* newName);

		void OpenFile();

		void DeleteFile(DirectoryNode& entry);
		
		std::unique_ptr<EditorWindow> CreateWindow() override;
	};
}

#endif