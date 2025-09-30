#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <fstream>
#include <filesystem>
#include "json.hpp"

namespace SliceEditor
{
	struct MetaData;

	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		bool CreateMetaDataFile(MetaData& metaData);

		void WriteMetaDataFile(const std::string& metaFilePath, MetaData& metaData);

		void Init();

		SliceEngine::GUID ReadGUIDFromDescriptor(std::filesystem::path path);



	private:
		
		std::filesystem::path mAssetDirectory = "../SliceEditor/Assets";
		std::filesystem::path mDescriptorDirectory = "../SliceEditor/Descriptor";
		std::filesystem::path mResourcesDirectory = "../SliceEditor/Resources";
		std::unordered_map <std::string, SliceEngine::GUID> mDescriptorMap; // Maps files to GUIDs
	};

}

#endif