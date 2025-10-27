/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ContentBrowserWindow.h

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Declares the Content Browser Window class. It is responsible for rendering the Content Browser window in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef COnTENT_BROWSER_WINDOW_H
#define CONTENT_BROWSER_WINDOW_H

#include "../WindowManager/EditorWindow.h"
#include "ContentBrowserManager.h"

namespace SliceEditor
{
	class ContentBrowserWindow : public EditorWindow
	{
		ContentBrowserManager& mManager;
	public:
		//ContentBrowserManager() = default;
		~ContentBrowserWindow() = default;
		ContentBrowserWindow(ContentBrowserManager& manager, Registry& reg);

		void Init() override;
		void Draw() override final;

		void DisplayFolders(DirectoryNode& node);

		void DisplayItems(DirectoryNode& node);

		void DisplayButton(DirectoryNode* node, DirectoryNode& entry, bool isDirectory);

		void RenameFilePopup(DirectoryNode& entry);

		void CompileAssetPopup(DroppedFile& entry, bool& isOpen);

		void SelectFile(DirectoryNode& node);
	};
}

#endif