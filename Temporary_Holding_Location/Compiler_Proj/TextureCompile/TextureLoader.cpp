#include "TextureLoader.h"
#include "Compressonator.h"
#include <iostream>
#include <filesystem>
#include <fstream>

namespace Texture_Asset{
	//static CMP_BOOL CompressionCallback(CMP_FLOAT fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2)
	//{
	//	std::printf("\rCompression progress = %3.0f", fProgress);

	//	return false;
	//}


	void Texture_Compiler::Read_Json(nlohmann::json const& obj) {
		//parse meta file for the compiling options
		if (obj.contains("comp_quality")) {
			comp_quality = obj["comp_quality"];
		}
		if (obj.contains("generateMips")) {
			generate_mips = obj["generateMips"];
		}
		if (obj.contains("mip_count")) {
			mip_count = obj["mip_count"];
		}
		if (obj.contains("comp_format")) {
			CompressionFormat format = obj["comp_format"];
			switch (format) {
			case BC1:
				cmp_format = CMP_FORMAT_BC1;
				break;
			case BC2:
				cmp_format = CMP_FORMAT_BC2;
				break;
			case BC3:
				cmp_format = CMP_FORMAT_BC3;
				break;
			case BC4:
				cmp_format = CMP_FORMAT_BC4;
				break;
			case BC4s:
				cmp_format = CMP_FORMAT_BC4_S;
				break;
			case BC5:
				cmp_format = CMP_FORMAT_BC5;
				break;
			case BC5s:
				cmp_format = CMP_FORMAT_BC5_S;
				break;
			case BC6:
				cmp_format = CMP_FORMAT_BC6H;
				break;
			case BC6s:
				cmp_format = CMP_FORMAT_BC6H_SF;
				break;
			case BC7:
				cmp_format = CMP_FORMAT_BC7;
				break;
			default:
				cmp_format = CMP_FORMAT_BC7;
				break;
			}
		}
	}

	bool Texture_Compiler::Compile_Asset(const char* src, const char* dest) {
		//CMP_InitFramework(); //not needed apparently
		CMP_MipSet img{};

		CMP_ERROR status = CMP_LoadTexture(src, &img);
		if (status != CMP_OK) {
			std::cout << "Error loading asseet: " << status << std::endl;
			std::cout << "Given path: " << src << std::endl;
			return false;
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

		if (generate_mips ) {
			//if (img.m_nMipLevels <= 1) {
			CMP_INT requestLevels = mip_count;
			CMP_INT minSize = CMP_CalcMinMipSize(img.m_nHeight, img.m_nWidth, requestLevels);

			CMP_GenerateMIPLevels(&img, minSize);
			//}
		}

		KernelOptions options{};
		options.format = (CMP_FORMAT)cmp_format;
		options.fquality = comp_quality;
		options.threads = 0;	//auto

		CMP_MipSet buffer{};

		status = CMP_ProcessTexture(&img, &buffer, options, nullptr);
		if (status != CMP_OK) {
			std::cout << "Error processing asseet: " << status << std::endl;
			CMP_FreeMipSet(&img);

			return false;
		}

		status = CMP_SaveTexture(dest, &buffer);
		if (status != CMP_OK) {
			std::cout << "Error processing asseet: " << status << std::endl;
			CMP_FreeMipSet(&img);
			CMP_FreeMipSet(&buffer);

			return false;
		}

		CMP_FreeMipSet(&buffer);
		CMP_FreeMipSet(&img);
	}
}