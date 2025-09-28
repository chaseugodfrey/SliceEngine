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

		bool CreateMetaDataFile(const std::string& assetPath, MetaData& metaData);

		void WriteMetaDataFile(const std::string& metaFilePath, MetaData& metaData);

		void Init(std::filesystem::path);



	private:
		
		std::filesystem::path mAssetDirectory;
	};

}

#endif