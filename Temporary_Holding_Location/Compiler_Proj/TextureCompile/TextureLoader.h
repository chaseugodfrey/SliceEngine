#ifndef TEXTURE_LOADER
#define TEXTURE_LOADER

#include <json.hpp>
namespace Texture_Asset{
	enum CompressionFormat : std::uint8_t {
		//ima be real idk whats the diff between signed and unsigned, im just yoinking these enums from compressonator
		//same as idk what exactly these are used for, will ask tomas next time
		//ref: https://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats/#bc1

		//		RGBA_UNCOMPRESSED,
		BC1,	//RGB + single bit A, color maps, cutout color maps, normal maps
		BC2,	//rgba kind off, mostly not used anymore
		BC3,	//rgba, color maps with full alpha, packing color and mono maps together
		BC4,	//grayscale, height maps, gloss maps, font atlas, any grayscale image
		BC4s,	//bc4 but signed
		BC5,	//2x grayscale, tangent maps
		BC5s,	//bc5 but signed
		BC6,	//RGB, floats, HDR
		BC6s,	//bc6 but signed
		BC7		//RGB/RGBA, high quality color maps, color maps with full alpha
	};

	class Texture_Compiler {
		//Get the data out
	public:
		unsigned int cmp_format{ 49 };	//49 is number for bc3 comp
		bool generate_mips{ true };
		unsigned int mip_count{ 10 };
		float comp_quality{ 1.f };

		void Read_Json(nlohmann::json const&);
		bool Compile_Asset(const char* src, const char* dest);
	};
}

//easy enum serializing
#endif