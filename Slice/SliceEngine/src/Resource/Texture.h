#ifndef TEXTURE_H
#define TEXTURE_H

#include <filesystem>

namespace SliceEngine
{
	// Define what a texture would have here 
	class Texture
	{
		//ngl no idea whr to put this so ill put this for now
		static unsigned int LoadTexture(std::filesystem::path const&);
	};
}

#endif
