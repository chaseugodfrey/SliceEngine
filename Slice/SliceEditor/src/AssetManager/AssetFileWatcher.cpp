/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AssetFileWatcher.cpp

 author:	  Lee Yong Yee

 email:       l.yongyee@digipen.edu

 brief:		  Defines the file watching functions

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "AssetFileWatcher.h"
#include "AssetManager.h"
#include "AssetTypes.h"
#include "Core/Registry.h"
#include <Systems/SceneSystem.h>
#include <ContentBrowser/ContentBrowserManager.h>
#include "../../SliceEngine/src/Configuration/ProjectSettings.h"

namespace SliceEditor
{
    void AssetFileWatcher::UpdateFolder(ContentBrowserManager& manager, AssetManager& am)
    {
        std::vector<RawFileEvent> rawEvents;

        {
            std::lock_guard<std::mutex> lock(am.mEventQueueMutex);
            while (!am.mRawFileQueue.empty())
            {
                rawEvents.push_back(am.mRawFileQueue.front());
                am.mRawFileQueue.pop();
            }
        }

        if (rawEvents.empty())
        {
            return;
        }

        AssetFileChangedEvent processedEvents = { false };

        if (rawEvents.size() > 1)
        {
            RawFileEvent previousAction{};

            // TODO: Change all the events for file watcher to pull the correct meta file now instead of the one in resource folder

            if (rawEvents.begin()->changeType == filewatch::Event::removed && rawEvents.at(1).changeType == filewatch::Event::added)
            {

                HandleAssetMoved(am, rawEvents);
            }
            else if (rawEvents.begin()->changeType == filewatch::Event::renamed_old && rawEvents.at(1).changeType == filewatch::Event::renamed_new)
            {

                HandleAssetRenamed(am, rawEvents.at(0), rawEvents.at(1));

            }
            else if (rawEvents.begin()->changeType == rawEvents.at(1).changeType && rawEvents.begin()->filePath == rawEvents.at(1).filePath)
            {
                HandleAssetModified(am, rawEvents.at(0));
                //SLICE_LOG("Modifying file: " + rawEvents.begin()->filePath.string());
            }
            else
            {
                for (auto& eventType : rawEvents)
                {
                    if (eventType.changeType == filewatch::Event::added)
                    {
                        if (!am.mFilenameToGUID.contains(eventType.filePath.stem().string()))
                        {
                            HandleAssetAdded(manager, am, eventType);
                            previousAction = eventType;
                        }
                    }
                    else if (eventType.changeType == filewatch::Event::removed)
                    {
                        if (am.mFilenameToGUID.contains(eventType.filePath.stem().string()))
                        {
                            HandleAssetRemoved(am, eventType);
                            previousAction = eventType;
                        }
                    }
                    else if (eventType.changeType == filewatch::Event::modified)
                    {
                        //if(previousAction.changeType == filewatch::Event::removed && previousAction.change)
                        if (am.mFilenameToGUID.contains(eventType.filePath.stem().string()))
                        {
                            HandleAssetModified(am, eventType);
                            previousAction = eventType;
                        }
                    }
                    else if (eventType.changeType == filewatch::Event::renamed_old)
                    {
                        previousAction = eventType;
                        continue;
                    }
                    else if (eventType.changeType == filewatch::Event::renamed_new)
                    {
                        if (previousAction.changeType == filewatch::Event::renamed_old)
                        {
                            HandleAssetRenamed(am, previousAction, eventType);
                        }
                    }
                }
            }

        }
        else
        {
            switch (rawEvents.begin()->changeType)
            {
            case filewatch::Event::added:
            {
                HandleAssetAdded(manager, am, rawEvents.at(0));

                break;
            }
            case filewatch::Event::removed:
            {

                HandleAssetRemoved(am, rawEvents.at(0));
                break;
            }
            case filewatch::Event::modified:
            {
                HandleAssetModified(am, rawEvents.at(0));
                break;
            }
            }
        }
    }

