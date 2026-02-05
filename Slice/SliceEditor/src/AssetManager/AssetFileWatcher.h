/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        FileWatcher.h

 author:	  Lee Yong Yee
 co-author:   Nic Lai

 email:       l.yongyee@digipen.edu

 brief:		  Contains all the declarations for the file watching functions

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef ASSET_FILE_WATCHER_H
#define ASSET_FILE_WATCHER_H

#include <vector>
#include <filesystem>

namespace SliceEditor
{
    class AssetManager;
    class ContentBrowserManager;
    struct RawFileEvent;

    namespace AssetFileWatcher
    {
        void UpdateFolder(ContentBrowserManager& manager, AssetManager& am);

        void HandleAssetAdded(ContentBrowserManager& manager, AssetManager& am, RawFileEvent& addEvent);
        void HandleAssetRemoved(AssetManager& am, RawFileEvent& removeEvent);
        void HandleAssetRenamed(AssetManager& am, RawFileEvent& renamedOld, RawFileEvent& renamedNew);
        void HandleAssetModified(AssetManager& am, RawFileEvent& event);
        void HandleAssetMoved(AssetManager& am, std::vector<RawFileEvent>& events);

        std::optional<uint64_t> HashFile(const std::filesystem::path& filePath);

   
    }
}

#endif