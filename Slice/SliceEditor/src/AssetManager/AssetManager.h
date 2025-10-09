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
		void CreateDescriptorFile(const std::filesystem::path filePath);
		void CompileTextureAsset(std::filesystem::path const& desc_file);
		void CompileFBXAsset(std::filesystem::path const& desc_file);
		void CompileAudioAsset(AudioData* metaData);
		void CompileShaderAsset(ShaderData* metaData);
		void CompileSceneAsset(SceneData* metaData);
		//std::string TimeToString(std::filesystem::file_time_type ftime);
	private:
		
		std::filesystem::path mAssetDirectory = std::filesystem::path("../SliceEditor/Assets");
		// TODO: Change this to be configurable
		std::filesystem::path mResourcesDirectory = std::filesystem::path("Resources");
		std::unordered_map <std::string, uint64_t> mDescriptorMap; // Maps files to GUIDs

		// Gives editor a vector of all asset files by name for displaying in inspector
		std::unordered_map<AssetType, std::vector<std::string>> mAssets; 

		
		std::unordered_map <std::string, AssetType> mSupportedAssetTypes = 
		{
			{".png", AssetType::Texture},
			{".jpg", AssetType::Texture},
			{".jpeg", AssetType::Texture},
			{".dds", AssetType::Texture},
			{".tga", AssetType::Texture},
			{".gif", AssetType::Texture},
			{".obj", AssetType::Model},
			{".fbx", AssetType::Model},
			{".rainne", AssetType::Model},
			{".wav", AssetType::Audio},
			{".mp3", AssetType::Audio},
			{".ogg", AssetType::Audio},
			{".scene", AssetType::Scene},
			{".shader", AssetType::Shader},
			//{".vert", AssetType::Shader},
			//{".frag", AssetType::Shader}
		};

		std::unordered_map <AssetType, std::string> mAssetExtensions =
		{
			{AssetType::Texture, ".dds"},
			{AssetType::Model, ".mdl"},
			{AssetType::Scene, ".scene"},
			{AssetType::Shader, ".shader"}
		};
	};

}

#endif