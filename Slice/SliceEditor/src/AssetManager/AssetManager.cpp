/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AssetManager.cpp

 author:	  Gideon Nicholas Francis
 co-author:   Nic Lai, Lee Yong Yee

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
#include <Systems/SceneSystem.h>
#include "../../SliceEngine/src/Configuration/ProjectSettings.h"
#include <Systems/PrefabSystem.h>
#include <algorithm>
#include "Core/Registry.h"

namespace SliceEditor
{
	void AssetManager::Init()
	{
		//Sanity Checks for the Directories
		if (!std::filesystem::exists(mAssetDirectory))
			std::filesystem::create_directory(mAssetDirectory);

		if (!std::filesystem::exists(mResourcesDirectory))
			std::filesystem::create_directory(mResourcesDirectory);


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

			std::filesystem::path assetPath = dirEntry.path();
			std::string extension = assetPath.extension().string();

			// we dont check meta file until we find the actual asset
			if (extension == ".meta")
				continue;
			// unsupported asset type
			if (mSupportedAssetTypes.find(extension) == mSupportedAssetTypes.end())
				continue;

			std::filesystem::path metaPath = assetPath;
			//metaPath.replace_extension(".meta");
			metaPath += ".meta";

			if (!std::filesystem::exists(metaPath))
			{
				SLICE_LOG_ERROR("Meta file missing for: " + assetPath.string() + ". Creating it now: ");
				CreateDescriptorFile(assetPath, false);
			}

			{
				// meta exist
				// check if the resource exist, if not then we have to recompile the asset
				std::unique_ptr<MetaData> metaData = CreateDefaultMeta(assetPath);

				if (metaData)
				{
					metaData->Deserialize(metaPath);

					if (metaData->assetType == ".mdl")
					{
						ModelData* modelData = static_cast<ModelData*>(metaData.get());
						std::filesystem::path fbxRelativePath(metaData->assetName);
						if (modelData->skeletonGUID.IsValid())
						{
							std::filesystem::path skelPath = fbxRelativePath;
							// change the extension so like if player.mdl to player.skl
							skelPath.replace_extension(".skl"); 

							std::string skelName = skelPath.generic_string();

							mGUIDtoFilename[modelData->skeletonGUID] = skelName;
							mFilenameToGUID[skelName] = modelData->skeletonGUID;
						}

						if (modelData->animationGUID.IsValid())
						{
							std::filesystem::path skelPath = fbxRelativePath;
							// change the extension so like if player.mdl to player.skl
							skelPath.replace_extension(".animpkg");

							std::string skelName = skelPath.generic_string();

							mGUIDtoFilename[modelData->animationGUID] = skelName;
							mFilenameToGUID[skelName] = modelData->animationGUID;
						}
					}

					std::filesystem::path resourcePath = metaData->resourcePath;

					bool resourceMissing = !std::filesystem::exists(resourcePath);

					bool assetIsNewer = false;
					
					// if resource is not missing
					// check if the asset is newer than the resource, if it is then recompile it
					if (!resourceMissing)
					{
						auto assetTime = std::filesystem::last_write_time(assetPath);
						auto resourceTime = std::filesystem::last_write_time(resourcePath);
						if (assetTime > resourceTime)
						{
							assetIsNewer = true;
						}
					}

					// if its either missing or the asset is newer then recompile
					if (resourceMissing || assetIsNewer)
					{
						SLICE_LOG_ERROR("Recompiling asset: " + assetPath.string());
						AssetType type = AssetType::Unsupported;
						for (auto it : mAssetExtensions)
						{
							if (it.second == metaData->assetType)
							{
								type = it.first;
								break;
							}
						}

						CreateResource(metaData.get(), type, false);
					}

					// register validated assets
					mGUIDtoFilename[metaData->guid] = metaData->assetName;
					mFilenameToGUID[metaData->assetName] = metaData->guid;

					//auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
					//resourceMgr->RegisterResourceAsset(metaPath.string());
				}
			}
		}

		mAssetFileWatcher = std::make_unique<filewatch::FileWatch<std::string>>(
			mAssetDirectory.string(),
			[this](const std::string& path, const filewatch::Event changeType)
			{
				OnAssetFileSystemEvent(path, changeType);
			}
		);
		AddDefaultModelsToMap();
		CreateAssetManifest();

