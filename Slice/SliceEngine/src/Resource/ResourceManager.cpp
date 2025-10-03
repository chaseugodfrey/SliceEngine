#include <pch.h>
#include "ResourceManager.h"
#include <fstream>
namespace SliceEngine
{
	void ResourceManager::InitResourceManager()
	{
		// Read the resource folder to store file paths and guids
		for(auto & dirEntry : std::filesystem::recursive_directory_iterator(mResourcesDirectory))
		{
			if (dirEntry.is_regular_file())
			{
				std::filesystem::path filePath = dirEntry.path();
				if (filePath.extension() == ".meta")
				{
					std::ifstream inFile(filePath);

					try
					{
						nlohmann::json metaData;
						inFile >> metaData;
						std::string assetName = metaData["assetName"].get<std::string>();
						uint64_t guid = metaData["assetGUID"].get<uint64_t>();
						std::string assetPath = metaData["assetPath"].get<std::string>();
						// idk what the otehr two things are meant to be
						mGUIDToPath[GUID(guid)] = assetPath;
						mFileNameToGUID[assetName] = GUID(guid);
					}
					catch (nlohmann::json::parse_error& e)
					{
						SLICE_LOG_ERROR("Failed to parse .meta file: {}", e.what());
					}
					/*std::string pathString = filePath.string();
					RegisterFileAsset(pathString);*/
				}

			}
		}
	}
}