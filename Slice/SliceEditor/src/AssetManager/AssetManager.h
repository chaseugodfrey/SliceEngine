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

		void Init();

		SliceEngine::GUID ReadGUIDFromDescriptor(std::filesystem::path path);

		void CreateDescriptorFile(const std::filesystem::path filePath);



	private:
		
		std::filesystem::path mAssetDirectory = std::filesystem::path("../SliceEditor/Assets");
		std::filesystem::path mDescriptorDirectory = std::filesystem::path("../SliceEditor/Descriptor");
		std::filesystem::path mResourcesDirectory = std::filesystem::path("../SliceEditor/Resources");
		std::unordered_map <std::string, SliceEngine::GUID> mDescriptorMap; // Maps files to GUIDs
	};

}

#endif