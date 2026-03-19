/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Font.cpp
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads .fnt Font assets

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Font.h"
#include <fstream>

namespace {
	//some consts to help typing
	constexpr uint16_t version_number = 1;	//i think having a vers number could be useful, maybe
}

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		namespace {
			uint16_t version_number = 1;
		}

		void Font_Data::InitializeDefault() {
			for (char ch = 32; ch < 127; ++ch) {
				GlyphData glyph{};
				glyph_datas[ch] = glyph;
			}
			font_size = 1;	//avoid divide by 0
			padding = 0;

			atlas_texture = 0;
		}

		bool Font_Data::unpack_data(char* const buffer, uint64_t& offset) {
			if (atlas_texture) {
				return false;
			}
			unsigned short atlasSize;
			unsigned char* atlas{};

			unsigned char glyph_count = *(buffer + offset); ++offset;
			assert(glyph_count == 95);
			unsigned char glyph_size = *(buffer + offset); ++offset;
			assert(glyph_size == 8);

			for (char ch = 32; ch < 127; ++ch) {
				GlyphData glyph{};

				glyph.w = *(buffer + offset); ++offset;
				glyph.h = *(buffer + offset); ++offset;
				glyph.xoff = *(buffer + offset); ++offset;
				glyph.yoff = *(buffer + offset); ++offset;
				memcpy(&glyph.advance, buffer + offset, 4); offset += 4;

				glyph_datas[ch] = glyph;
			}

			memcpy(&atlasSize, buffer + offset, sizeof(unsigned short)); offset += sizeof(unsigned short);
			font_size = *(buffer + offset); ++offset;
			padding = *(buffer + offset); ++offset;

			atlas = new unsigned char[atlasSize * atlasSize];
			memcpy(atlas, buffer + offset, (size_t)atlasSize * atlasSize); offset += (size_t)atlasSize * atlasSize;


			//get the atlas UV
			constexpr int max_glyphs_row = 10;
			int glyphWidth = (font_size + padding * 2);
			int glyphs_placed = 0;
			int row = 0;
			for (char ch = 33; ch < 127; ++ch) {
				GlyphData const& glyph = glyph_datas.at(ch);

				int start_x = glyphs_placed % max_glyphs_row * glyphWidth;
				int end_x = start_x + glyph.w;
				int start_y = row * glyphWidth;
				int end_y = start_y + glyph.h;

				Atlas_UV uv;
				uv.u_start = (float)start_x / atlasSize;
				uv.u_end = (float)end_x / atlasSize;
				uv.v_start = (float)start_y / atlasSize;
				uv.v_end = (float)end_y / atlasSize;

				atlas_uvs[ch] = uv;

				++glyphs_placed;
				if (!(glyphs_placed % max_glyphs_row)) {
					++row;
				}
			}


			//Get gpu resource
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glCreateTextures(GL_TEXTURE_2D, 1, &atlas_texture);
			CheckGLError();
			glTextureParameteri(atlas_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(atlas_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(atlas_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(atlas_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			CheckGLError();

			glTextureStorage2D(atlas_texture, 1, GL_R8, atlasSize, atlasSize);
			CheckGLError();
			glTextureSubImage2D(atlas_texture, 0, 0, 0, atlasSize, atlasSize, GL_RED, GL_UNSIGNED_BYTE, atlas);
			CheckGLError();

			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

			delete[] atlas;

			return true;
		}

		bool Font_Data::LoadFontResource(std::string const& file) {
			if (!std::filesystem::exists(file)) {
				return false;
			}
			//write binary blob to file
			std::fstream fs(file, std::ios::binary | std::ios::in);

			if (!fs.good()) {
				//std::cout << "Unable to open file: " << file << std::endl;
				return false;
			}

			constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);
			char header_buffer[header_size]{};
			try {
				fs.read(header_buffer, header_size);
			}
			catch (...) {
				//std::cout << "Error reading file: " << file << std::endl;
				fs.close();
				return false;
			}

			if (fs.fail() || fs.eof()) {
				//std::cout << "Unknown file format: " << file << std::endl;
				fs.close();
				return false;
			}

			if (header_buffer[0] != 'F' || header_buffer[1] != 'N' || header_buffer[2] != 'T') {
				//std::cout << "Not a proper fnt file: " << file << std::endl;
				fs.close();
				return false;
			}
			auto vers = version_number;
			vers = *((decltype(version_number)*)(header_buffer + 3));
			if (vers != version_number) {
				//std::cout << "Wrong version, please recompile: " << file << std::endl;
				fs.close();
				return false;
			}

			//finally, get the file size
			uint64_t buffer_size = *((uint64_t*)(header_buffer + 3 + sizeof(version_number)));

			char* const buffer = new char[buffer_size];
			uint64_t offset{};

			fs.read(buffer, buffer_size);

			fs.close();

			unpack_data(buffer, offset);

			delete[] buffer;

			return true;

		}
		void Font_Data::DestroyFontResource() {
			glDeleteTextures(1, &atlas_texture);
		}
	}
}