#ifndef CONTENT_BROWSER_MANAGER_H
#define CONTENT_BROWSER_MANAGER_H

#include <memory>
#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"

namespace SliceEditor
{
	struct DirectoryNode;
	class Registry;

	class ContentBrowserManager : public IBaseManager, public ICreateWindow
	{

		void BuildTree();

		void ResetRootDirectory(DirectoryNode& node);

		void CreateDirectory(DirectoryNode& node);

	
	public:

		ContentBrowserManager(Registry& reg) : IBaseManager(reg) {};
		~ContentBrowserManager() = default;

		std::unique_ptr<DirectoryNode> rootNode;

		DirectoryNode* selectedFolder;

		bool openRenameFile = false;

		void Init() override;
		
		void RebuildDirectory(DirectoryNode& node);

		void RenameFile(DirectoryNode& entry, char* newName);

		void OpenFile(DirectoryNode& entry);

		void DeleteFile(DirectoryNode& entry);
		
		std::unique_ptr<EditorWindow> CreateEditorWindow() override;
	};
}

#endif