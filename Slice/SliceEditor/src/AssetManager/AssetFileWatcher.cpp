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
#include "../../SliceEngine/src/Configuration/ProjectSettings.h"

namespace SliceEditor
{
    void AssetFileWatcher::UpdateFolder(AssetManager& am)
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
                            HandleAssetAdded(am,eventType);
                            previousAction = eventType;
                        }
                    }
                    else if (eventType.changeType == filewatch::Event::removed)
                    {
                        if (am.mFilenameToGUID.contains(eventType.filePath.stem().string()))
                        {
                            HandleAssetRemoved(am,eventType);
                            previousAction = eventType;
                        }
                    }
                    else if (eventType.changeType == filewatch::Event::modified)
                    {
                        //if(previousAction.changeType == filewatch::Event::removed && previousAction.change)
                        if (am.mFilenameToGUID.contains(eventType.filePath.stem().string()))
                        {
                            HandleAssetModified(am,eventType);
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
                HandleAssetAdded(am, rawEvents.at(0));

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

    void AssetFileWatcher::HandleAssetAdded(AssetManager& am, RawFileEvent& addEvent)
	{
        for (auto& [key, value] : am.mSupportedAssetTypes)
        {
            if (addEvent.filePath.extension() == key)
            {
                if (value.first == AssetType::Texture || value.first == AssetType::Model ||
                    value.first == AssetType::Controller || value.first == AssetType::Material)
                {
                    return;
                }
            }
        }

        if (addEvent.filePath.extension() == ".temp")
        {
            SliceEngine::GUID sceneGUID = am.mFilenameToGUID[addEvent.filePath.stem().string()];
            std::string guidFilename = std::to_string(sceneGUID.GetGUID()) + ".temp";
            std::filesystem::path destPath = am.mAssetDirectory.parent_path() / "Resources" / guidFilename;

            try {
                std::filesystem::copy_file(addEvent.filePath, destPath, std::filesystem::copy_options::overwrite_existing);
            }
            catch (const std::filesystem::filesystem_error& e) {
                SLICE_LOG_ERROR("Failed to update Temp file in resources: " + std::string(e.what()));
            }
        }
        else
        {
            am.CreateResource(addEvent.filePath, nullptr, true);

            if (addEvent.filePath.extension() == ".navmesh")
            {
                auto sScene = SliceEngine::Core::GetInstance()->GetSceneSystem();
                std::filesystem::path metaFilePath = am.GetMetaDataFromFilename(sScene->GetCurrentSceneName());

                std::ifstream inFile(metaFilePath);
                nlohmann::json metaJson;
                if (inFile >> metaJson) {
                    inFile.close();
                    auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
                    auto navMeshPath = resourceMgr->GetResourcePath(addEvent.filePath.stem().string());

                    if (navMeshPath.has_value()) {
                        metaJson["navMeshFile"] = navMeshPath.value();
                        metaJson["navMeshGUID"] = SliceEngine::GUID::FromString(navMeshPath.value().stem().string());
                        std::ofstream outFile(metaFilePath);
                        outFile << metaJson.dump(4);
                        outFile.close();
                    }
                }
            }
        }
        am.CreateAssetMaps();
        AssetFileChangedEvent processEvent = { true };
        EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);
    
	}

	void AssetFileWatcher::HandleAssetRemoved(AssetManager& am, RawFileEvent& removeEvent)
	{
        std::filesystem::path removedFilePath(removeEvent.filePath);
        if (removedFilePath.extension() == ".temp")
        {
            return;
        }

        std::string parentDirectory = removedFilePath.parent_path().filename().string();
        

        auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
        std::string assetPath = parentDirectory + "/" + removedFilePath.filename().string();
        auto path = resourceMgr->GetResourcePath(assetPath);

        if (path.has_value()) 
        {
            try 
            {
                std::filesystem::path metaFilePath = am.GetMetaDataFromFilename(assetPath);
                SliceEngine::GUID fileGUID = SliceEngine::GUID::FromString(path.value().stem().string());

                resourceMgr->ReleaseResource(fileGUID);
                am.mGUIDtoFilename.erase(fileGUID);
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

        if (oldFilePath.parent_path() == newFilePath.parent_path() && oldFilePath.extension() == newFilePath.extension())
        {
            //if (oldFilePath.extension() == ".scene")
            //{
            //    auto sScene = SliceEngine::Core::GetInstance()->GetSceneSystem();
            //    if (oldFilePath.stem() == sScene->GetDefaultScenePath().stem()) {
            //        sScene->SetDefaultScenePath(newFilePath);
            //        auto gSettings = SliceEngine::Core::GetInstance()->GetProjectSettingsService();
            //        auto& s = gSettings->Edit();
            //        for (auto& it : s.scenes) {
            //            if (std::filesystem::path(it).stem() == oldFilePath.stem()) it = newFilePath.string();
            //        }
            //        s.startupScene = newFilePath.string();
            //        gSettings->Save();
            //    }
            //}

            std::string parentDirectory;
            std::string extension = oldFilePath.extension().string();

            // Accessing the map from AssetManager
            auto it = am.mSupportedAssetTypes.find(extension);
            if (it != am.mSupportedAssetTypes.end()) 
            {
                parentDirectory = it->second.second;
            }

            auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
            std::string oldAssetName = parentDirectory + "/" + oldFilePath.filename().string();
            auto path = resourceMgr->GetResourcePath(oldAssetName);

            if (path.has_value()) 
            {

                std::filesystem::path metaFilePath = am.GetMetaDataFromFilename(oldAssetName);

                try 
                {
                    std::ifstream inFile(metaFilePath);
                    nlohmann::json metaJson;
                    inFile >> metaJson;
                    inFile.close();

                    std::string newAssetName = parentDirectory + "/" + newFilePath.filename().string();

                    metaJson["assetName"] = newAssetName;
                    metaJson["assetPath"] = newFilePath.string();

                    std::ofstream outFile(metaFilePath);
                    outFile << metaJson.dump(4);
                    outFile.close();

                    SliceEngine::GUID fileGUID = SliceEngine::GUID::FromString(path.value().stem().string());
                    /*Update these maps with the new filename
                    mGUIDtoFilename
                    mFilenameToGUID
                    mFileNameToGUID
                    */

                    am.mGUIDtoFilename[fileGUID] = newAssetName;
                    am.mFilenameToGUID.erase(oldAssetName);
                    am.mFilenameToGUID.insert({ newAssetName,fileGUID });
                    resourceMgr->mFileNameToGUID.erase(oldAssetName);
                    resourceMgr->mFileNameToGUID.insert({newAssetName ,fileGUID});

                    std::filesystem::path newMetaPath = metaFilePath;
                    newMetaPath.replace_filename(newFilePath.filename().string() + ".meta");

                    std::filesystem::rename(metaFilePath, newMetaPath);

                    am.CreateAssetMaps();
                    AssetFileChangedEvent processEvent = { true };
                    EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);
                }
                catch (const std::exception& e) 
                {
                    SLICE_LOG_ERROR("Failed to rename meta: " + std::string(e.what()));
                }
            }
        }

	}
	void AssetFileWatcher::HandleAssetModified(AssetManager& am, RawFileEvent& event)
	{
        std::filesystem::path modifiedFilePath(event.filePath);

       /* if (modifiedFilePath.extension() == ".temp")
        {
            auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
            SliceEngine::GUID sceneGUID = am.mFilenameToGUID[modifiedFilePath.stem().string()];

            std::string guidFilename = std::to_string(sceneGUID.GetGUID()) + ".temp";
            std::filesystem::path destPath = am.mResourcesDirectory / guidFilename;

            try
            {
                std::filesystem::copy_file(modifiedFilePath, destPath, std::filesystem::copy_options::overwrite_existing);

            }
            catch (const std::filesystem::filesystem_error& e)
            {
                SLICE_LOG_ERROR("Failed to update Temp file in resources: " + std::string(e.what()));
            }


            return;
        }*/

        if (modifiedFilePath.extension() == ".resource")
        {
            //return for now
            return;
        }

        std::string parentDirectory = modifiedFilePath.parent_path().filename().string();
        

        SliceEngine::GUID fileGUID;

        auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
        std::string assetPath = parentDirectory + "/" + modifiedFilePath.filename().string();
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

                            
                            am.CreateResource(modifiedFilePath, metaData.get(), true);

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
            std::string oldParentDirectory = events.begin()->filePath.parent_path().filename().string();

            // Accessing the map from AssetManager
            

            auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
            std::string oldAssetName = oldParentDirectory + "/" + events.begin()->filePath.filename().string();
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

                        std::string newParentDirectory = newAssetPath.parent_path().filename().string();
                        
                        std::filesystem::path newMetaPath = newAssetPath;
                        newMetaPath += ".meta";

                        
                        std::filesystem::rename(oldMetaPath, newMetaPath);

                        std::ifstream inFile(newMetaPath);
                        nlohmann::json metaJson;
                        inFile >> metaJson;
                        inFile.close();

                        std::string newAssetNameStr = newParentDirectory + "/" + newAssetPath.filename().string();
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