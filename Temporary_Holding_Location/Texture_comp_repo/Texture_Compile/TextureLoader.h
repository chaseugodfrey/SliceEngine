#ifndef TEXTURE_LOADER
#define TEXTURE_LOADER
#include <array>
#include <type_traits>
#include <xresource_guid.h>
#include <Asset_Descriptor.h>

namespace Texture_Asset{
	class Texture_Compiler {
	public:
		void Compile(std::filesystem::path const& asset_Path);
	};
}

//easy enum serializing
#endif