    void AssetFileWatcher::HandleAssetAdded(ContentBrowserManager& manager, AssetManager& am, RawFileEvent& addEvent)
    {
        if (!manager.mPendingDrops.empty() && !manager.mActiveDrop)
        {
            manager.mActiveDrop = std::move(manager.mPendingDrops.front());
        }

        if (manager.mActiveDrop.has_value())
        {
            return;
        }

        for (auto& [key, value] : am.mSupportedAssetTypes)
        {
            if (addEvent.filePath.extension() == key)
            {
                if (value.first == AssetType::Texture || value.first == AssetType::Model)
                {
                    return;
                }
            }
        }

        //if (addEvent.filePath.extension() == ".temp")
        //{
        //    SliceEngine::GUID sceneGUID = am.mFilenameToGUID[addEvent.filePath.stem().string()];
        //    std::string guidFilename = std::to_string(sceneGUID.GetGUID()) + ".temp";
        //    std::filesystem::path destPath = am.mAssetDirectory.parent_path() / "Resources" / guidFilename;

        //    try {
        //        std::filesystem::copy_file(addEvent.filePath, destPath, std::filesystem::copy_options::overwrite_existing);
        //    }
        //    catch (const std::filesystem::filesystem_error& e) {
        //        SLICE_LOG_ERROR("Failed to update Temp file in resources: " + std::string(e.what()));
        //    }
        //}
        //else
        {
            //Get the name of the asset
            std::string originalFileName = addEvent.filePath.stem().string();
            //Get the extension of the asset
            std::string originalExt = addEvent.filePath.extension().string();
            std::string parentDirectory = addEvent.filePath.parent_path().filename().string();

            std::string assetName = std::filesystem::relative(addEvent.filePath, am.mAssetDirectory).generic_string();

            //std::string assetName = parentDirectory + "/" + originalFileName + originalExt;

            int nameCount = 0;

            if (originalExt != ".bin" && originalExt != ".navmesh")
            {
                if (am.mFilenameToGUID.contains(assetName))
                {

                    while (am.mFilenameToGUID.contains(assetName))
                    {
                        nameCount++;
                        std::filesystem::path p = std::filesystem::relative(addEvent.filePath, am.mAssetDirectory);
                        std::string newFilename = originalFileName + "_" + std::to_string(nameCount) + originalExt;
                        p.replace_filename(newFilename);
                        assetName = p.generic_string();
                        //assetName = parentDirectory + "/" + originalFileName + "_" + std::to_string(nameCount) + originalExt;
                    }

                    std::filesystem::path newAssetFileName(assetName);

                    addEvent.filePath.replace_filename(newAssetFileName.filename());

                }

            }



            if (ImGui::BeginDragDropSource())
            {
                return;
            }

            // check if the resource already exist if a new resource is added.
            // mostly for prefabs cause creating a prefab calls create resource
            // i rlly hope this doesnt break something else liike before
            // if it does then god bless
            std::string metaPath = assetName + ".meta";
            bool resourceExist = false;
            // if the meta file exist
            // then check if the resource exist

            std::string thisAssetName = parentDirectory + "/" + originalFileName + "_" + std::to_string(nameCount) + originalExt;

            if (std::filesystem::exists(metaPath))
            {
                std::unique_ptr<MetaData> metaData = am.CreateDefaultMeta(thisAssetName);
				metaData->Deserialize(metaPath);
				if (std::filesystem::exists(metaData->resourcePath))
                {
                    AssetExistEvent assetEvent(thisAssetName);
                    EventManager::GetInstance()->Publish<AssetExistEvent>(assetEvent);
                    resourceExist = true;
                }
            }

            if (!resourceExist)
                am.CreateResource(addEvent.filePath, nullptr, true);
        }
        am.CreateAssetMaps();
        AssetFileChangedEvent processEvent = { true };
        EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);

    }

    void AssetFileWatcher::HandleAssetRemoved(AssetManager& am, RawFileEvent& removeEvent)
    {
        std::filesystem::path removedFilePath(removeEvent.filePath);
        //if (removedFilePath.extension() == ".temp")
        //{
        //    return;
        //}

        //std::string parentDirectory = removedFilePath.parent_path().filename().string();

        std::string assetPath = std::filesystem::relative(removedFilePath, am.mAssetDirectory).generic_string();

        auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
        //std::string assetPath = parentDirectory + "/" + removedFilePath.filename().string();
        auto path = resourceMgr->GetResourcePath(assetPath);

        if (path.has_value())
        {
            try
            {
                std::filesystem::path metaFilePath = am.GetMetaDataFromFilename(assetPath);
                SliceEngine::GUID fileGUID = SliceEngine::GUID::FromString(path.value().stem().string());

                resourceMgr->ReleaseResource(fileGUID);
                am.mGUIDtoFilename.erase(fileGUID);
                resourceMgr->mFileNameToGUID.erase(removedFilePath.stem().string());
                am.mFilenameToGUID.erase(removedFilePath.stem().string());

                std::filesystem::remove(metaFilePath);
                std::filesystem::remove(path.value());

                AssetFileChangedEvent processEvent = { true };
                EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);
            }
            catch (const std::exception& e)
            {
                SLICE_LOG_ERROR("Failed to remove resource: " + std::string(e.what()));
            }
        }

        am.CreateAssetMaps();
    }

    void AssetFileWatcher::HandleAssetRenamed(AssetManager& am, RawFileEvent& renamedOld, RawFileEvent& renamedNew)
    {
        std::filesystem::path oldFilePath(renamedOld.filePath);
        std::filesystem::path newFilePath(renamedNew.filePath);

        // Ensure we are dealing with the same file type
        if (oldFilePath.extension() != newFilePath.extension()) return;

        std::string extension = oldFilePath.extension().string();
        //std::string parentDirectory = oldFilePath.parent_path().filename().string();

        auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
        /*std::string oldAssetName = parentDirectory + "/" + oldFilePath.filename().string();
        std::string newAssetName = parentDirectory + "/" + newFilePath.filename().string();*/

        std::string oldAssetName = std::filesystem::relative(oldFilePath, am.mAssetDirectory).generic_string();
        std::string newAssetName = std::filesystem::relative(newFilePath, am.mAssetDirectory).generic_string();

        auto path = resourceMgr->GetResourcePath(oldAssetName);

        if (path.has_value())
        {
            std::filesystem::path oldMetaPath = am.GetMetaDataFromFilename(oldAssetName);

            try
            {
                if (std::filesystem::exists(oldMetaPath))
                {
                    std::ifstream inFile(oldMetaPath);
                    nlohmann::json metaJson;
                    inFile >> metaJson;
                    inFile.close();

                    metaJson["assetName"] = newAssetName;
                    metaJson["assetPath"] = newFilePath.string();

                    std::ofstream outFile(oldMetaPath);
                    outFile << metaJson.dump(4);
                    outFile.close();

                    // 2. Physically rename the .meta file
                    std::filesystem::path newMetaPath = oldMetaPath;
                    newMetaPath.replace_filename(newFilePath.filename().string() + ".meta");
                    std::filesystem::rename(oldMetaPath, newMetaPath);
                }

                SliceEngine::GUID fileGUID = SliceEngine::GUID::FromString(path.value().stem().string());

                am.mGUIDtoFilename[fileGUID] = newAssetName;
                am.mFilenameToGUID.erase(oldAssetName);
                am.mFilenameToGUID[newAssetName] = fileGUID;

                resourceMgr->mFileNameToGUID.erase(oldAssetName);
                resourceMgr->mFileNameToGUID[newAssetName] = fileGUID;

                am.CreateAssetMaps();

                AssetFileChangedEvent processEvent = { true };
                EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);
            }
            catch (const std::exception& e)
            {
                SLICE_LOG_ERROR("Failed to rename meta or update maps: " + std::string(e.what()));
            }
        }

    }
    void AssetFileWatcher::HandleAssetModified(AssetManager& am, RawFileEvent& event)
    {
        std::filesystem::path modifiedFilePath(event.filePath);
        auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();



        if (modifiedFilePath.extension() == ".resource")
        {
            //return for now
            return;
        }

        //std::string parentDirectory = modifiedFilePath.parent_path().filename().string();


        SliceEngine::GUID fileGUID;

        //std::string assetPath = parentDirectory + "/" + modifiedFilePath.filename().string();
        std::string assetPath = std::filesystem::relative(modifiedFilePath, am.mAssetDirectory).generic_string();
        auto path = resourceMgr->GetResourcePath(assetPath);
        // NOTE: meta file no longer in resource path

        if (path.has_value())
        {
            auto hashA = HashFile(modifiedFilePath);
            auto hashB = HashFile(path.value());

            if (hashA && hashB)
            {
                if (hashA.value() == hashB.value())
                {
                    SLICE_LOG("Files are the same");
                    //Dont do anything
                    return;
                }
                else
                {
                    try
                    {


                        std::string guidString = path.value().stem().string();
                        fileGUID = SliceEngine::GUID::FromString(guidString);

                        std::filesystem::path metaFilePath = am.GetMetaDataFromFilename(assetPath);
                        std::unique_ptr<MetaData> metaData = am.CreateDefaultMeta(modifiedFilePath);

                        if (metaData)
                        {
                            metaData->Deserialize(metaFilePath);


                            am.CreateResource(modifiedFilePath, metaData.get(), true, true);

                            if (resourceMgr->CheckResource(fileGUID))
                            {

                                resourceMgr->ReloadResourceInPlace(fileGUID);

                            }

                            //Should be moved 
                            /*if (modifiedFilePath.extension() == ".prefab")
                            {
                                EventManager::GetInstance()->Publish<OnPrefabModifiedEvent>(fileGUID);
                            }*/

                            SLICE_LOG("Modified and Hot-Reloaded: " + assetPath);
                        }



                    }
                    catch (const std::exception& e)
                    {
                        SLICE_LOG_ERROR("Failed to update resource file for " + modifiedFilePath.filename().string() + ": " + e.what());
                    }
                }

            }
            else
            {
                SLICE_LOG_ERROR("Could not read hash files");
            }
        }
    }
    void AssetFileWatcher::HandleAssetMoved(AssetManager& am, std::vector<RawFileEvent>& events)
    {
        std::string fileName = events.begin()->filePath.stem().string();


        if (fileName == events.at(1).filePath.stem().string())
        {
            std::string oldAssetName = std::filesystem::relative(events.begin()->filePath, am.mAssetDirectory).generic_string();

            // Accessing the map from AssetManager


            auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
            //std::string oldAssetName = oldParentDirectory + "/" + events.begin()->filePath.filename().string();
            auto path = resourceMgr->GetResourcePath(oldAssetName);
            // NOTE: meta file no longer in resource path
            if (path.has_value())
            {
                std::filesystem::path oldMetaPath = am.GetMetaDataFromFilename(oldAssetName);
                //metaFilePath.replace_extension(".meta");

                if (std::filesystem::exists(oldMetaPath))
                {
                    try
                    {
                        std::filesystem::path newAssetPath;
                        for (auto const& ev : events)
                        {
                            if (ev.changeType == filewatch::Event::added)
                            {
                                newAssetPath = ev.filePath;
                                break;
                            }
                        }

                        if (newAssetPath.empty()) return;

                        //std::string newParentDirectory = newAssetPath.parent_path().string();
                        std::string newAssetNameStr = std::filesystem::relative(newAssetPath, am.mAssetDirectory).generic_string();

                        std::filesystem::path newMetaPath = newAssetPath;
                        newMetaPath += ".meta";


                        std::filesystem::rename(oldMetaPath, newMetaPath);

                        std::ifstream inFile(newMetaPath);
                        nlohmann::json metaJson;
                        inFile >> metaJson;
                        inFile.close();

                       // std::string newAssetNameStr = newParentDirectory + "/" + newAssetPath.filename().string();
                        metaJson["assetName"] = newAssetNameStr;
                        metaJson["assetPath"] = newAssetPath.string();


                        std::ofstream outFile(newMetaPath);
                        outFile << metaJson.dump(4); // 4 spaces for pretty printing
                        outFile.close();

                        SliceEngine::GUID fileGUID = SliceEngine::GUID::FromString(path.value().stem().string());

                        am.mGUIDtoFilename[fileGUID] = newAssetNameStr;
                        am.mFilenameToGUID.erase(oldAssetName);
                        am.mFilenameToGUID.insert({ newAssetNameStr, fileGUID });

                        resourceMgr->mFileNameToGUID.erase(oldAssetName);
                        resourceMgr->mFileNameToGUID.insert({ newAssetNameStr, fileGUID });

                        SLICE_LOG("Updated meta file for moved asset: " + fileName);


                        AssetFileChangedEvent processEvent = { true };
                        EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);
                    }
                    catch (const std::exception& e)
                    {
                        SLICE_LOG_ERROR("Failed to update meta file for " + fileName + ": " + e.what());
                    }
                }
            }
            else
            {
                SLICE_LOG_WARNING("Could not find resource path for moved file: " + fileName);
            }


        }
    }

    std::optional<uint64_t> AssetFileWatcher::HashFile(const std::filesystem::path& filePath)
    {
        if (!std::filesystem::exists(filePath))
        {
            return std::nullopt;
        }

        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
        {
            return std::nullopt;
        }

        const size_t kBufferSize = 4096;
        std::array<char, kBufferSize> buffer;


        uint64_t hash = SliceEngine::FNVHash::OffsetBasis;


        while (file.read(buffer.data(), buffer.size()).gcount() > 0)
        {
            std::streamsize bytesRead = file.gcount();


            for (std::streamsize i = 0; i < bytesRead; ++i)
            {
                hash = (hash ^ static_cast<uint64_t>(static_cast<unsigned char>(buffer[i]))) * SliceEngine::FNVHash::Prime;
            }
        }

        return hash;
    }
}