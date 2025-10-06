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

			
			metaData->resourcePath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + metaData->assetType;
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
					try
					{
						std::filesystem::copy(filePath, tempPath);
					}
					catch (std::filesystem::filesystem_error& e)
					{
						SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
						//return;
					}
				}
				else
				{
					// TODO: Add the call to exe to convert to dds
					return;
				}

				metaData->Serialize(mResourcesDirectory);
				break;
			case AssetType::Model:
				if (ext == ".mdl")
				{
					metaData->Serialize(mResourcesDirectory);


					try
					{
						std::filesystem::copy(filePath, tempPath);
					}
					catch (std::filesystem::filesystem_error& e)
					{
						SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
						//return;
					}
				}
				else
				{
					// Note:: This really has to be reworked cause the sequence of
					// compiler run and asset load is no way at all properly synchronized and planned
					// cause of how last min i got the compile function i have to duck tape for submission

					metaData->resourcePath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".mdl";
					metaData->Serialize(mResourcesDirectory);

					std::filesystem::path m = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".meta";
					CompileFBXAsset(m);
					//TODO: Should pass in the resource directory into compileFBX and the guid and it should create the file there
					// instead of this round about way to move it there
					std::filesystem::path oldCompiledFile = filePath;
					std::string newFileName = oldCompiledFile.filename().stem().string();
					newFileName += "_mdl.mdl";
					std::filesystem::path compiledFile = oldCompiledFile.replace_filename(newFileName);

					std::filesystem::path newPath = tempPath;
					newPath.replace_extension(".mdl");

					std::filesystem::path finalDestination = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".mdl";

					std::filesystem::rename(compiledFile, finalDestination);
				}
				break;
			case AssetType::Audio:

				break;
			case AssetType::Scene:
				try
				{
					std::filesystem::copy(filePath, tempPath);
				}
				catch (std::filesystem::filesystem_error& e)
				{
					SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
					//return;
				}

				metaData->Serialize(mResourcesDirectory);
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
				try
				{
					std::filesystem::copy(filePath, tempPath);
					std::filesystem::copy(vertPath, tempVertPath);
					std::filesystem::copy(fragPath, tempFragPath);
				}
				catch (std::filesystem::filesystem_error& e)
				{
					SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
					//return;
				}

				metaData->Serialize(mResourcesDirectory);
				break;
			}


			// serialize the meta file 


			// register into resource manager
			auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
			resourceMgr->RegisterResourceAsset(metaData->guid, metaData->resourcePath);

			mAssets[assetType].push_back(metaData->assetName);

			// Update the descriptor map
			mDescriptorMap[filePath.filename().string()] = metaData->guid;
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

}
