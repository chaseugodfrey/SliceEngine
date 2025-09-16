#ifndef COnTENT_BROWSER_WINDOW_H
#define CONTENT_BROWSER_WINDOW_H

#include "../WindowManager/EditorWindow.h"
#include "ContentBrowserManager.h"

namespace SliceEditor
{
	class ContentBrowserWindow : public EditorWindow
	{
		ContentBrowserManager& manager;
	public:
		//ContentBrowserManager() = default;
		~ContentBrowserWindow() = default;
		ContentBrowserWindow(ContentBrowserManager& manager);
		void Draw() override final;

		void DisplayFolders(DirectoryNode& node);

		void DisplayItems(DirectoryNode& node);

		void RenameFilePopup(DirectoryNode& entry);

		void SelectFile(DirectoryNode& node);
	};
}

#endif