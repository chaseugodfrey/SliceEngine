#include <pch.h>
#include "AssetManager.h"


namespace SliceEditor
{
	bool AssetManager::CreateMetaDataFile(const std::string& assetPath, MetaData& metaData)
	{
		std::ifstream fileCheck(assetPath + ".meta");
		if (fileCheck.good())
		{
			SLICE_LOG_WARNING("Meta file already exists for asset: " + assetPath);
			return false;
		}
		fileCheck.close();

		metaData.path = assetPath;

		metaData.guid = SliceEngine::GUID::Generate();

		//Section for Switch Case for Asset Types

		metaData.assetType = ".txt";

		//For Resource File that is relevent:
		metaData.resourcePath.push_back(std::string("NIL")); // Set this to the actual resource path when applicable

		WriteMetaDataFile(assetPath + ".meta", metaData);
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

	void AssetManager::Init(std::filesystem::path)
	{
		for(auto & dirEntry : std::filesystem::recursive_directory_iterator("../SliceEditor/Assets"))
		{
			if (dirEntry.is_regular_file())
			{
				std::filesystem::path filePath = dirEntry.path();
				if (filePath.extension() != ".meta")
				{
					std::ifstream fileCheck(filePath.string() + ".meta");
					if (!fileCheck.good())
					{
						MetaData metaData;
						CreateMetaDataFile(filePath.string(), metaData);
						//WriteMetaDataFile(filePath.string() + ".meta", metaData);
					}
					fileCheck.close();
				}
			}
		}
		SLICE_LOG("Asset Manager Initialized");
	}
}