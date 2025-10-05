#include <pch.h>
#include "AssetManager.h"
#include "AssetTypes.h"
namespace SliceEditor
{
	void AssetManager::Init()
	{
		//Sanity Checks for the Directories
		if(!std::filesystem::exists(mAssetDirectory))
		{
			std::filesystem::create_directory(mAssetDirectory);
		}	

		if(!std::filesystem::exists(mDescriptorDirectory))
		{
			std::filesystem::create_directory(mDescriptorDirectory);
		}

		auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();

		//Searching Descriptor Folder and Assigning to "Assets"
		//Looping through Assets to see who does not have a descriptor file (very sad. nobody is describing it.)
		for (auto it = std::filesystem::recursive_directory_iterator(mAssetDirectory);
			it != std::filesystem::recursive_directory_iterator();
			++it)
		{
			auto& dirEntry = *it;

			// TODO: Resource folder shouldn't be in asset folder
			// but fornow we just skip #gonext
			if (dirEntry.is_directory() && dirEntry.path().filename() == "Resources")
			{
				it.disable_recursion_pending();
				continue;
			}

			std::string fileName = dirEntry.path().filename().stem().string();

			// Since this isn't unity style where meta files are alongside assets
			// we need to compare wit hthe file name to GUID from the resource manager
			// which holds the map of names to GUIDs to resource paths
			if (resourceMgr->mFileNameToGUID.find(fileName) == resourceMgr->mFileNameToGUID.end())
			{
				// this file does not have a meta/descriptor file
				// make one ig?
				CreateDescriptorFile(dirEntry.path());
			}
		}
		SLICE_LOG("Asset Manager Initialized");
	}

	SliceEngine::GUID AssetManager::ReadGUIDFromDescriptor(std::filesystem::path path)
	{
		auto guid = path.stem();
		
		return SliceEngine::GUID::FromString(guid.string());
	}

	void AssetManager::CreateDescriptorFile(const std::filesystem::path filePath)
	{
		//Find out the type of asset:
		filePath.extension().string();
		//static const std::map<std::string, AssetType> extensionMap = {
		//	{".png", AssetType::Texture}, {".jpg", AssetType::Texture}, {".tga", AssetType::Texture},
		//	{".fbx", AssetType::Model},   {".obj", AssetType::Model},
		//	{".wav", AssetType::Audio},   {".mp3", AssetType::Audio}
		//};

		auto it = mSupportedAssetTypes.find(filePath.extension().string());
		if (it == mSupportedAssetTypes.end())
		{
			SLICE_LOG("Unsupported asset type for file: " + filePath.string());
			return; // Unsupported asset type
		}

		AssetType assetType = it->second;
		std::unique_ptr<MetaData> metaData;

		// I think can compile assets somewhere around here
		uint64_t typeID = 0;
		switch (assetType)
		{
			case AssetType::Texture:
				metaData = std::make_unique<TextureData>();
				typeID = ResourceTypeIDs::TEXTURE;
				break;
			case AssetType::Model:
				metaData = std::make_unique<ModelData>();
				typeID = ResourceTypeIDs::MODEL;
				break;
			case AssetType::Audio:
				//metaData = std::make_unique<AudioData>();
				break;
			case AssetType::Scene:
				metaData = std::make_unique<SceneData>();
				typeID = ResourceTypeIDs::SCENE;
				break;
		}

		if (metaData)
		{
			metaData->assetName = filePath.stem().string();
			metaData->guid = SliceEngine::GUID::Generate(metaData->assetName, typeID);
			metaData->assetType = filePath.extension().string();
			metaData->assetPath = filePath.string();
			// meta files are gonna be named after guid + meta
			//metaData->resourcePath = std::to_string(metaData->guid.GetGUID()) + ".meta"; nvm this isnt resource

			// used only for cube testing and scenes
			std::string tempPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + metaData->assetType;

			// compile the asset here?? or before creating the meta file?
			switch (assetType)
			{
			case AssetType::Texture:

				break;
			case AssetType::Model:
				std::filesystem::copy(filePath, tempPath);
				break;
			case AssetType::Audio:

				break;
			case AssetType::Scene:
				std::filesystem::copy(filePath, tempPath);
				break;
			}


			// serialize the meta file 
			metaData->Serialize(mResourcesDirectory);


			// register into resource manager
			auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
			resourceMgr->RegisterResourceAsset(metaData->guid, metaData->resourcePath);

			mAssets[assetType].push_back(metaData->assetName);

			// Update the descriptor map
			mDescriptorMap[filePath.filename().string()] = metaData->guid;
		}
	}
}