/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Texture.h
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Textures

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		class Texture {
		public:
			//ngl no idea whr to put this so ill put this for now
			//just a wrapper for containing funcs ig
			static Texture LoadTexture(std::string const&);
			void DestroyTexture();

			unsigned int texture_id{};
		};
	}
}

#endif
