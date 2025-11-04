/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        AssetManager.h

 author:	  Gideon Nicholas Francis
 co-author:   Nic Lai

 email:       g.francis@digipen.edu

 brief:		  Declares the AssetManager class which handles all asset related operations on the editor side.
			  Creates and manages asset descriptor files, GUIDs, and asset listings.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <fstream>
#include <filesystem>
#include "../thirdparty/filewatch/FileWatcher.h"
#include "json.hpp"
#include "AssetTypes.h"

namespace SliceEditor
{
	class MetaData;

	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		void Init();

		SliceEngine::GUID ReadGUIDFromDescriptor(std::filesystem::path path);

		/// <summary>
		/// Check if the resource folder meta files are still accurate
		/// meaning, if the original asset path still exist or if
		/// the resource still exist
		/// if not then delete the meta file so that it can be recompiled
		/// </summary>
		/// <param name="path">Resource Folder Path</param>
		void ScanResourceFolder();
		std::string CreateDescriptorFile(const std::filesystem::path filePath);
		std::unique_ptr<MetaData> CreateDefaultMeta(const std::filesystem::path filePath);
		void AddDefaultModelsToMap();
		void CreateResource(MetaData* metaData, AssetType assetType);
		void CompileTextureAsset(std::filesystem::path const& desc_file);
		void CompileFBXAsset(std::filesystem::path const& desc_file);
		void CompileAudioAsset(AudioData* metaData);
		void CompileShaderAsset(ShaderData* metaData);
		void CompileMaterialAsset(MaterialData* metaData);
		void CompileSceneAsset(SceneData* metaData);
		void CreatePrefab(SliceEngine::GameObject GO);
		void CleanUpSceneTemp();
		//std::string TimeToString(std::filesystem::file_time_type ftime);

		std::unordered_map <SliceEngine::GUID, std::string> mGUIDtoFilename; // Maps GUIDs to fileName
		std::unordered_map <std::string, std::pair<AssetType,std::string>> mSupportedAssetTypes = 
		{
			{".png",  {AssetType::Texture, "Texture"}},
			{".jpg", {AssetType::Texture, "Texture"}},
			{".jpeg", {AssetType::Texture, "Texture"}},
			{".dds", {AssetType::Texture, "Texture"}},
			{".tga", {AssetType::Texture, "Texture"}},
			{".gif", {AssetType::Texture, "Texture"}},
			{".obj", {AssetType::Model, "Model"}},
			{".fbx", {AssetType::Model, "Model"}},
			{".wav", {AssetType::Audio, "Audio"}},
			{".mp3", {AssetType::Audio, "Audio"}},
			{".ogg", {AssetType::Audio, "Audio"}},
			{".scene", {AssetType::Scene, "Scene"}},
			{".shader", {AssetType::Shader, "Shader"}},
			{".mat", {AssetType::Material, "Material"}},
			{".prefab", {AssetType::Prefab, "Prefab"}}
			//{".vert", AssetType::Shader},
			//{".frag", AssetType::Shader}
		};

		std::unordered_map <AssetType, std::string> mAssetExtensions =
		{
			{AssetType::Texture, ".dds"},
			{AssetType::Model, ".mdl"},
			{AssetType::Scene, ".scene"},
			{AssetType::Shader, ".shader"},
			{AssetType::Audio, ".wav"},
			{AssetType::Material, ".mat"},
			{AssetType::Prefab, ".prefab"}
		};
	private:
		
		std::filesystem::path mAssetDirectory = std::filesystem::path("../SliceEditor/Assets");
		// TODO: Change this to be configurable
		std::filesystem::path mResourcesDirectory = std::filesystem::path("Resources");

		std::unique_ptr<filewatch::FileWatch<std::string>> mAssetFileWatcher;
		// Gives editor a vector of all asset files by name for displaying in inspector
		//std::unordered_map<AssetType, std::vector<std::string>> mAssets; 


	};

}

#endif