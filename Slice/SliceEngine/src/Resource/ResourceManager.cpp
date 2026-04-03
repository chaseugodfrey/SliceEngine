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
#include "Core/Core.h"
namespace SliceEngine
{
	void ResourceManager::InitResourceManager()
	{
		std::filesystem::path manifestPath = mResourcesDirectory / "AssetManifest.json";

		std::ifstream inFile(manifestPath);
		if (!inFile.is_open())
		{
			SLICE_LOG_ERROR("Asset manifest not found at " + manifestPath.string());
		}
		else
		{
			try
			{
				nlohmann::json manifest;
				inFile >> manifest;
				if (manifest.contains("assets") && manifest["assets"].is_array())
				{
					for (const auto& entry : manifest["assets"])
					{
						std::string assetName = entry["name"].get<std::string>();
						uint64_t guidVal = entry["guid"].get<uint64_t>();
						std::string resourcepath = entry["path"].get<std::string>();

						GUID guid(guidVal);
						mFileNameToGUID[assetName] = guid;
						mGUIDToResource[guid] = resourcepath;
					}

					SLICE_LOG("Resource loaded: " + std::to_string(manifest["assets"].size()) + " assets from manifest");
				}
			}
			catch (const nlohmann::json::parse_error& e)
			{
				SLICE_LOG_ERROR(std::string("Failed to parse asset manifest json file") + e.what());
			}

			inFile.close();
		}
	
		mGUIDToResource[(GUID)DefaultResourceIDs::CUBE_DEFAULT] = std::to_string(DefaultResourceIDs::CUBE_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::SPHERE_DEFAULT] = std::to_string(DefaultResourceIDs::SPHERE_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT] = std::to_string(DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::CAPSULE_DEFAULT] = std::to_string(DefaultResourceIDs::CAPSULE_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::CYLINDER_DEFAULT] = std::to_string(DefaultResourceIDs::CYLINDER_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::QUAD_DEFAULT] = std::to_string(DefaultResourceIDs::QUAD_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::PLANE_DEFAULT] = std::to_string(DefaultResourceIDs::PLANE_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::FRUSTRUM_DEFAULT] = std::to_string(DefaultResourceIDs::FRUSTRUM_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::LINE_DEFAULT] = std::to_string(DefaultResourceIDs::LINE_DEFAULT);
		
		mGUIDToResource[(GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT] = std::to_string(DefaultResourceIDs::COLOR_DEADED_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::COLOR_NORMAL_DEFAULT] = std::to_string(DefaultResourceIDs::COLOR_NORMAL_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::FONT_BLANK_DEFAULT] = std::to_string(DefaultResourceIDs::FONT_BLANK_DEFAULT);
		mGUIDToResource[(GUID)DefaultResourceIDs::CSHADER_DEFAULT] = std::to_string(DefaultResourceIDs::CSHADER_DEFAULT);
	
		// Default material
		mGUIDToResource[(GUID)Type<SliceEngineTypes::Material>::defaultResourceGUID] = std::to_string(Type<SliceEngineTypes::Material>::defaultResourceGUID);
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

	void ResourceManager::ReloadResourceInPlace(const GUID& guid)
	{
		auto it = mInstances.find(guid);
		if (it == mInstances.end())
		{
			SLICE_LOG_WARNING("Attempted to reload a resource that does not exist");
			return;
		}

		auto& instance = it->second;

		if (!instance.reload_in_place)
		{
			SLICE_LOG_WARNING("Resource has no reload_in_place function");
			return;
		}

		instance.reload_in_place(instance.data.get(), *this, instance.filePath);
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
			std::string resourcePath = metaData["resourcePath"];
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
		inFile.close();
	}

	/// <summary>
	/// IDK whether we should just write one update loop to check for any changes
	/// in resources
	/// or manually call a function to update the resource when its changed
	/// 
	/// nvm chase said manually update
	/// </summary>
	
	//this func is litearlly not called - elton
	//void ResourceManager::UpdateEntityResources()
	//{


	//	//auto& registry = Core::GetInstance()->GetRegistry();
	//	//auto entityView = registry.view<SliceEntity>();
	//	//for (auto entity : entityView)
	//	//{
	//	//	// if it has renderer component
	//	//	if (registry.any_of<Renderer>(entity))
	//	//	{
	//	//		auto& rend = registry.get<Renderer>(entity);
	//	//		// get the GUID of the current file name
	//	//		GUID currGUID = mFileNameToGUID[rend.modelHandle.fileName];

	//	//		// check if its the same GUID as its set
	//	//		if (currGUID != rend.modelHandle.mGUID)
	//	//		{
	//	//			// if its not the same then reload
	//	//			rend.modelHandle = get<SliceEngineTypes::Model>(rend.modelHandle.mGUID);
	//	//		}
	//	//	}
	//	//}
	//}

	void ResourceManager::ReleaseResource(const GUID& guid)
	{
		////std::cout << "Resource being released " << guid.GetGUID() << " : ";
		//for(const auto& [key, val] : mFileNameToGUID)
		//{
		//	if (val == guid)
		//	{
		//		//std::cout << key << std::endl;
		//	}
		//}
		auto it = mInstances.find(guid);
		if (it != mInstances.end())
		{
			it->second.refCount--;
			if (it->second.refCount <= 0)
			{
				// store it in a local so that itll destrroy itself instead of a nested destroy when calling erase(it)
				auto ToDestroy = std::move(it->second.data);

				// erasing is safer now since the instance is empty after moving
				mInstances.erase(it);
			}
		}
	}

	void ResourceManager::Shutdown()
	{
		std::vector<std::unique_ptr<void, std::function<void(void*)>>> dataToDestroy;
		dataToDestroy.reserve(mInstances.size()); 

		for (auto& pair : mInstances)
		{
			dataToDestroy.push_back(std::move(pair.second.data));
		}
	
		mInstances.clear();
		dataToDestroy.clear();
	}

	GUID ResourceManager::GetSkeletonGUIDFromModel(GUID modelGUID)
	{
		std::string resourceStr = mGUIDToResource[modelGUID];
		std::filesystem::path resourcePath(resourceStr);
		resourcePath.replace_extension(".meta");

		std::ifstream file(resourcePath);

		if (!file.is_open())
		{
			SLICE_LOG_ERROR("Could not open meta file for model");
			return (GUID)0;
		}

		nlohmann::json meta;
		try
		{
			meta = nlohmann::json::parse(file);
		}
		catch (nlohmann::json::parse_error& e)
		{
			SLICE_LOG_ERROR(std::string("Invalid model meta file") + e.what());
			return (GUID)0;
		}

		std::string skeleMeta = meta["skeleMetaPath"].get<std::string>();
		//std::filesystem::path skelePath(skeleMeta);

		size_t lastSlash = skeleMeta.find_last_of('/');
		size_t startPos = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
		size_t dotPos = skeleMeta.find_last_of('.');
		if (dotPos == std::string::npos || dotPos < startPos)
		{
			SLICE_LOG_ERROR("Invalid meta path for skeleton");
			return (GUID)0;
		}

		size_t length = dotPos - startPos;

		std::string GUIDstr = skeleMeta.substr(startPos, length);

		return (GUID)std::stoull(GUIDstr);
	}

	GUID ResourceManager::GetAnimationGUIDFromModel(GUID modelGUID)
	{
		std::string resourceStr = mGUIDToResource[modelGUID];
		std::filesystem::path resourcePath(resourceStr);
		resourcePath.replace_extension(".meta");

		std::ifstream file(resourcePath);

		if (!file.is_open())
		{
			SLICE_LOG_ERROR("Could not open meta file for model");
			return (GUID)0;
		}

		nlohmann::json meta;
		try
		{
			meta = nlohmann::json::parse(file);
		}
		catch (nlohmann::json::parse_error& e)
		{
			SLICE_LOG_ERROR(std::string("Invalid model meta file") + e.what());
			return (GUID)0;
		}

		std::string animMeta = meta["animMetaPath"].get<std::string>();
		//std::filesystem::path skelePath(skeleMeta);

		size_t lastSlash = animMeta.find_last_of('/');
		size_t startPos = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
		size_t dotPos = animMeta.find_last_of('.');
		if (dotPos == std::string::npos || dotPos < startPos)
		{
			SLICE_LOG_ERROR("Invalid meta path for skeleton");
			return (GUID)0;
		}

		size_t length = dotPos - startPos;

		std::string GUIDstr = animMeta.substr(startPos, length);

		return (GUID)std::stoull(GUIDstr);
	}

	bool ResourceManager::CheckResource(GUID guid)
	{
		if (mGUIDToResource.find(guid) != mGUIDToResource.end())
			return true;

		return false;
	}

	std::optional<std::filesystem::path> ResourceManager::GetResourcePath(std::string filename)
	{
		auto it = mFileNameToGUID.find(filename);
		if (it != mFileNameToGUID.end())
		{
			auto it2 = mGUIDToResource.find(it->second);
			if (it2 != mGUIDToResource.end())
			{
				return it2->second;
			}
		}

		return {};
	}
}