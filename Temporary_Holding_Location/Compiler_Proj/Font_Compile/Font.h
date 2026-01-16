#pragma once

#include <string>
#include <unordered_map>

#define COMPILE_ONLY 1
namespace Font {

	/*
	* The glyph parameters are defined with the origin at the top left
	*/
	struct GlyphData {
		unsigned char* bitmap;
		char w, h, xoff, yoff;	//only really need short because of the reso of our fonts, capped at 200 reso
		float advance;
	};

	struct Atlas_UV {
		float u_start{}, u_end{}, v_start, v_end{};
	};

	struct FontData {
		
		//parse custom file format
		bool Load_Fnt_File(std::string const&);
		//saves current data into a custom file format
		void Save_Fnt_File(std::string const&);

		//loads truetype font
		bool Load_TTF_File(std::string const&);
		void PackAtlas();

		void Free();


		unsigned char* atlas{};
		unsigned short atlasSize;

		unsigned char font_size{50};
		unsigned char padding{2};

		std::unordered_map<char, unsigned int> glyph_texture;
		std::unordered_map<char, Atlas_UV> atlas_uvs;
		unsigned int atlas_texture;

		std::unordered_map<char, GlyphData> glyph_datas;
	private:	//private stuff is used purely for processing and not for rendering in anyway
		uint64_t get_fnt_size() const;
		void pack_data(char* const buffer, uint64_t& offset) const;
		void unpack_data(char* const buffer, uint64_t& offset);
	};

	struct FontRenderer {
		float text_size{32.f};

		struct FontColor {
			float r{ 1.f }, g{ 1.f }, b{ 1.f }, a{ 1.f };
		} font_color;

		std::string text{"Hello World"};
	};


}