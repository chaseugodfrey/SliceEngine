/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AssetManager.cpp

 author:	  Gideon Nicholas Francis
 co-author:   Nic Lai

 email:       g.francis@digipen.edu

 brief:		  Defines the AssetManager class which handles all asset related operations on the editor side.
			  Creates and manages asset descriptor files, GUIDs, and asset listings.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "AssetManager.h"
#include "AssetTypes.h"
#include <Serializer/JSONSerializer.h>
namespace SliceEditor
{
	void AssetManager::Init()
	{
		//Sanity Checks for the Directories
		if (!std::filesystem::exists(mAssetDirectory))
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

			std::string fileName = dirEntry.path().filename().stem().stem().string();

			// Since this isn't unity style where meta files are alongside assets
			// we need to compare wit hthe file name to GUID from the resource manager
			// which holds the map of names to GUIDs to resource paths
			if (mDescriptorMap.find(fileName) == mDescriptorMap.end())
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

	std::string AssetManager::CreateDescriptorFile(const std::filesystem::path filePath)
	{
		//Find out the type of asset:
		std::string ext = filePath.extension().string();

		auto it = mSupportedAssetTypes.find(ext);
		if (it == mSupportedAssetTypes.end())
		{
			SLICE_LOG("Unsupported asset type for file: " + filePath.string());
			return "";
		}

		AssetType assetType = it->second.first;
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
		case AssetType::Prefab:
			metaData = std::make_unique<PrefabData>();
			typeID = ResourceTypeIDs::PREFAB;
			break;
		}

		if (metaData)
		{
			metaData->assetName = filePath.stem().string();
			metaData->guid = SliceEngine::GUID::Generate(metaData->assetName, typeID);
			metaData->assetType = mAssetExtensions[assetType];
			metaData->assetPath = filePath.string();
			metaData->resourcePath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + metaData->assetType;

			// for rainne's old models and stuff idk
			if (ext == ".rainne")
			{
				metaData->resourcePath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".rainne";
			}
			
			
			/*
				meta file breakdown

				assetName = file path name without extension
				guid
				asset type = compiled asset extension
				asset path = original asset path in asset folder
				resource path = path within the resource folder 
			*/
			
			// used only for things that copies over its original asset type (i.e .scene/.shader/.vert/etc
			std::string tempPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + metaData->assetType;
			
			// get the file path to the meta file
			std::filesystem::path metaPath = metaData->Serialize(mResourcesDirectory);

			// compile the asset here?? or before creating the meta file?
			switch (assetType)
			{
			case AssetType::Texture:
				// This should create the texture asset into the resource folder
				CompileTextureAsset(metaPath);
				break;
			case AssetType::Model:
				// Compile the model file and write into the resource folder
				if (ext == ".rainne")
				{
					// cause rainne is still using manual vertice fbx files
					// i renamed them to .rainne
					// and ill just copy it over instead
					try
					{
						std::filesystem::copy(metaData->assetPath, metaData->resourcePath);
					}
					catch (std::filesystem::filesystem_error& e)
					{
						SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
						//return;
					}
				}
				else
				{
					CompileFBXAsset(metaPath);
				}
				break;
			case AssetType::Audio:
				// idk audio yet
				break;
				// prefab and scene is the same just copy it over
			case AssetType::Prefab:
			case AssetType::Scene:
				CompileSceneAsset(static_cast<SceneData*>(metaData.get()));
				break;
			case AssetType::Shader:
				CompileShaderAsset(static_cast<ShaderData*>(metaData.get()));
				break;
			}


			// should we check if the compiled asset worked?
			// cause if not we should delete the meta file created
			if (!std::filesystem::exists(metaData->resourcePath))
			{
				SLICE_LOG_ERROR("Unable to compile asset :" + metaData->assetName + " at " + metaData->assetPath);
				// delete the meta file if it didn't compile properly
				std::filesystem::remove(metaPath);

				return "";
			}


			// register into resource manager
			//auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
			//resourceMgr->RegisterResourceAsset(metaData->guid, metaData->resourcePath);

			mAssets[assetType].push_back(metaData->assetName);

			// Update the descriptor map
			mDescriptorMap[filePath.filename().string()] = metaData->guid.GetGUID();
		
			return  metaData->resourcePath;
		}
	}