	//	CreateDefaultAsset(mAssetDirectory, AssetType::Material);

		SLICE_LOG("Asset Manager Initialized");
	}

	void AssetManager::UpdateFolder()
	{
		//SliceEditor::FileWatcherLogic::UpdateFolder(*this);
	}

	SliceEngine::GUID AssetManager::ReadGUIDFromDescriptor(std::filesystem::path path)
	{
		auto guid = path.stem();

		return SliceEngine::GUID::FromString(guid.string());
	}

	std::string AssetManager::CreateDescriptorFile(const std::filesystem::path filePath, bool AddToRM)
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
		metaData = CreateDefaultMeta(filePath);


		if (metaData)
		{				
			/*
				meta file breakdown

				assetName = file path name without extension
				guid
				asset type = compiled asset extension
				asset path = original asset path in asset folder
				resource path = path within the resource folder 
			*/
			
			// used only for things that copies over its original asset type (i.e .scene/.shader/.vert/etc
			
			//UNUSED
			//std::string tempPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + metaData->assetType;
			std::filesystem::path metaPath = filePath;
			//metaPath.replace_extension(".meta");
			metaPath += ".meta";

			// get the file path to the meta file
			metaData->Serialize(metaPath);
			#pragma region Resource Compiling Section
			// compile the asset here?? or before creating the meta file?
			switch (assetType)
			{
			case AssetType::Texture:
				// This should create the texture asset into the resource folder
				CompileTextureAsset(metaPath);
				break;
			case AssetType::Model:
			{
				auto* data = static_cast<ModelData*>(metaData.get());
				if (data->is_static == false)
				{
					std::unique_ptr<MetaData> skeleData = std::make_unique<SkeletonData>();
					skeleData->InitMetaData(filePath, AssetType::Skeleton, mAssetExtensions[AssetType::Skeleton]);
					data->skeleMetaPath = CreateResource(skeleData.get(), AssetType::Skeleton, AddToRM).string();
					data->skeletonGUID = skeleData->guid;

					std::unique_ptr<MetaData> animData = std::make_unique<AnimData>();
					animData->InitMetaData(filePath, AssetType::Animation, mAssetExtensions[AssetType::Animation]);
					data->animMetaPath = CreateResource(animData.get(), AssetType::Animation, AddToRM).string();
					data->animationGUID = animData->guid;
				}
				metaPath = data->Serialize(metaPath); //Re-serialise with the skele and anim dataPaths
				CompileFBXAsset(metaPath);

				break;
			}
			case AssetType::Audio:
				// idk audio yet
				CompileAudioAsset(static_cast<AudioData*>(metaData.get()));
				break;
				// prefab and scene is the same just copy it over
			case AssetType::Prefab:
			case AssetType::Scene:
				CompileSceneAsset(static_cast<SceneData*>(metaData.get()));
				break;
			case AssetType::Controller:
				CompileStateMachineAsset(static_cast<StateMachineData*>(metaData.get()));
				break;
			case AssetType::Shader:
				CompileShaderAsset(static_cast<ShaderData*>(metaData.get()));
				break;
			case AssetType::VertShader:
				CompileVertShaderAsset(static_cast<VertShaderData*>(metaData.get()));
				break;
			case AssetType::GeomShader:
				CompileGeomShaderAsset(static_cast<GeomShaderData*>(metaData.get()));
				break;
			case AssetType::FragShader:
				CompileFragShaderAsset(static_cast<FragShaderData*>(metaData.get()));
				break;
			case AssetType::Material:
				CompileMaterialAsset(static_cast<MaterialData*>(metaData.get()));
				break;
			case AssetType::NavMesh:
				CompileNavMeshAsset(static_cast<NavMeshData*>(metaData.get()));
				break;
			}
			#pragma endregion

			// register into resource manager
			//auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
			//resourceMgr->RegisterResourceAsset(metaData->guid, metaData->resourcePath);

			//mAssets[assetType].push_back(metaData->assetName);

			// Update the descriptor map
			//mDescriptorMap[filePath.filename().string()] = metaData->guid.GetGUID();
			mGUIDtoFilename[metaData->guid] = filePath.filename().stem().string();
			mFilenameToGUID[metaData->assetName] = metaData->guid;

			if (AddToRM)
			{
				auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
				resourceMgr->RegisterResourceAsset(metaPath.string());
			}

			return metaData->resourcePath;
		}
		return "";
	}

	std::filesystem::path AssetManager::CreateResource(MetaData* metaData, AssetType assetType, bool AddToRM)
	{
		// get the meta file path
		std::filesystem::path metaPath = metaData->assetPath;
		//metaPath.replace_extension(".meta");
		metaPath += ".meta";

		metaData->Serialize(metaPath);

		//mAssets[assetType].push_back(metaData->assetName);
		// Update the descriptor map
		//mDescriptorMap[metaData->assetName] = metaData->guid.GetGUID();
		mGUIDtoFilename[metaData->guid] = metaData->assetName;
		mFilenameToGUID[metaData->assetName] = metaData->guid;

		switch (assetType)
		{
		case AssetType::Texture:
			// This should create the texture asset into the resource folder
			CompileTextureAsset(metaPath);
			break;
		case AssetType::Skeleton:
		{
			CompileFBXAsset(metaPath);
			// after creating resource file
			//std::filesystem::path origin = metaData->resourcePath; // path to the compiled resource
			//// Assets/Models/Player.fbx <-- asset path
			//std::filesystem::path target = metaData->assetPath;
			//target = target.parent_path(); // take Assets/Models/ <-- Assetpath folder

			//std::string assetFullname = metaData->assetName + metaData->assetType;

			//target = target / assetFullname;

			//std::filesystem::copy(origin, target, std::filesystem::copy_options::overwrite_existing);

			break;
		}
		case AssetType::Animation:
		{
			CompileFBXAsset(metaPath);
			// after creating resource file
			//std::filesystem::path origin = metaData->resourcePath; // path to the compiled resource
			//// Assets/Models/Player.fbx <-- asset path
			//std::filesystem::path target = metaData->assetPath;
			//target = target.parent_path(); // take Assets/Models/ <-- Assetpath folder

			//std::string assetFullname = metaData->assetName + metaData->assetType;

			//target = target / assetFullname;

			//std::filesystem::copy(origin, target, std::filesystem::copy_options::overwrite_existing);

			//then add to the map pepeHand
			// if static
			break;
		}

		case AssetType::Model:
			// Compile the model file and write into the resource folder
			CompileFBXAsset(metaPath);

			break;
		case AssetType::Audio:
			// idk audio yet
			CompileAudioAsset(static_cast<AudioData*>(metaData));
			break;
			// prefab and scene is the same just copy it over
		case AssetType::Prefab:
		case AssetType::Scene:
			CompileSceneAsset(static_cast<SceneData*>(metaData));
			break;
		case AssetType::Controller:
			CompileStateMachineAsset(static_cast<StateMachineData*>(metaData));
			break;
		case AssetType::Shader:
			CompileShaderAsset(static_cast<ShaderData*>(metaData));
			break;
		case AssetType::VertShader:
			CompileVertShaderAsset(static_cast<VertShaderData*>(metaData));
			break;
		case AssetType::GeomShader:
			CompileGeomShaderAsset(static_cast<GeomShaderData*>(metaData));
			break;
		case AssetType::FragShader:
			CompileFragShaderAsset(static_cast<FragShaderData*>(metaData));
			break;
		case AssetType::Material:
			CompileMaterialAsset(static_cast<MaterialData*>(metaData));
			break;
		case AssetType::NavMesh:
			CompileNavMeshAsset(static_cast<NavMeshData*>(metaData));
			break;
		}


		// register into resource manager
		if (AddToRM)
		{
			auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
			resourceMgr->RegisterResourceAsset(metaPath.string());
		}

		return metaData->resourcePath;
	}

	std::unique_ptr<MetaData> AssetManager::CreateDefaultMeta(const std::filesystem::path filePath)
	{
		//Find out the type of asset:
		std::string ext = filePath.extension().string();

		auto it = mSupportedAssetTypes.find(ext);
		if (it == mSupportedAssetTypes.end())
		{
			SLICE_LOG("Unsupported asset type for file: " + filePath.string());
			return nullptr;
		}

		AssetType assetType = it->second.first;
		std::unique_ptr<MetaData> metaData;

		// I think can compile assets somewhere around here
		switch (assetType)
		{
		case AssetType::Texture:
			metaData = std::make_unique<TextureData>();
			break;
		case AssetType::Model:
			metaData = std::make_unique<ModelData>();
			break;
		case AssetType::Audio:
			metaData = std::make_unique<AudioData>();
			break;
		case AssetType::Scene:
			metaData = std::make_unique<SceneData>();
			break;
		case AssetType::Controller:
			metaData = std::make_unique<StateMachineData>();
			break;
		case AssetType::Shader:
			metaData = std::make_unique<ShaderData>();
			break;
		case AssetType::VertShader:
			metaData = std::make_unique<VertShaderData>();
			break;
		case AssetType::GeomShader:
			metaData = std::make_unique<GeomShaderData>();
			break;
		case AssetType::FragShader:
			metaData = std::make_unique<FragShaderData>();
			break;
		case AssetType::Material:
			metaData = std::make_unique<MaterialData>();
			break;
		case AssetType::NavMesh:
			metaData = std::make_unique<NavMeshData>();
			break;
		case AssetType::Prefab:
			metaData = std::make_unique<PrefabData>();
			break;
		}

		if (metaData)
		{
			metaData->InitMetaData(filePath, assetType, mAssetExtensions[assetType]);

			return metaData;
		}

		return nullptr;
	}

	void AssetManager::AddDefaultModelsToMap()
	{
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CUBE_DEFAULT] = "Cube";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::SPHERE_DEFAULT] = "Sphere";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT] = "Low Poly Sphere";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CAPSULE_DEFAULT] = "Capsule";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::LINE_DEFAULT] = "Line";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::QUAD_DEFAULT] = "Quad";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::FRUSTRUM_DEFAULT] = "Frustrum";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::COLOR_DEADED_DEFAULT] = "Color Deaded";


	}
	
