#include <pch.h>
#include "AssetManager.h"


namespace SliceEditor
{
	bool AssetManager::CreateMetaDataFile(MetaData& metaData)
	{
		std::ifstream fileCheck(mDescriptorDirectory.string() + ".meta");
		if (fileCheck.good())
		{
			SLICE_LOG_WARNING("Meta file already exists for asset: ");
			return false;
		}
		fileCheck.close();

		//metaData.path = assetPath;

		metaData.guid = SliceEngine::GUID::Generate();

		//Section for Switch Case for Asset Types

		metaData.assetType = ".txt";

		//For Resource File that is relevent:
		metaData.resourcePath.push_back(std::string("NIL")); // Set this to the actual resource path when applicable

		//WriteMetaDataFile(assetPath + ".meta", metaData);
		return true;
	}
	void AssetManager::WriteMetaDataFile(const std::string& metaFilePath, MetaData& metaData)
	{
		nlohmann::json j;
		j["assetPath"] = metaData.path;
		j["assetGUID"] = metaData.guid.GetGUID();
		j["assetType"] = metaData.assetType;
		for (int i = 0; i < metaData.resourcePath.size(); i++)
		{
			j["resourcePath"].push_back(metaData.resourcePath[i]);
		}
		std::ofstream outFile(metaFilePath);
		if (outFile.is_open())
		{
			outFile << j.dump(4); // Pretty print with 4 spaces indentation
			outFile.close();
			SLICE_LOG("Meta file written: " + metaFilePath);
		}
		else
		{
			SLICE_LOG_ERROR("Failed to write meta file: " + metaFilePath);
		}
	}

	void AssetManager::Init()
	{

		//Searching Descriptor and Assigning to "Assets"
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

		}
		SLICE_LOG("Asset Manager Initialized");
	}

	SliceEngine::GUID ReadGUIDFromDescriptor(std::filesystem::path path)
	{
		auto guid = path.stem();
		
		return SliceEngine::GUID::FromString(guid.string());
	}
}