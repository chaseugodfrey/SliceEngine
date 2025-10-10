#ifndef TEXTURE_LOADER
#define TEXTURE_LOADER

#include <json.hpp>
namespace Texture_Asset{
	class Texture_Compiler {
		//Get the data out
		unsigned int cmp_format{ 49 };	//49 is number for bc3 comp
		bool generate_mips{ true };
		unsigned int mip_count{ 10 };
		float comp_quality{ 1.f };
	public:
		void Read_Json(nlohmann::json const&);
		bool Compile_Asset(const char* src, const char* dest);
	};
}

//easy enum serializing
#endif