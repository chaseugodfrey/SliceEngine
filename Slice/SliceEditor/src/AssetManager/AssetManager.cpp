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
		std::string ext = filePath.extension().string();

		auto it = mSupportedAssetTypes.find(ext);
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
			case AssetType::Shader:
				metaData = std::make_unique<ShaderData>();
				typeID = ResourceTypeIDs::SHADER;
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

			// used only for things that copies over its original asset type (i.e .scene/.shader/.vert/etc
			std::string tempPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + metaData->assetType;

			// compile the asset here?? or before creating the meta file?
			switch (assetType)
			{
			case AssetType::Texture:
				if (ext == ".dds")
				{
					// if its a dds then just copy over
					// if its not then we need to convert it to dds
					std::filesystem::copy(filePath, tempPath);
				}
				else
				{
					// TODO: Add the call to exe to convert to dds
					return;
				}
				break;
			case AssetType::Model:
				std::filesystem::copy(filePath, tempPath);
				break;
			case AssetType::Audio:

				break;
			case AssetType::Scene:
				std::filesystem::copy(filePath, tempPath);
				break;
			case AssetType::Shader:
				std::string fileName = filePath.stem().string();
				std::filesystem::path parentPath = filePath.parent_path();
				std::filesystem::path vertPath = parentPath / (fileName + ".vert");
				std::filesystem::path fragPath = parentPath / (fileName + ".frag");
				std::string tempVertPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".vert";
				std::string tempFragPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".frag";

				// copy the 3 files over to resources
				// cause loading shaders now come in 3s
				// but they have the same name so their guid would end up being the same
				// so only the main .shader file is used for guid generation
				std::filesystem::copy(filePath, tempPath);
				std::filesystem::copy(vertPath, tempVertPath);
				std::filesystem::copy(fragPath, tempFragPath);

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