#include <pch.h>
#include "AssetManager.h"
#include "../../SliceEngine/src/Resource/ResourceManager.h"
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
		for (auto& dirEntry : std::filesystem::recursive_directory_iterator(mAssetDirectory))
		{
			std::string fileName = dirEntry.path().filename().string();

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
		static const std::map<std::string, AssetType> extensionMap = {
			{".png", AssetType::Texture}, {".jpg", AssetType::Texture}, {".tga", AssetType::Texture},
			{".fbx", AssetType::Model},   {".obj", AssetType::Model},
			{".wav", AssetType::Audio},   {".mp3", AssetType::Audio}
		};

		auto it = extensionMap.find(filePath.extension().string());
		if (it == extensionMap.end())
		{
			SLICE_LOG("Unsupported asset type for file: " + filePath.string());
			return; // Unsupported asset type
		}

		AssetType assetType = it->second;
		std::unique_ptr<MetaData> metaData;

		// I think can compile assets somewhere around here

		switch (assetType)
		{
			case AssetType::Texture:
				metaData = std::make_unique<TextureData>();
				break;
			case AssetType::Model:
				//metaData = std::make_unique<ModelData>();
				break;
			case AssetType::Audio:
				//metaData = std::make_unique<AudioData>();
				break;
		}

		if (metaData)
		{
			metaData->guid = SliceEngine::GUID::Generate();
			metaData->assetName = filePath.filename().string();
			metaData->assetType = filePath.extension().string();
			metaData->assetPath = filePath.string();
			// meta files are gonna be named after guid + meta
			//metaData->resourcePath = std::to_string(metaData->guid.GetGUID()) + ".meta"; nvm this isnt resource


			// serialize the meta file 
			metaData->Serialize(mResourcesDirectory);

			// compile the asset here??
			switch (assetType)
			{
			case AssetType::Texture:

				break;
			case AssetType::Model:
				
				break;
			case AssetType::Audio:
				
				break;
			}

			// register into resource manager


			// Update the descriptor map
			mDescriptorMap[filePath.filename().string()] = metaData->guid;
		}
	}
}