#ifndef ASSET_MANAGER
#define ASSET_MANAGER
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <optional>

#include <xresource_guid.h>
#include <Asset_Descriptor.h>

namespace Asset {
	static const std::filesystem::path asset_dir		= { "Assets" };
	static const std::filesystem::path descriptor_dir	= { "Descriptors" };
	static const std::filesystem::path resource_dir		= { "Resources" };
	static const std::filesystem::path compiler_dir		= { "Compilers" };

	class AssetManager {
	public:
		void SetupDirectories();
		std::optional<std::filesystem::path> CreateDescriptor(std::filesystem::path const& asset_path);	//create descriptor for an asset + allocate guid
		void CompileAsset(std::filesystem::path const&);		//compile asset based on descriptor
	};
}

#endif