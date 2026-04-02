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
		SLICE_LOG("Initializing Asset Manager.");
		EventManager::GetInstance()->Subscribe<AssetRecompiledEvent, &AssetManager::ReloadResource>(this);

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
			{
				// if its a meta file, check if the asset exist, if its not then clear it 
				// since its orphaned and it will keep failing micah's test case thing
				DefaultMeta defMeta;
				defMeta.Deserialize(assetPath);

				if (!std::filesystem::exists(defMeta.assetPath))
				{
					// if the asset path doesn't exist
					std::filesystem::remove(assetPath);
				}
			}
			// unsupported asset type
			if (mSupportedAssetTypes.find(extension) == mSupportedAssetTypes.end())
				continue;

			std::filesystem::path metaPath = assetPath;
			//metaPath.replace_extension(".meta");
			metaPath += ".meta";

			if (!std::filesystem::exists(metaPath))
			{
				SLICE_LOG_ERROR("Meta file missing for: " + assetPath.string() + ". Creating it now: ");
				CreateResource(assetPath, nullptr, false);
				//CreateDescriptorFile(assetPath, false);
			}

			{
				// meta exist
				// check if the resource exist, if not then we have to recompile the asset
				std::unique_ptr<MetaData> metaData = CreateDefaultMeta(assetPath);

				if (metaData)
				{
					metaData->Deserialize(metaPath);

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

						CreateResource(assetPath, metaData.get(), false, true);
					}

					// register validated assets
					mGUIDtoFilename[metaData->guid] = metaData->assetName;
					mFilenameToGUID[metaData->assetName] = metaData->guid;

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
		CreateAssetMaps();

	//	CreateDefaultAsset(mAssetDirectory, AssetType::Material);

		SLICE_LOG("Asset Manager Initialized");
	}

	void AssetManager::ReloadResource(AssetRecompiledEvent event)
	{
		auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
		resourceMgr->ReloadResourceInPlace(event.fileGUID);
	}

	SliceEngine::GUID AssetManager::ReadGUIDFromDescriptor(std::filesystem::path path)
	{
		auto guid = path.stem();

		return SliceEngine::GUID::FromString(guid.string());
	}

	std::filesystem::path AssetManager::CreateResource(const std::filesystem::path filePath, MetaData* metaData, bool AddToRM, bool recompile)
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
		std::unique_ptr<MetaData> tempMeta;
		if (metaData == nullptr)
		{
			tempMeta = CreateDefaultMeta(filePath);

			metaData = tempMeta.get();
		}

		if (!metaData)
		{
			SLICE_LOG_ERROR("Unable to create default meta ");
			return std::filesystem::path("");
		}
		// get the meta file path
		std::filesystem::path metaPath = metaData->assetPath;
		//metaPath.replace_extension(".meta");
		metaPath += ".meta";

		// check if a file already exist
		if (std::filesystem::exists(metaData->resourcePath) && !recompile)
		{
			AssetExistEvent assetEvent(metaData->assetName);
			EventManager::GetInstance()->Publish<AssetExistEvent>(assetEvent);
			SLICE_LOG_ERROR("Trying to import asset that already exist :" + metaData->assetName);
			return metaData->resourcePath;
			//return std::filesystem::path("");
		}

		metaData->Serialize(metaPath);

		//mAssets[assetType].push_back(metaData->assetName);
		// Update the descriptor map
		//mDescriptorMap[metaData->assetName] = metaData->guid.GetGUID();

		switch (assetType)
		{
		case AssetType::Texture:
			// This should create the texture asset into the resource folder
			CompileTextureAsset(metaPath);
			break;
		case AssetType::Skeleton:
			CompileFBXAsset(metaPath);
			break;
		case AssetType::Animation:
			CompileFBXAsset(metaPath);
			break;
		case AssetType::SequencePackage:
			CompileSequencePkgAsset(static_cast<SequencePkgData*>(metaData));
			break;
		case AssetType::Sequence:
			CompileAnimAsset(static_cast<SequenceData*>(metaData));
			break;
		case AssetType::Model:
		{
			auto* data = static_cast<ModelData*>(metaData);
			if (data->is_static == false)
			{
				// skele meta path is just path to resource now
				// if the resource doesnt exist then we have to create it again
				if (!std::filesystem::exists(data->skeleMetaPath))
				{
					std::unique_ptr<MetaData> skeleData = std::make_unique<SkeletonData>();
					skeleData->InitMetaData(filePath, AssetType::Skeleton, mAssetExtensions[AssetType::Skeleton]);
					if (data->skeletonGUID.IsValid())
					{
						skeleData->guid = data->skeletonGUID;
						skeleData->resourcePath = mResourcesDirectory.string() + "/" + std::to_string(skeleData->guid.GetGUID()) + mAssetExtensions[AssetType::Skeleton];
						//skeleData->resourcePath = data->skeleMetaPath;
					}
					data->skeleMetaPath = CreateResource(skeleData->resourcePath, skeleData.get(), AddToRM).string();
					data->skeletonGUID = skeleData->guid;
				}

				if (!std::filesystem::exists(data->animMetaPath))
				{
					std::unique_ptr<MetaData> animData = std::make_unique<AnimationData>();
					animData->InitMetaData(filePath, AssetType::Animation, mAssetExtensions[AssetType::Animation]);
					if (data->animationGUID.IsValid())
					{
						animData->guid = data->animationGUID;
						animData->resourcePath = mResourcesDirectory.string() + "/" + std::to_string(animData->guid.GetGUID()) + mAssetExtensions[AssetType::Animation];
						//animData->resourcePath = data->animMetaPath;
					}
					data->animMetaPath = CreateResource(animData->resourcePath, animData.get(), AddToRM).string();
					data->animationGUID = animData->guid;
				}
			}
			metaPath = data->Serialize(metaPath); //Re-serialise with the skele and anim dataPaths
			CompileFBXAsset(metaPath);
			break;
		}
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
		case AssetType::CustomShader:
			CompileCustomShaderAsset(static_cast<CustomShaderData*>(metaData));
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
		case AssetType::NavMeshBin:
			CompileNavMeshBinAsset(static_cast<NavMeshBinData*>(metaData));
			break;
		case AssetType::Font:
			CompileFontAsset(metaPath);
			break;
		}

		mGUIDtoFilename[metaData->guid] = metaData->assetName;
		mFilenameToGUID[metaData->assetName] = metaData->guid;

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
		SliceEngine::GUID defaultGUID = SliceEngine::GUID::null();
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
		case AssetType::SequencePackage:
			metaData = std::make_unique<SequencePkgData>();
			break;
		case AssetType::Sequence:
			metaData = std::make_unique<SequenceData>();
			break;
		case AssetType::Shader:
			metaData = std::make_unique<ShaderData>();
			break;
		case AssetType::CustomShader:
			metaData = std::make_unique<CustomShaderData>();
			
			defaultGUID = (SliceEngine::GUID)SliceEngine::Type<SliceEngine::SliceEngineTypes::CustomShader>::defaultResourceGUID;

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
		case AssetType::NavMeshBin:
			metaData = std::make_unique<NavMeshBinData>();
			break;
		case AssetType::Prefab:
			metaData = std::make_unique<PrefabData>();
			break;
		case AssetType::CSV:
			metaData = std::make_unique<CSVData>();
			break;
		case AssetType::Font:
			metaData = std::make_unique<FontMetaData>();
			break;
		}

		if (metaData)
		{
			metaData->InitMetaData(filePath, assetType, mAssetExtensions[assetType]);
			std::filesystem::path relativePath = std::filesystem::relative(filePath, "Assets");
			std::string assetName = relativePath.stem().generic_string();
			// Handle default resources here
			// Idk how else without having a function to hardcode register all default assets
			// its prob better to just have a default name for all default resources
			
			if (assetName == "default")
			{
				metaData->guid = defaultGUID;
			}


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
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CYLINDER_DEFAULT] = "Cylinder";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::LINE_DEFAULT] = "Line";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::QUAD_DEFAULT] = "Quad";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::PLANE_DEFAULT] = "Plane";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::FRUSTRUM_DEFAULT] = "Frustrum";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::COLOR_DEADED_DEFAULT] = "White256";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::COLOR_NORMAL_DEFAULT] = "NormalMap";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::FONT_BLANK_DEFAULT] = "Font Default";
		mGUIDtoFilename[(SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CSHADER_DEFAULT] = "Shader Default";
	}

	void AssetManager::CreateAssetMaps()
	{
		//Clear the Map, Just create it again
		mAssetTypeToGUIDs.clear();

		//Create the supported drop-down types
		mAssetTypeToGUIDs[AssetType::Audio] = {};
		mAssetTypeToGUIDs[AssetType::Controller] = {};
		mAssetTypeToGUIDs[AssetType::Material] = {};
		mAssetTypeToGUIDs[AssetType::Model] = {};
		mAssetTypeToGUIDs[AssetType::Texture] = {};
		mAssetTypeToGUIDs[AssetType::Scene] = {};
		mAssetTypeToGUIDs[AssetType::Font] = {};
		mAssetTypeToGUIDs[AssetType::Prefab] = {};
		mAssetTypeToGUIDs[AssetType::CustomShader] = {};

		//Add the Default Values
		mAssetTypeToGUIDs[AssetType::Model].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CUBE_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Model].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::SPHERE_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Model].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Model].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CAPSULE_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Model].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CYLINDER_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Model].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::QUAD_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Model].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::PLANE_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Model].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::FRUSTRUM_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Texture].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::COLOR_DEADED_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Texture].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::COLOR_NORMAL_DEFAULT);
		mAssetTypeToGUIDs[AssetType::Font].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::FONT_BLANK_DEFAULT);
		mAssetTypeToGUIDs[AssetType::CustomShader].push_back((SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CSHADER_DEFAULT);
		
		//Loop Through and Add the Respective GUIDs
		for (const auto& [guid, filename] : mGUIDtoFilename)
		{
			std::filesystem::path sourcePath(filename);
			std::string sourceExt = sourcePath.extension().string();

			if (mSupportedAssetTypes.find(sourceExt) == mSupportedAssetTypes.end())
			{
				continue;
			}

			AssetType type = mSupportedAssetTypes.at(sourceExt).first;

			if (mAssetTypeToGUIDs.find(type) == mAssetTypeToGUIDs.end())
			{
				continue;
			}

			mAssetTypeToGUIDs[type].push_back(guid);
		}
	}

	std::vector<SliceEngine::GUID>* AssetManager::GetMapFromAssetType(std::string assetType)
	{
		if (assetType == "Audio")
		{
			return &mAssetTypeToGUIDs[AssetType::Audio];
		}

		else if (assetType == "Model")
		{
			return &mAssetTypeToGUIDs[AssetType::Model];
		}

		else if (assetType == "Texture")
		{
			return &mAssetTypeToGUIDs[AssetType::Texture];
		}

		else if (assetType == "Material")
		{
			return &mAssetTypeToGUIDs[AssetType::Material];
		}

		else if (assetType == "Controller")
		{
			return &mAssetTypeToGUIDs[AssetType::Controller];
		}

		else if (assetType == "Scene")
		{
			return &mAssetTypeToGUIDs[AssetType::Scene];
		}

		else if (assetType == "Font")
		{
			return &mAssetTypeToGUIDs[AssetType::Font];
		}

		if (assetType == "Prefab")
		{
			return &mAssetTypeToGUIDs[AssetType::Prefab];
		}

		if (assetType == "Custom Shader")
		{
			return &mAssetTypeToGUIDs[AssetType::CustomShader];
		}

        else 
            return nullptr;

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

	void AssetManager::CompileFontAsset(std::filesystem::path const& desc_file)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		//std::filesystem::path rel_Path = std::filesystem::relative(desc_file, compiler_dir);

		std::wstring cmd = desc_file.wstring();
		std::filesystem::path compiler = "Font_Compile.exe";
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
	void AssetManager::CompileCustomShaderAsset(CustomShaderData* metaData)
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
	void AssetManager::CompileNavMeshBinAsset(NavMeshBinData* metaData)
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

	void AssetManager::CompileSequencePkgAsset(SequencePkgData* metaData)
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

	void AssetManager::CompileAnimAsset(SequenceData* metaData)
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

	//Creates the Prefab Asset
	void AssetManager::CreatePrefab(SliceEngine::GameObject GO)
	{

		// idk if this will work yet cause i need it implemented in the editor to test
		// but this should create the prefab and compile it to create the resource as well 

		SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().MakePrefab(GO.GetEntity());

		// Create the prefab file
		std::string path = SliceEngine::JSONSerializer::SerializePrefab(GO.GetEntity());
		std::filesystem::path filePath(path);
		// Create the descriptor
		std::string resourcePath = CreateResource(filePath, nullptr, true).string();//CreateDescriptorFile(filePath, true);
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
		auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
		std::unique_ptr<MetaData> meta;
		switch (type)
		{
			case AssetType::Material:
			{
				meta = std::make_unique<MaterialData>();
				// Create a file in asset folder
				MaterialData* derived = dynamic_cast<MaterialData*>(meta.get());
				derived->shader = (SliceEngine::GUID)SliceEngine::Type<SliceEngine::SliceEngineTypes::CustomShader>::defaultResourceGUID;
				auto shdr = resourceMgr->get<SliceEngine::SliceEngineTypes::CustomShader>(derived->shader);
				for (auto& i : shdr.get()->dataIn)
				{
					switch (i.dataType)
					{
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::BOOL:
						derived->data[i.name] = std::get<bool>(i.baseData);
						break;
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::UINT:
						derived->data[i.name] = std::get<uint32_t>(i.baseData);
						break;
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::INT:
						derived->data[i.name] = std::get<int32_t>(i.baseData);
						break;
					case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::FLOAT:
						derived->data[i.name] = std::get<float>(i.baseData);
						break;
					}
				}

				
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
			case AssetType::CustomShader:
			{
				meta = std::make_unique<CustomShaderData>();
				CustomShaderData* derived = dynamic_cast<CustomShaderData*>(meta.get());

				auto defaultShaderGUID = (SliceEngine::GUID)SliceEngine::Type<SliceEngine::SliceEngineTypes::CustomShader>::defaultResourceGUID;

				if (!resourceMgr->CheckResource(defaultShaderGUID))
				{
					SLICE_LOG_ERROR("Failed to load default Custom Shader");
					break;
				}
				auto defaultShader = resourceMgr->get<SliceEngine::SliceEngineTypes::CustomShader>(defaultShaderGUID).get();
				nlohmann::json metaJson;

				



				std::ofstream output(filePath);

				if (output.is_open())
				{
					output << metaJson.dump(4);
					output.close();
				}

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
		CreateResource(filePath, meta.get(), true);
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

		//Add Defaults to Manifest
		AddDefaultsToManifest(manifestJson);

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

	void AssetManager::AddDefaultsToManifest(nlohmann::json& manifestJSON)
	{
		nlohmann::json assetEntry;

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CUBE_DEFAULT;
		assetEntry["name"] = "Cube";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::SPHERE_DEFAULT;
		assetEntry["name"] = "Sphere";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT;
		assetEntry["name"] = "Low Poly Sphere";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CAPSULE_DEFAULT;
		assetEntry["name"] = "Capsule";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CYLINDER_DEFAULT;
		assetEntry["name"] = "Cylinder";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::LINE_DEFAULT;
		assetEntry["name"] = "Line";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::QUAD_DEFAULT;
		assetEntry["name"] = "Quad";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::PLANE_DEFAULT;
		assetEntry["name"] = "Plane";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::FRUSTRUM_DEFAULT;
		assetEntry["name"] = "Frustrum";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::COLOR_DEADED_DEFAULT;
		assetEntry["name"] = "White256";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::COLOR_NORMAL_DEFAULT;
		assetEntry["name"] = "NormalMap";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::FONT_BLANK_DEFAULT;
		assetEntry["name"] = "Font Default";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);

		assetEntry["guid"] = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::CSHADER_DEFAULT;
		assetEntry["name"] = "Shader Default";
		assetEntry["path"] = "NIL";
		manifestJSON["assets"].push_back(assetEntry);
	}

	void AssetManager::CreateModelGO(SliceEngine::GUID guid, HistoryManager& hist, Entity parent, bool isPrefabInspected)
	{
		// get file name from the GUID
		std::optional<std::string>  fileName = GetFilenameFromGUID(guid);
		if (fileName.has_value())
		{
			std::filesystem::path metapath = GetMetaDataFromFilename(fileName.value());
			ModelData modelData;
			modelData.Deserialize(metapath);

			EditorUtilities::GameObject_CreateModel(modelData.guid, modelData.skeletonGUID, modelData.animationGUID, parent, &hist, isPrefabInspected);
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
