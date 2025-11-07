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

			if (mFilenameToGUID.find(fileName) == mFilenameToGUID.end())
			{
				// this file does not have a meta/descriptor file
				// make one ig?
				CreateDescriptorFile(dirEntry.path());
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

	//	CreateDefaultAsset(mAssetDirectory, AssetType::Material);

		SLICE_LOG("Asset Manager Initialized");
	}

	void AssetManager::UpdateFolder()
	{
		std::vector<RawFileEvent> rawEvents;
		

		{
			std::lock_guard<std::mutex> lock(mEventQueueMutex);
			while (!mRawFileQueue.empty())
			{
				rawEvents.push_back(mRawFileQueue.front());
				
				//SLICE_LOG("event added " + filewatch::event_to_string(mRawFileQueue.front()->changeType));
				mRawFileQueue.pop();
			}
		}

		if (rawEvents.empty())
		{
			return;
		}

		//std::vector<AssetFileChangedEvent> processedEvents;

		AssetFileChangedEvent processedEvents = { false };

		if (rawEvents.size() > 1)
		{
			if (rawEvents.begin()->changeType == filewatch::Event::removed && rawEvents.at(1).changeType == filewatch::Event::added)
			{

				HandleAssetMoved(rawEvents);
			}
			else if (rawEvents.begin()->changeType == filewatch::Event::renamed_old && rawEvents.at(1).changeType == filewatch::Event::renamed_new)
			{
				
				HandleAssetRenamed(rawEvents);
				
			}
			else if (rawEvents.begin()->changeType == rawEvents.at(1).changeType && rawEvents.begin()->filePath == rawEvents.at(1).filePath)
			{
				HandleAssetModified(rawEvents);
			}
			else if (rawEvents.begin()->changeType == filewatch::Event::added)
			{
				HandleAssetAdded(rawEvents.at(0));
			}
			else if (rawEvents.begin()->changeType == filewatch::Event::removed)
			{
				HandleAssetRemoved(rawEvents.at(0));
			}
			
		}
		else
		{
			switch (rawEvents.begin()->changeType)
			{
				case filewatch::Event::added:
				{
					HandleAssetAdded(rawEvents.at(0));

					break;
				}
				case filewatch::Event::removed:
				{

					HandleAssetRemoved(rawEvents.at(0));
					break;
				}
				case filewatch::Event::modified:
				{
					HandleAssetModified(rawEvents);

					break;
				}
			}
		}
		

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
			
			// get the file path to the meta file
			std::filesystem::path metaPath = metaData->Serialize(mResourcesDirectory);


			#pragma region Resource Compiling Section
			// compile the asset here?? or before creating the meta file?
			switch (assetType)
			{
			case AssetType::Texture:
				// This should create the texture asset into the resource folder
				CompileTextureAsset(metaPath);
				break;
			case AssetType::Model:
				// Compile the model file and write into the resource folder
				CompileFBXAsset(metaPath);
				break;
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
			case AssetType::Material:
				CompileMaterialAsset(static_cast<MaterialData*>(metaData.get()));
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

			#pragma endregion

			// register into resource manager
			//auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
			//resourceMgr->RegisterResourceAsset(metaData->guid, metaData->resourcePath);

			//mAssets[assetType].push_back(metaData->assetName);

			// Update the descriptor map
			//mDescriptorMap[filePath.filename().string()] = metaData->guid.GetGUID();
			mGUIDtoFilename[metaData->guid] = filePath.filename().stem().string();
			mFilenameToGUID[filePath.filename().stem().string()] = metaData->guid;
			return metaData->resourcePath;
		}
		return "";
	}

	std::filesystem::path AssetManager::CreateResource(MetaData* metaData, AssetType assetType, bool AddToRM)
	{


		std::filesystem::path metaPath = metaData->Serialize(mResourcesDirectory);

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
		case AssetType::Animation:
		case AssetType::Model:
			// Compile the model file and write into the resource folder
			CompileFBXAsset(metaPath);
			// if static
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
		case AssetType::Material:
			CompileMaterialAsset(static_cast<MaterialData*>(metaData));
			break;
		}

		// register into resource manager
		if (AddToRM)
		{
			auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
			resourceMgr->RegisterResourceAsset(metaPath.string());
		}

		return metaPath;
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
			metaData = std::make_unique<AudioData>();
			typeID = ResourceTypeIDs::SOUND;
			break;
		case AssetType::Scene:
			metaData = std::make_unique<SceneData>();
			typeID = ResourceTypeIDs::SCENE;
			break;
		case AssetType::Controller:
			metaData = std::make_unique<StateMachineData>();
			typeID = ResourceTypeIDs::CONTROLLER;
			break;
		case AssetType::Shader:
			metaData = std::make_unique<ShaderData>();
			typeID = ResourceTypeIDs::SHADER;
			break;
	case AssetType::Material:
			typeID = ResourceTypeIDs::MATERIAL;
			metaData = std::make_unique<MaterialData>();
			break;
		case AssetType::Prefab:
			metaData = std::make_unique<PrefabData>();
			typeID = ResourceTypeIDs::PREFAB;
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
			std::filesystem::copy(filePath, metaData->resourcePath);
		}
		catch (std::filesystem::filesystem_error& e)
		{
			SLICE_LOG_ERROR("Error copying file: " + std::string(e.what()));
		}
	}

	void AssetManager::CompileShaderAsset(ShaderData* metaData)
	{
		std::string fileName = metaData->assetName;
		std::filesystem::path filePath(metaData->assetPath);
		std::filesystem::path parentPath = filePath.parent_path();
		// get the vert and frag path
		std::filesystem::path vertPath = parentPath / (fileName + ".vert");
		std::filesystem::path fragPath = parentPath / (fileName + ".frag");
		std::filesystem::path geomPath = parentPath / (fileName + ".geom");
		// get the destination path for all 2 files
		std::string tempVertPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".vert";
		std::string tempFragPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".frag";
		std::string tempGeomPath = mResourcesDirectory.string() + "/" + std::to_string(metaData->guid.GetGUID()) + ".geom";

		// copy the 3 files over to resources
		// cause loading shaders now come in 3s
		// but they have the same name so their guid would end up being the same
		// so only the main .shader file is used for guid generation
		try
		{
			std::filesystem::copy(filePath, metaData->resourcePath);
			std::filesystem::copy(vertPath, tempVertPath);
			std::filesystem::copy(fragPath, tempFragPath);
			if (std::filesystem::exists(geomPath))
				std::filesystem::copy(geomPath, tempGeomPath);
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
			std::filesystem::copy(filePath, metaData->resourcePath);
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

	void AssetManager::CompileStateMachineAsset(StateMachineData* metaData)
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
						else
						{
							mGUIDtoFilename[(SliceEngine::GUID)guid] = assetName;
							mFilenameToGUID[assetName] = (SliceEngine::GUID)guid;
						}

						//mDescriptorMap.insert_or_assign(assetName, guid);
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
		mAssetDirectoryFolder /= "Default";

		for (const auto& file : std::filesystem::directory_iterator(mAssetDirectoryFolder))
		{
			if (file.is_regular_file() && file.path().extension() == ".temp")
			{
				remove(file);
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

	void AssetManager::RecompileAsset(MetaData* metaData)
	{
		// get the asset type
		AssetType type = AssetType::Unsupported;
		for (auto it : mAssetExtensions)
		{
			if (it.second == metaData->assetType)
			{
				type = it.first;
				break;
			}
		}

		// if the meta data is modified, create resource modifies the resource file
		// but we also have to reflect it in the asset manager for files such as material, controller, etc
		// things that aren't imported resources.

		// before calling create resource
		// we need to update the file in asset folder since create resource copies it to resource
		// for some like material/shader/etc

		switch (type)
		{
		case AssetType::Material:
		{
			MaterialData* derived = dynamic_cast<MaterialData*>(metaData);
			// recreate resource file at the file path
			derived->SerializeAsset(metaData->assetPath);

			break;
		}
		case AssetType::Controller:
		{

			break;
		}
		
		}

		// update the meta file with the new meta data and resource file
		CreateResource(metaData, type);
	}

	void AssetManager::HandleAssetAdded(RawFileEvent& addEvent)
	{
		if (addEvent.filePath.extension() == ".mat" || addEvent.filePath.extension() == ".controller")
		{
			return;
		}

		CreateDescriptorFile(addEvent.filePath);
		SLICE_LOG("Added event at " + addEvent.filePath.filename().string());

		AssetFileChangedEvent processEvent = { true };
		EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);
	}

	void AssetManager::HandleAssetRemoved(RawFileEvent& removeEvent)
	{
		std::filesystem::path removedFilePath(removeEvent.filePath);

		SliceEngine::GUID fileGUID;

		auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
		auto path = resourceMgr->GetResourcePath(removedFilePath.stem().string());

		if (path.has_value())
		{
			try
			{
				fileGUID = SliceEngine::GUID::FromString(path.value().stem().string());

				resourceMgr->ReleaseResource(fileGUID);

				mGUIDtoFilename.erase(fileGUID);

				resourceMgr->mFileNameToGUID.erase(path.value().stem().string());
				//resourceMgr->mGUIDToResource.erase()

				AssetFileChangedEvent processEvent = { true };
				EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);

			}
			catch (const std::exception& e)
			{
				SLICE_LOG_ERROR("Failed to remove resource for " + removedFilePath.filename().string() + ": " + e.what());
			}
		}

		SLICE_LOG("Removed event at " + removeEvent.filePath.string());
	}

	void AssetManager::HandleAssetRenamed(std::vector<RawFileEvent>& events)
	{
		std::filesystem::path oldFilePath(events.begin()->filePath);
		std::filesystem::path newFilePath(events.at(1).filePath);

		SliceEngine::GUID fileGUID;

		//rename old 0
		//rename new 1
		//modify 2

		//Check if the files are in the same folder and if files are the same extension
		if (oldFilePath.parent_path().string() == newFilePath.parent_path().string() && oldFilePath.extension() == newFilePath.extension())
		{
			auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();

			//Find the resource meta file using the old file name
			auto path = resourceMgr->GetResourcePath(oldFilePath.stem().string());
			if (path.has_value())
			{
				std::filesystem::path metaFilePath = path.value();
				metaFilePath.replace_extension(".meta");

				std::string guidString = metaFilePath.stem().string();
				fileGUID = SliceEngine::GUID::FromString(guidString);
				// 2. Use the helper to create the GUID object


				try
				{
					std::ifstream inFile(metaFilePath);
					nlohmann::json metaJson;
					inFile >> metaJson;
					inFile.close();

					std::filesystem::path newAssetPath = newFilePath;
					std::string newAssetName = newFilePath.stem().string();

					metaJson["assetName"] = newAssetName;
					metaJson["assetPath"] = newAssetPath;


					std::ofstream outFile(metaFilePath);
					outFile << metaJson.dump(4); // 4 spaces for pretty printing
					outFile.close();

					mGUIDtoFilename[fileGUID] = newFilePath.filename().string();
					resourceMgr->mFileNameToGUID.erase(oldFilePath.stem().string());
					resourceMgr->mFileNameToGUID.emplace(std::pair< std::string, SliceEngine::GUID>(newFilePath.stem().string(), fileGUID));

					SLICE_LOG("Updated meta file for renamed asset: " + oldFilePath.filename().string());
					AssetFileChangedEvent processEvent = { true };
					EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);
				}
				catch (const std::exception& e)
				{
					SLICE_LOG_ERROR("Failed to update meta file for " + oldFilePath.filename().string() + ": " + e.what());
				}
			}
			else
			{
				SLICE_LOG_WARNING("Could not find resource path for renamed old file: " + newFilePath.filename().string());
			}
		}
	}

	void AssetManager::HandleAssetModified(std::vector<RawFileEvent>& events)
	{
		std::filesystem::path modifiedFilePath(events.begin()->filePath);

		

		SliceEngine::GUID fileGUID;

		auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
		auto path = resourceMgr->GetResourcePath(modifiedFilePath.stem().string());

		if (path.has_value())
		{
			auto hashA = HashFile(modifiedFilePath);
			auto hashB = HashFile(path.value());

			std::filesystem::path metaFilePath = path.value();
			metaFilePath.replace_extension(".meta");

			std::string guidString = path.value().stem().string();
			fileGUID = SliceEngine::GUID::FromString(guidString);

			if (hashA && hashB)
			{
				if (hashA.value() == hashB.value())
				{
					SLICE_LOG("Files are the same");
					//Dont do anything
					return;
				}
				else
				{
					try
					{	
						
						std::filesystem::remove(path.value());

						//CreateResource(metaData, assetType);

						CreateDescriptorFile(modifiedFilePath);
						resourceMgr->ReloadResource(fileGUID);

						SLICE_LOG("Modified event at " + events.begin()->filePath.string());
						AssetFileChangedEvent processEvent = { true };
						EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);
					
					
				
					}
					catch (const std::exception& e)
					{
						SLICE_LOG_ERROR("Failed to update resource file for " + modifiedFilePath.filename().string() + ": " + e.what());
					}
				}
				
			}
			else
			{
				SLICE_LOG_ERROR("Could not read hash files" + hashA.value() + hashB.value());
			}
		}
	}

	void AssetManager::HandleAssetMoved(std::vector<RawFileEvent>& events)
	{
		std::string fileName = events.begin()->filePath.stem().string();


		if (fileName == events.at(1).filePath.stem().string())
		{
			auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
			auto path = resourceMgr->GetResourcePath(fileName);
			if (path.has_value())
			{
				std::filesystem::path metaFilePath = path.value();
				metaFilePath.replace_extension(".meta");

				if (std::filesystem::exists(metaFilePath))
				{
					try
					{

						std::ifstream inFile(metaFilePath);
						nlohmann::json metaJson;
						inFile >> metaJson;
						inFile.close();

						std::filesystem::path newAssetPath = events.at(3).filePath;
						metaJson["assetPath"] = newAssetPath.string();


						std::ofstream outFile(metaFilePath);
						outFile << metaJson.dump(4); // 4 spaces for pretty printing
						outFile.close();

						SLICE_LOG("Updated meta file for moved asset: " + fileName);

						
						AssetFileChangedEvent processEvent = { true };
						EventManager::GetInstance()->Publish<AssetFileChangedEvent>(processEvent);
					}
					catch (const std::exception& e)
					{
						SLICE_LOG_ERROR("Failed to update meta file for " + fileName + ": " + e.what());
					}
				}
			}
			else
			{
				SLICE_LOG_WARNING("Could not find resource path for moved file: " + fileName);
			}


		}
	}

	std::optional<uint64_t> AssetManager::HashFile(const std::filesystem::path& filePath)
	{
		if (!std::filesystem::exists(filePath))
		{
			return std::nullopt;
		}

		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open())
		{
			return std::nullopt;
		}

		const size_t kBufferSize = 4096;
		std::array<char, kBufferSize> buffer;

		
		uint64_t hash = SliceEngine::FNVHash::OffsetBasis;

		
		while (file.read(buffer.data(), buffer.size()).gcount() > 0)
		{
			std::streamsize bytesRead = file.gcount();

		
			for (std::streamsize i = 0; i < bytesRead; ++i)
			{
				hash = (hash ^ static_cast<uint64_t>(static_cast<unsigned char>(buffer[i]))) * SliceEngine::FNVHash::Prime;
			}
		}

		return hash;
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
