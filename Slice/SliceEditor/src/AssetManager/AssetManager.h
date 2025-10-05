#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <fstream>
#include <filesystem>
#include "json.hpp"
#include "AssetTypes.h"

namespace SliceEditor
{
	struct MetaData;

	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		void Init();

		SliceEngine::GUID ReadGUIDFromDescriptor(std::filesystem::path path);

		void CreateDescriptorFile(const std::filesystem::path filePath);

	private:
		
		std::filesystem::path mAssetDirectory = std::filesystem::path("../SliceEditor/Assets");
		std::filesystem::path mDescriptorDirectory = std::filesystem::path("../SliceEditor/Descriptor");
		// TODO: Change this to be configurable
		std::filesystem::path mResourcesDirectory = std::filesystem::path("Resources");
		std::unordered_map <std::string, SliceEngine::GUID> mDescriptorMap; // Maps files to GUIDs
		
		// Gives editor a vector of all asset files by name for displaying in inspector
		std::unordered_map<AssetType, std::vector<std::string>> mAssets; 

		
		std::unordered_map <std::string, AssetType> mSupportedAssetTypes = 
		{
			{".png", AssetType::Texture},
			{".jpg", AssetType::Texture},
			{".jpeg", AssetType::Texture},
			{".bmp", AssetType::Texture},
			{".tga", AssetType::Texture},
			{".gif", AssetType::Texture},
			{".obj", AssetType::Model},
			{".fbx", AssetType::Model},
			//{".wav", AssetType::Audio},
			//{".mp3", AssetType::Audio},
			//{".ogg", AssetType::Audio},
			{".scene", AssetType::Scene},
			{".shader", AssetType::Shader},
			//{".vert", AssetType::Shader},
			//{".frag", AssetType::Shader}
		};
	};

}

#endif