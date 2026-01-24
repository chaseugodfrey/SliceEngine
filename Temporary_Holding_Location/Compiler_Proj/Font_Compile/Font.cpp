#include "Font.h"

#if _DEBUG
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>

namespace {
	stbtt_fontinfo font_info;

	uint16_t version_number = 1;
}
namespace Font {

	void FontData::Save_Fnt_File(std::string const& file_path) {
		std::filesystem::path path(file_path);
		std::filesystem::create_directories(path.parent_path());


		constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);
		uint64_t buffer_size = get_fnt_size();
		uint64_t total = get_fnt_size();

		char* const buffer = new char[buffer_size + header_size];

		//header
		uint64_t offset{};
		*(buffer + offset) = 'F'; offset += 1;
		*(buffer + offset) = 'N'; offset += 1;
		*(buffer + offset) = 'T'; offset += 1;
		memcpy(buffer + offset, &version_number, sizeof(version_number)); offset += sizeof(version_number);
		memcpy(buffer + offset, &buffer_size, sizeof(uint64_t)); offset += sizeof(uint64_t);

		//buffer
		pack_data(buffer, offset);

		//write binary blob to file
		std::fstream fs(file_path, std::ios::binary | std::ios::out);
		if (!fs.good()) {
			delete[] buffer;
			return;
		}

		fs.write(buffer, buffer_size + header_size);
		fs.close();

		delete[] buffer;
	}

	void FontData::pack_data(char* const buffer, uint64_t& offset) const  {
		*(buffer + offset) = 95; ++offset;
		*(buffer + offset) = 8; ++offset;

		for (char ch = 32; ch < 127; ++ch) {
			GlyphData const& glyph = glyph_datas.at(ch);

			*(buffer + offset) = glyph.w; ++offset;
			*(buffer + offset) = glyph.h; ++offset;
			*(buffer + offset) = glyph.xoff; ++offset;
			*(buffer + offset) = glyph.yoff; ++offset;

			memcpy(buffer + offset, &glyph.advance, 4); offset += 4;
		}

		memcpy(buffer + offset, &atlasSize, sizeof(unsigned short)); offset += sizeof(unsigned short);
		*(buffer + offset) = font_size; ++offset;
		*(buffer + offset) = padding; ++offset;

		memcpy(buffer + offset, atlas, (size_t)atlasSize * atlasSize);
	}

	bool FontData::Load_Fnt_File(std::string const& file) {
		if (!std::filesystem::exists(file)) {
			return false;
		}
		//write binary blob to file
		std::fstream fs(file, std::ios::binary | std::ios::in);

		if (!fs.good()) {
			std::cout << "Unable to open file: " << file << std::endl;
			return false;
		}

		constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);
		char header_buffer[header_size]{};
		try {
			fs.read(header_buffer, header_size);
		}
		catch (...) {
			std::cout << "Error reading file: " << file << std::endl;
			fs.close();
			return false;
		}

		if (fs.fail() || fs.eof()) {
			std::cout << "Unknown file format: " << file << std::endl;
			fs.close();
			return false;
		}

		if (header_buffer[0] != 'F' || header_buffer[1] != 'N' || header_buffer[2] != 'T') {
			std::cout << "Not a proper fnt file: " << file << std::endl;
			fs.close();
			return false;
		}
		auto vers = version_number;
		vers = *((decltype(version_number)*)(header_buffer + 3));
		if (vers != version_number) {
			std::cout << "Wrong version, please recompile: " << file << std::endl;
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

	void FontData::unpack_data(char* const buffer, uint64_t& offset) {
		atlasSize = 0;
		if (atlas) {
			delete[] atlas;
		}

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
	}

	uint64_t FontData::get_fnt_size() const {
		uint64_t size = 0;

		size += 
			+ sizeof(unsigned char)		//num glyphs(should be 95 always)
			+ sizeof(unsigned char)		//size of glyph(should be 8 always)
			+ (8 * 95)					//glyph data
			+ sizeof(unsigned short)	//atlas size
			+ sizeof(unsigned char)		//font size
			+ sizeof(unsigned char)		//padding
			+ atlasSize * atlasSize
			;

		return size;
	}

	bool FontData::Load_TTF_File(std::string const& file_path) {
		std::filesystem::path path(file_path);
		if (!std::filesystem::is_regular_file(path)) {
			return false;
		}

		int file_size = std::filesystem::file_size(file_path);
		unsigned char* file_buffer = new unsigned char[file_size];
		std::fstream fs(file_path, std::ios_base::binary|std::ios_base::in);
		fs.read((char*)file_buffer, file_size);
		fs.close();

		if (!stbtt_InitFont(&font_info, file_buffer, 0)) {
			delete[] file_buffer;
			return false;
		}


		//font_size = 50;

		float scale = stbtt_ScaleForPixelHeight(&font_info, (float)font_size);
		//padding = 2;

		unsigned char onEdge = 128;
		float pixel_dist_scale = 64.f;
		//int w, h, xoff, yoff;
		for (char ch = 32; ch < 127; ++ch) {
			GlyphData glyph{};
			int w{}, h{}, xoff{}, yoff{}, advance{};
			glyph.bitmap = stbtt_GetCodepointSDF(&font_info, scale, ch, padding, onEdge, pixel_dist_scale, &w, &h, &xoff, &yoff);
			glyph.w = w;
			glyph.h = h;

			//align offset so position is based on center of quad
			glyph.xoff = xoff + w / 2;
			glyph.yoff = yoff + h / 2;
			stbtt_GetCodepointHMetrics(&font_info, ch, &advance, NULL);
			glyph.advance = advance * scale;
			glyph_datas[ch] = glyph;
		}

		////sanity check
		//stbi_write_png("../Asset/sdf_test.png", w, h, 1, bitmap, 0);

		delete[] file_buffer;

		PackAtlas();

#if _DEBUG
		stbi_write_png("../Asset/atlas_test.png", atlasSize, atlasSize, 1, atlas, 0);
#endif
		return true;
	}

	void FontData::PackAtlas() {
		atlasSize = 0;
		if (atlas) {
			delete[] atlas;
		}
		//not gona use the rect pack or modify it to work with sdf vers, so just pack inefficiently
		//each glyph uses max (font size + padding * 2)^2 number of pixels
		//glyph count is fixed at 94 chars + space char
		//closest square root of 94 is 10 rounded up
		//so width and height of atlas should be 10 * (font_size + padding * 2)

		constexpr int max_glyphs_row = 10;
		int glyphWidth = (font_size + padding * 2);
		atlasSize = max_glyphs_row * glyphWidth;
		atlas = new unsigned char[atlasSize * atlasSize] {};

		//for now assume row major, ch = 33 skips space(32)
		int glyphs_placed = 0;
		int row = 0;
		for (char ch = 33; ch < 127; ++ch) {
			GlyphData const& glyph = glyph_datas.at(ch);

			int start_x = glyphs_placed % max_glyphs_row * glyphWidth;
			int start_y = row * glyphWidth;

			for (char w{}; w < glyph.w; ++w) {
				for (char h{}; h < glyph.h; ++h) {
					atlas[(start_x + w) + (start_y + h) * atlasSize] = glyph.bitmap[w + h * glyph.w];
				}
			}

			++glyphs_placed;
			if (!(glyphs_placed % max_glyphs_row)) {
				++row;
			}
		}
	}

	void FontData::Free() {
		if (atlas) {
			delete[] atlas;
		}

		for (auto& glyph : glyph_datas) {
			stbtt_FreeSDF(glyph.second.bitmap, font_info.userdata);
		}
	}
}