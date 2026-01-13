/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Font.h
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads .fnt Font assets

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef FONT_H
#define FONT_H

#include <vector>
#include <string>

void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

namespace SliceEngine
{
	namespace SliceEngineTypes
	{	
		/*
		* The glyph parameters are defined with the origin at the top left
		*/
		struct GlyphData {
			char w, h, xoff, yoff;
			float advance;
		};

		struct Atlas_UV {
			float u_start{}, u_end{}, v_start, v_end{};
		};

		struct Font_Data {

			unsigned int atlas_texture;
			std::unordered_map<char, Atlas_UV> atlas_uvs;
			std::unordered_map<char, GlyphData> glyph_datas;

			//unsigned short atlasSize;

			unsigned char font_size{};
			unsigned char padding;

			bool LoadFontResource(std::string const&);
			void DestroyFontResource();
		private:
			bool unpack_data(char* const buffer, uint64_t& offset);
		};

	}
}

#endif