#pragma region Asset Compiling
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
	void AssetManager::CompileAudioAsset(AudioData* metaData)
	{
		std::filesystem::path filePath(metaData->assetPath);

		try
		{
			std::filesystem::copy(
				filePath,
				metaData->resourcePath,
				std::filesystem::copy_options::overwrite_existing
			);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
		}
	}
	void AssetManager::CompileShaderAsset(ShaderData* metaData)
	{
		std::filesystem::path filePath(metaData->assetPath);
		try
		{
			std::filesystem::copy(
				filePath,
				metaData->resourcePath,
				std::filesystem::copy_options::overwrite_existing
			);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
			//return;
		}

	}
	void AssetManager::CompileVertShaderAsset(VertShaderData* metaData)
	{
		std::filesystem::path filePath(metaData->assetPath);
		try
		{
			std::filesystem::copy(
				filePath,
				metaData->resourcePath,
				std::filesystem::copy_options::overwrite_existing
			);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
			//return;
		}

	}
	void AssetManager::CompileGeomShaderAsset(GeomShaderData* metaData)
	{
		std::filesystem::path filePath(metaData->assetPath);
		try
		{
			std::filesystem::copy(
				filePath,
				metaData->resourcePath,
				std::filesystem::copy_options::overwrite_existing
			);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
			//return;
		}

	}
	void AssetManager::CompileFragShaderAsset(FragShaderData* metaData)
	{
		std::filesystem::path filePath(metaData->assetPath);
		try
		{
			std::filesystem::copy(
				filePath,
				metaData->resourcePath,
				std::filesystem::copy_options::overwrite_existing
			);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
			//return;
		}

	}
	void AssetManager::CompileMaterialAsset(MaterialData* metaData)
	{
		std::filesystem::path filePath(metaData->assetPath);

		try
		{
			std::filesystem::copy(
				filePath,
				metaData->resourcePath,
				std::filesystem::copy_options::overwrite_existing
			);
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
			std::filesystem::copy(
				filePath,
				metaData->resourcePath,
				std::filesystem::copy_options::overwrite_existing
			);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
			//return;
		}
	}
	void AssetManager::CompileNavMeshAsset(NavMeshData* metaData)
	{
		std::filesystem::path filePath(metaData->assetPath);

		try
		{
			std::filesystem::copy(
				filePath,
				metaData->resourcePath,
				std::filesystem::copy_options::overwrite_existing
			);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
			//return;
		}
	}
	void AssetManager::CompileStateMachineAsset(StateMachineData* metaData)
	{
		std::filesystem::path filePath(metaData->assetPath);

		try
		{
			std::filesystem::copy(
				filePath,
				metaData->resourcePath,
				std::filesystem::copy_options::overwrite_existing
			);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
			//return;
		}
	}	
