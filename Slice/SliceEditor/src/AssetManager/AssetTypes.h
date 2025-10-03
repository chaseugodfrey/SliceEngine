#ifndef ASSET_TYPES_H
#define ASSET_TYPES_H

#include <filesystem>

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

struct MetaData
{
	SliceEngine::GUID guid;
	std::string assetType;
	std::string path;
	std::vector<std::string> resourcePath;

	virtual void Serialize(std::filesystem::path& const) = 0;
	virtual void Deserialize(std::filesystem::path& const) = 0;
};

struct TextureData : public MetaData
{
	CompressionFormat cmp_format{ CompressionFormat::RGBA_BC3 };
	MipMapFilter mip_filter{ MipMapFilter::NONE };
	WrapType u_wrap{ WrapType::CLAMP_TO_EDGE };
	WrapType v_wrap{ WrapType::CLAMP_TO_EDGE };
	UsageType usage_type{ UsageType::COLOR };

	float comp_quality{ 0.5f };
	bool generateMips{ true };
	unsigned char mip_count{ 8 };
	bool hasAlpha{ true };
	unsigned char alpha_threshold{ 128 };	//used only for non-blending

	virtual void Serialize(std::filesystem::path const& desc_path)
	{

	}
};


#endif