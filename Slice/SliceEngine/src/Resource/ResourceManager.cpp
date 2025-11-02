/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ResourceManager.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles all resources

Copyright (C) 2025 DigiPen Institute of Technology.
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
		mGUIDToResource[(GUID)DefaultResourceIDs::SPHERE_DEFAULT] = std::to_string(DefaultResourceIDs::SPHERE_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT] = std::to_string(DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::CAPSULE_DEFAULT] = std::to_string(DefaultResourceIDs::CAPSULE_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::QUAD_DEFAULT] = std::to_string(DefaultResourceIDs::QUAD_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::FRUSTRUM_DEFAULT] = std::to_string(DefaultResourceIDs::FRUSTRUM_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::LINE_DEFAULT] = std::to_string(DefaultResourceIDs::LINE_DEFAULT);
	}

	void ResourceManager::ReloadResource(const GUID& guid)
	{
		auto it = mInstances.find(guid);
		if (it == mInstances.end())
		{
			SLICE_LOG_WARNING("Attempted to reload a resource that does not exist");
			return;
		}

		auto& instance = it->second;

		if (!instance.reload)
		{
			SLICE_LOG_WARNING("Resource has no reload function");
			return;
		}

		// recompile the new data
		auto newData = instance.reload(*this, instance.filePath);

		instance.data = std::move(newData);
	}

	void ResourceManager::RegisterResourceAsset(const GUID& guid, const std::string& path)
	{
		mGUIDToResource[guid] = path;
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

	void ResourceManager::ReleaseResource(const GUID& guid)
	{
		std::cout << "Resource being released " << guid.GetGUID() << " : ";
		for(const auto& [key, val] : mFileNameToGUID)
		{
			if (val == guid)
			{
				std::cout << key << std::endl;
			}
		}
		auto it = mInstances.find(guid);
		if (it != mInstances.end())
		{
			it->second.refCount--;
			if (it->second.refCount <= 0)
			{
				mInstances.erase(it);
			}
		}
	}

}