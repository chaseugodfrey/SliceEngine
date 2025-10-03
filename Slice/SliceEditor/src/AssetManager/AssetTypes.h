#ifndef ASSET_TYPES_H
#define ASSET_TYPES_H

enum class AssetType 
{
	Texture,
	Model,
	Audio
};

enum CompressionFormat : std::uint8_t {
	//		RGBA_UNCOMPRESSED,
	RGB_BC1,
	RGBA_BC3
};
enum MipMapFilter : std::uint8_t {
	NONE,
	POINT,
	LINEAR,
	TRIANGLE,
	BOX
};
enum WrapType : std::uint8_t {
	CLAMP_TO_EDGE,
	WRAP,
	MIRROR
};

enum UsageType : std::uint8_t {
	COLOR,
	COLOR_ALPHA,
	TANGENT_NORMAL,
	INTENSITY
};

#endif