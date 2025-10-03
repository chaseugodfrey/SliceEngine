#include <pch.h>
#include "AssetManager.h"


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

		if(!std::filesystem::exists(mResourcesDirectory))
		{
			std::filesystem::create_directory(mResourcesDirectory);
		}

		//Searching Descriptor Folder and Assigning to "Assets"
		//Note: Not Sure if we want to sort the descriptors into subfolders based on type as well. Currently all in the same folder.
		for(auto & dirEntry : std::filesystem::recursive_directory_iterator(mDescriptorDirectory))
		{
			nlohmann::json metaData;

			if (dirEntry.is_regular_file())
			{
				std::filesystem::path filePath = dirEntry.path();
				if (filePath.extension() == ".meta")
				{
					SliceEngine::GUID guid = ReadGUIDFromDescriptor(filePath);
					std::string fileName;

					std::ifstream inFile(filePath);
					if (inFile.is_open())
					{
						if (metaData.contains("fileName"))
						{
							fileName = metaData["fileName"];
						}
					}
					inFile.close();

					mDescriptorMap[fileName] = guid;
				}
			}
		}

		//Looping through Assets to see who does not have a descriptor file (very sad. nobody is describing it.)
		for (auto& dirEntry : std::filesystem::recursive_directory_iterator(mAssetDirectory))
		{
			std::string fileName = dirEntry.path().filename().string();

			// If the file does not have a descriptor, create one
			if (mDescriptorMap.find(fileName) == mDescriptorMap.end())
			{
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
		// Generate a new GUID for the asset
		SliceEngine::GUID newGUID = SliceEngine::GUID::Generate();
		// Create metadata
		// Write metadata to the descriptor file
		nlohmann::json metaData;
		metaData["GUID"] = std::to_string(newGUID.GetGUID());
		metaData["fileName"] = filePath.filename().string();
		metaData["fileType"] = filePath.extension().string();
		metaData["importSettings"] = "Import Settings"; // Placeholder for import settings


		// Determine the path for the descriptor file
		std::filesystem::path descriptorPath = mDescriptorDirectory / (std::to_string(newGUID.GetGUID()) + ".meta");
		// Write metadata to the descriptor file
		std::ofstream outFile(descriptorPath);
		if (outFile.is_open())
		{
			outFile << metaData.dump(4); // Pretty print with 4 spaces indentation
			outFile.close();
			SLICE_LOG("Created descriptor for asset: " + filePath.string());
		}
		else
		{
			SLICE_LOG("Failed to create descriptor for asset: " + filePath.string());
		}
		// Update the descriptor map
		mDescriptorMap[filePath.filename().string()] = newGUID;
	}
}