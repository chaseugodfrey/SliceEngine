/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ResourceManager.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles all resources

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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
					RegisterResourceAsset(filePath.string());
					/*std::string pathString = filePath.string();
					RegisterFileAsset(pathString);*/
				}

			}
		}
	
		mGUIDToResource[(GUID)DefaultResourceIDs::CUBE_DEFAULT] = std::to_string(DefaultResourceIDs::CUBE_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::QUAD_DEFAULT] = std::to_string(DefaultResourceIDs::QUAD_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::FRUSTRUM_DEFAULT] = std::to_string(DefaultResourceIDs::FRUSTRUM_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::LINE_DEFAULT] = std::to_string(DefaultResourceIDs::LINE_DEFAULT);
	}

	void ResourceManager::RegisterResourceAsset(const std::string& path)
	{
		//if (mGUIDToResource.count(guid) != 0)
		//	mGUIDToResource[guid] = path;
		//else
		//	SLICE_LOG_ERROR("REGISTERING REPEATED ASSET");
		std::ifstream inFile(path);

		try
		{
			nlohmann::json metaData;
			inFile >> metaData;
			std::string assetName = metaData["assetName"].get<std::string>();
			uint64_t guid = metaData["guid"].get<uint64_t>();
			std::string assetPath = metaData["assetPath"].get<std::string>();
			std::string resourcePath = metaData["resourcePath"].get<std::string>();
			// idk what the otehr two things are meant to be
			mGUIDToPath[GUID(guid)] = assetPath;
			mGUIDToResource[GUID(guid)] = resourcePath;
			mFileNameToGUID[assetName] = GUID(guid);
		}
		catch (nlohmann::json::parse_error& e)
		{
			const char* errorMessageCStr = e.what();
			SLICE_LOG_ERROR("Failed to parse .meta file: {}" + std::string(errorMessageCStr));
		}

	}


}