	void AssetManager::CompileTextureAsset(std::filesystem::path const& desc_file) {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		//std::filesystem::path rel_Path = std::filesystem::relative(desc_file, compiler_dir);

		std::wstring cmd = desc_file.wstring();
		std::filesystem::path compiler = "TextureCompile.exe";
		// Start the child process. 
		if (!CreateProcess(compiler.wstring().c_str(),   // No module name (use command line)
			cmd.data(),        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi)           // Pointer to PROCESS_INFORMATION structure
			)
		{
			printf("CreateProcess failed (%d).\n", GetLastError());
			return;
		}

		// Wait until child process exits.
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	void AssetManager::CompileFBXAsset(std::filesystem::path const& desc_file)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		//std::filesystem::path rel_Path = std::filesystem::relative(desc_file, compiler_dir);

		std::wstring cmd = desc_file.wstring();
		std::filesystem::path compiler = "FBX_Compile.exe";
		// Start the child process. 
		if (!CreateProcess(compiler.wstring().c_str(),   // No module name (use command line)
			cmd.data(),        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi)           // Pointer to PROCESS_INFORMATION structure
			)
		{
			printf("CreateProcess failed (%d).\n", GetLastError());
			return;
		}

		// Wait until child process exits.
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	void AssetManager::CompileShaderAsset(ShaderData* metaData)
	{
		std::string fileName = metaData->assetName;
		std::filesystem::path filePath(metaData->assetPath);
		std::filesystem::path parentPath = filePath.parent_path();
		// get the vert and frag path
		std::filesystem::path vertPath = parentPath / (fileName + ".vert");
		std::filesystem::path fragPath = parentPath / (fileName + ".frag");
		// get the destination path for all 2 files
		std::string tempVertPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".vert";
		std::string tempFragPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".frag";

		// copy the 3 files over to resources
		// cause loading shaders now come in 3s
		// but they have the same name so their guid would end up being the same
		// so only the main .shader file is used for guid generation
		try
		{
			std::filesystem::copy(filePath, metaData->resourcePath);
			std::filesystem::copy(vertPath, tempVertPath);
			std::filesystem::copy(fragPath, tempFragPath);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
			//return;
		}

	}

	void AssetManager::CompileSceneAsset(SceneData* metaData)
	{
		std::filesystem::path filePath(metaData->assetPath);

		try
		{
			std::filesystem::copy(filePath, metaData->resourcePath);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
			//return;
		}
	}

	void AssetManager::ScanResourceFolder()
	{
		for (auto& dirEntry : std::filesystem::recursive_directory_iterator(mResourcesDirectory))
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
						uint64_t guid = metaData["guid"].get<uint64_t>();
						std::string fileType = metaData["assetType"];
						std::string assetPath = metaData["assetPath"].get<std::string>();
						std::string resourcePath = metaData["resourcePath"].get<std::string>();

						// if its a shader file just delete that shit
						// cause we got no file watcher to check if a shader was modified
						// so we just delete them and recompile everytime its ran
						if (fileType == ".shader")
						{
							inFile.close();
							// and remove the meta file
							std::filesystem::remove(filePath);

							if (std::filesystem::exists(resourcePath))
							{
								std::filesystem::remove(resourcePath);
							}

							std::filesystem::path resourceFilePath = resourcePath;

							resourceFilePath.replace_extension(".vert");

							if (std::filesystem::exists(resourceFilePath))
								std::filesystem::remove(resourceFilePath);
							resourceFilePath.replace_extension(".frag");

							if (std::filesystem::exists(resourceFilePath))
								std::filesystem::remove(resourceFilePath);

							//return;
						}
						// now check both asset path and resource path
						// if both exist then the asset is fine
						if (!std::filesystem::exists(assetPath) || !std::filesystem::exists(resourcePath))
						{
							// if either of them dont exist
							// then delete the meta and which ever doesn't exist
		/*					if (std::filesystem::exists(assetPath))
							{
							// original asset shouldn't ever be removed oops
								std::filesystem::remove(assetPath);
							}*/

							if (std::filesystem::exists(resourcePath))
							{
								std::filesystem::remove(resourcePath);
							}

							// close the ifstream before removing meta file
							inFile.close();
							// and remove the meta file
							std::filesystem::remove(filePath);

							continue;
							// note: for shaders since it comes in a set of 3 files
							// i dont rlly know how to clean that up
						}
					
						// Check if the asset file was modified after meta file creation
						std::filesystem::file_time_type assetTime = std::filesystem::last_write_time(assetPath);
						std::filesystem::file_time_type resourceTime = std::filesystem::last_write_time(resourcePath);
						
						
						// compare these two
						if (resourceTime < assetTime)
						{
							// Resource file is older than asset file, so recompile the resource
							// close the ifstream before removing meta file
							inFile.close();
							// and remove the meta file
							std::filesystem::remove(filePath);
							if (std::filesystem::exists(resourcePath))
							{
								std::filesystem::remove(resourcePath);
							}

							// idk about shaders

							continue;
						}

						mDescriptorMap.insert_or_assign(assetName, guid);
					}
					catch (nlohmann::json::parse_error& e)
					{
						const char* errorMessageCStr = e.what();
						SLICE_LOG_ERROR("Failed to parse .meta file: {}" + std::string(errorMessageCStr));
					}
				}
			}
		}
	}

	void AssetManager::CreatePrefab(SliceEngine::GameObject GO)
	{

		// idk if this will work yet cause i need it implemented in the editor to test
		// but this should create the prefab and compile it to create the resource as well 

		// Create the prefab file
		std::string path = SliceEngine::JSONSerializer::SerializePrefab(GO.GetEntity());
		std::filesystem::path filePath(path);
		// Create the descriptor
		std::string resourcePath = CreateDescriptorFile(filePath);

		auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
		resourceMgr->RegisterResourceAsset(resourcePath);

	}

	//std::string AssetManager::TimeToString(std::filesystem::file_time_type ftime) 
	//{
	//	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>
	//		(
	//		ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
	//		);
	//	std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
	//	std::string time_str = std::ctime(&cftime);
	//	if (!time_str.empty() && time_str.back() == '\n') 
	//	{
	//		time_str.pop_back();
	//	}
	//	return time_str;
	//}
}
