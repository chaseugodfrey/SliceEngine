#include "TextureLoader.h"
#include <TextureDescriptor.h>
#include "Compressonator.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <type_traits>

namespace Texture_Asset{
	static const std::filesystem::path resource_dir = { "Resources" };

	static CMP_BOOL CompressionCallback(CMP_FLOAT fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2)
	{
		std::printf("\rCompression progress = %3.0f", fProgress);

		return false;
	}

	void Texture_Compiler::Compile(std::filesystem::path const& desc_path) {
		Texture_Descriptor desc{};
		desc.Deserialize(desc_path);
		std::filesystem::path asset_Path = desc.file_path;

		xresource::full_guid new_guid = desc.guid;
		//CMP_InitFramework(); //not needed apparently
		CMP_MipSet img{};


		std::filesystem::path dest_path = resource_dir / folder_name;
		std::filesystem::create_directories(dest_path);
		std::string dest_filename = desc_path.stem().string() + ".dds";
		dest_path /= dest_filename;
		//std::cout << dest_path.relative_path() << std::endl;

		CMP_ERROR status = CMP_LoadTexture(asset_Path.string().c_str(), &img);
		if (status != CMP_OK) {
			std::cout << "Error loading asseet: " << status << std::endl;
			std::cout << "Given path: " << asset_Path << std::endl;
			std::exit(EXIT_FAILURE);
		}
		
		//Premultiply alpha
		if (img.m_TextureDataType == TDT_ARGB && img.m_ChannelFormat == CF_8bit) {
			for (CMP_INT pixel{}; pixel < (CMP_INT)img.dwDataSize; pixel += 4) {
				float alpha = (float)img.pData[pixel + 3] / 255;
				for (int c = 0; c < 3; ++c) {
					img.pData[pixel + c] = CMP_BYTE((float)img.pData[pixel + c] * alpha);
				}
			}
		}

		if (img.m_nMipLevels <= 1) {
			CMP_INT requestLevels = desc.mip_count;
			CMP_INT minSize = CMP_CalcMinMipSize(img.m_nHeight, img.m_nWidth, requestLevels);

			CMP_GenerateMIPLevels(&img, minSize);
		}
		KernelOptions options{};
		switch (desc.cmp_format) {
		case Texture_Asset::RGB_BC1:
			options.format = CMP_FORMAT_BC1;
			break;
		case Texture_Asset::RGBA_BC3:
			options.format = CMP_FORMAT_BC3;
			break;
		}
		options.fquality = desc.comp_quality;
		options.threads = 0;	//auto

		CMP_MipSet buffer{};

		status = CMP_ProcessTexture(&img, &buffer, options, CompressionCallback);
		if (status != CMP_OK) {
			std::cout << "Error processing asseet: " << status << std::endl;
			CMP_FreeMipSet(&img);

			std::exit(EXIT_FAILURE);
		}

		status = CMP_SaveTexture(dest_path.string().c_str(), &buffer);
		if (status != CMP_OK) {
			std::cout << "Error processing asseet: " << status << std::endl;
			CMP_FreeMipSet(&img);
			CMP_FreeMipSet(&buffer);

			std::exit(EXIT_FAILURE);
		}

		CMP_FreeMipSet(&buffer);
		CMP_FreeMipSet(&img);
	}
}