#pragma endregion

	void AssetManager::CreatePrefab(SliceEngine::GameObject GO)
	{

		// idk if this will work yet cause i need it implemented in the editor to test
		// but this should create the prefab and compile it to create the resource as well 

		SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().MakePrefab(GO.GetEntity());

		// Create the prefab file
		std::string path = SliceEngine::JSONSerializer::SerializePrefab(GO.GetEntity());
		std::filesystem::path filePath(path);
		// Create the descriptor
		std::string resourcePath = CreateDescriptorFile(filePath, true);
		size_t count = resourcePath.find_last_of(".") - (resourcePath.find_last_of("/\\") + 1);
		std::string guidStr = resourcePath.substr(resourcePath.find_last_of("/\\") + 1, count);
		SliceEngine::GUID guid = (SliceEngine::GUID)std::stoull(guidStr);
		SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().UpdatePrefabComponent(GO.GetEntity(), guid);

		EventManager::GetInstance()->Publish<RefreshContentBrowser>();

		//auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
		//resourceMgr->RegisterResourceAsset(resourcePath);

	}

	void AssetManager::OnAssetFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{

		std::string fullPath = mAssetDirectory.string() + "/" + path;

		std::filesystem::path filePath(fullPath);

		if (filePath.extension() == ".meta")
		{
			return;
		}

		std::lock_guard<std::mutex> lock(mEventQueueMutex);
		mRawFileQueue.push({ filePath, changeType });

	}

	void AssetManager::CleanUpSceneTemp()
	{
		std::filesystem::path mAssetDirectoryFolder = mAssetDirectory;
		//mAssetDirectoryFolder /= "Default";

		for (const auto& file : std::filesystem::directory_iterator(mAssetDirectoryFolder))
		{
			if (file.is_regular_file() && file.path().extension() == ".temp")
			{
				std::filesystem::remove(file);
				break;
			}
		}

		std::filesystem::path mScenesDirectoryFolder = mAssetDirectory;
		mScenesDirectoryFolder /= "Scenes";

		// Check if this directory exists before iterating
		if (std::filesystem::exists(mScenesDirectoryFolder) && std::filesystem::is_directory(mScenesDirectoryFolder))
		{
			for (const auto& file : std::filesystem::directory_iterator(mScenesDirectoryFolder))
			{
				if (file.is_regular_file() && file.path().extension() == ".temp")
				{
					std::filesystem::remove(file.path());
				}
			}
		}

		for (const auto& file : std::filesystem::directory_iterator(mResourcesDirectory))
		{
			if (file.is_regular_file() && file.path().extension() == ".temp")
			{
				std::filesystem::remove(file.path());
			}
		}

	}

	/// <summary>
	/// Call this to create a default asset in asset window 
	/// </summary>
	/// <param name="folderPath">File path to where the asset should be created</param>
	/// <param name="type">Type of asset being created</param>
	void AssetManager::CreateDefaultAsset(std::filesystem::path& folderPath, AssetType type)
	{
		//AssetType type = AssetType::Unsupported;

		if (mDefaultNames.find(type) == mDefaultNames.end())
		{
			SLICE_LOG_ERROR("Creating default does not exist for this type");
			return;
		}

		std::string ext = mAssetExtensions[type];

		std::string baseName = mDefaultNames[type];
		std::string fileName;
		std::filesystem::path filePath;
		int counter = 0;
		// handle file name checking for duplicates
		while (true)
		{
			if (counter == 0)
			{
				fileName = baseName + ext;
			}
			else
			{
				fileName = baseName + "_" + std::to_string(counter) + ext;
			}

			filePath = folderPath / fileName;

			// if this file doesn't exist
			if (!std::filesystem::exists(filePath))
			{
				// this will be the file path that we will create our asset in
				break;
			}

			counter++;
		}

		std::unique_ptr<MetaData> meta;
		switch (type)
		{
			case AssetType::Material:
			{
				meta = std::make_unique<MaterialData>();
				// Create a file in asset folder
				MaterialData* derived = dynamic_cast<MaterialData*>(meta.get());
				// create a default asset file at the file path
				derived->SerializeAsset(filePath); 
				
				break;
			}
			case AssetType::Controller:
			{
				meta = std::make_unique<StateMachineData>();
				// Create a file in asset folder
				StateMachineData* derived = dynamic_cast<StateMachineData*>(meta.get());
				// create a default asset file at the file path
				derived->SerializeAsset(filePath);

				break;
			}
			default:
			{
				SLICE_LOG_ERROR("CAN'T CREATE DEFAULT FOR UNSUPPORTED TYPES");
				break;
			}
		}

		// then now we initialize the other meta data variables
		meta->InitMetaData(filePath, type, ext);
		CreateResource(meta.get(), type);
	}

	void AssetManager::CreateAssetManifest()
	{
		nlohmann::json manifestJson;
		manifestJson["assets"] = nlohmann::json::array();
		for (const auto& [guid, filename] : mGUIDtoFilename)
		{
			std::filesystem::path sourcePath(filename);
			std::string sourceExt = sourcePath.extension().string();

			if (mSupportedAssetTypes.find(sourceExt) == mSupportedAssetTypes.end())
			{
				continue;
			}

			AssetType type = mSupportedAssetTypes.at(sourceExt).first;

			// get the compiled asset extension
			if (mAssetExtensions.find(type) == mAssetExtensions.end())
			{
				continue;
			}

			std::string compiledExt = mAssetExtensions.at(type);

			// NOTE: i could just scan for every .meta file and build the manifest instead of relying on map
			// but this doesnt need me to open up files, only iterate the map and construct strings from there
			// so this might be cheaper?
			// but I need to test how reliable it is

			std::string resourcePath = "Resources/" + std::to_string(guid.GetGUID()) + compiledExt;

			nlohmann::json assetEntry;
			assetEntry["guid"] = guid.GetGUID();
			assetEntry["name"] = filename;
			assetEntry["path"] = resourcePath;

			manifestJson["assets"].push_back(assetEntry);
		}

		// after constructing asset manifest, write to the file path
		std::filesystem::path manifestPath = mResourcesDirectory / "AssetManifest.json";
		std::ofstream outFile(manifestPath);
		if (outFile.is_open())
		{
			outFile << manifestJson.dump(4); // 4 spaces for pretty printing
			outFile.close();
		}
		else
		{
			SLICE_LOG_ERROR("knncb can't make manifest file");
		}

	}

	void AssetManager::CreateModelGO(SliceEngine::GUID guid, HistoryManager& hist)
	{
		// get file name from the GUID
		std::optional<std::string>  fileName = GetFilenameFromGUID(guid);
		if (fileName.has_value())
		{
			std::filesystem::path metapath = GetMetaDataFromFilename(fileName.value());
			ModelData modelData;
			modelData.Deserialize(metapath);

			EditorUtilities::GameObject_CreateModel(modelData.guid, modelData.skeletonGUID, modelData.animationGUID, entt::null, &hist);
		}
	}

	std::filesystem::path AssetManager::GetMetaDataFromFilename(std::string fileName)
	{
		if (mFilenameToGUID.find(fileName) != mFilenameToGUID.end())
		{
			
			std::filesystem::path metaPath = mAssetDirectory / fileName;
			//metaPath.replace_extension(".meta");
			metaPath += ".meta";
			return metaPath;
		}
		else
		{
			SLICE_LOG_WARNING("Filename not found!");
			return "";
		}
	}

	//void AssetManager::HandleAssetAdded(RawFileEvent& addEvent)
	//{

	//}

	void AssetManager::HandleAssetRemoved(RawFileEvent& removeEvent)
	{
		
	}

	void AssetManager::HandleAssetRenamed(RawFileEvent& renamedOld, RawFileEvent& renamedNew)
	{
		
	}

	//Recompile the asset if it has been modified
	void AssetManager::HandleAssetModified(RawFileEvent& event)
	{
		
	}

	void AssetManager::HandleAssetMoved(std::vector<RawFileEvent>& events)
	{
		
	}

	

	std::optional<std::string> AssetManager::GetFilenameFromGUID(SliceEngine::GUID guid)
	{
		std::optional<std::string> filename{};

		auto it = mGUIDtoFilename.find(guid);
		if (it == mGUIDtoFilename.end())
			return filename;

		filename.emplace(it->second);
		return filename;
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
