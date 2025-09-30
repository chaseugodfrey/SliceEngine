#ifndef MESH_DESCRIPTOR
#define MESH_DESCRIPTOR
#include <array>
#include <type_traits>
#include "Asset_Descriptor.h"
#include <fstream>
#include <iostream>

namespace Mesh_Asset{


	inline static constexpr auto resource_type_guid_v = xresource::type_guid(xresource::guid_generator::Instance64FromString("mesh"));
	inline static constexpr auto folder_name = "Mesh";
	//just deal with these 2 first


	struct Mesh_Descriptor : public Asset::Descriptor_Base{
		
		
		virtual void Serialize(std::filesystem::path const& desc_path) {
			std::cout << "Descriptor path: " << desc_path << std::endl;

			std::ofstream ofs(desc_path, std::ios_base::out);
			if (!ofs.good()) {
				std::cerr << "error writing to descriptor" << std::endl;
				std::exit(EXIT_FAILURE);
			}

		/*	ofs << file_path << " " << guid.m_Instance.m_Value << " " << guid.m_Type.m_Value << " " << cmp_format << " " << mip_filter << " " << u_wrap << " " << v_wrap <<
				" " << usage_type << " " << comp_quality << " " << mip_count << " " << hasAlpha << " " << alpha_threshold;*/
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
		/*	ifs >> file_path >> guid.m_Instance.m_Value >> guid.m_Type.m_Value >> readEnum(cmp_format) >> readEnum(mip_filter) >> readEnum(u_wrap) >> readEnum(v_wrap)
				>> readEnum(usage_type) >> comp_quality >> mip_count >> hasAlpha >> alpha_threshold;*/
		}
	};
}
#endif