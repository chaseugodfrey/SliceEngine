#ifndef TEXTURE_DESCRIPTOR
#define TEXTURE_DESCRIPTOR
#include <array>
#include <type_traits>
#include "Asset_Descriptor.h"
#include <fstream>
#include <iostream>

namespace Texture_Asset{

	inline static constexpr auto resource_type_guid_v = xresource::type_guid(xresource::guid_generator::Instance64FromString("texture"));
	inline static constexpr auto folder_name = "Texture";
	static constexpr auto available_extensions = std::array{
		".png",
		".jpg"
	};
	//just deal with these 2 first
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


	struct Texture_Descriptor : public Asset::Descriptor_Base{
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
		
		virtual void Serialize(std::filesystem::path const& desc_path) {
			std::cout << "Descriptor path: " << desc_path << std::endl;

			std::ofstream ofs(desc_path, std::ios_base::out);
			if (!ofs.good()) {
				std::cerr << "error writing to descriptor" << std::endl;
				std::exit(EXIT_FAILURE);
			}

			ofs << file_path << " " << guid.m_Instance.m_Value << " " << guid.m_Type.m_Value << " " << cmp_format << " " << mip_filter << " " << u_wrap << " " << v_wrap <<
				" " << usage_type << " " << comp_quality << " " << mip_count << " " << hasAlpha << " " << alpha_threshold;
			ofs.close();
		}
		virtual void Deserialize(std::filesystem::path const& desc_path) {
			std::cout << "Descriptor path: " << desc_path << std::endl;

			std::ifstream ifs(desc_path, std::ios_base::in);
			if (!ifs.good()) {
				std::cerr << "error reading descriptor" << std::endl;
				std::exit(EXIT_FAILURE);
			}
			//enums
			//ifs >> readEnum(cmp_format);
			ifs >> file_path >> guid.m_Instance.m_Value >> guid.m_Type.m_Value >> Asset::readEnum(cmp_format) >> Asset::readEnum(mip_filter) >> Asset::readEnum(u_wrap) >> Asset::readEnum(v_wrap)
				>> Asset::readEnum(usage_type) >> comp_quality >> mip_count >> hasAlpha >> alpha_threshold;
		}


		/*	reference options for descriptor in xtexture
		* float                       m_Quality                   { 0.5f };
        bool                        m_bSRGB                     { true };
        bool                        m_bGenerateMips             { true };
        int                         m_MipCustomMinSize          {1};
        bool                        m_bFillAveColorByAlpha      { false };
        std::uint8_t                m_AlphaThreshold            { 128 };
        mipmap_filter               m_MipmapFilter              { mipmap_filter::BOX };
        wrap_type                   m_UWrap                     { wrap_type::CLAMP_TO_EDGE };
        wrap_type                   m_VWrap                     { wrap_type::CLAMP_TO_EDGE };
        bool                        m_bTillableFilter           {false};
        float                       m_TilableWidthPercentage    { 0.1f };
        float                       m_TilableHeightPercentage   { 0.1f };
        bool                        m_bNormalMapFlipY           {false};
        bool                        m_bNormalizeNormals         {true};
        bool                        m_bConvertToCubeMap         {false};
        std::uint32_t               m_ToCubeMapFaceResolution   { 1024 };
        bool                        m_ToCubeMapUseBilinear      { true };
		*/
	};
}
#endif