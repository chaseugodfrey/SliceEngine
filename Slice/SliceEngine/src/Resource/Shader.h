/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Shader.h
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Shaders

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef SHADER_H
#define SHADER_H

#include <string>

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		/*
		* Shaders are kind of a hack now because they need multiple
		* source files to compile
		* and they need to be compiled after building the exe
		* so it needs to be done on engine code and cant precompile
		*/
		class Shader {
			//ngl no idea whr to put this so ill put this for now
			static unsigned int CompileShader(const std::string& vertFile, const std::string& fragFile);

		public:

			unsigned int s;

			//takes in a shader resource file that links to shader files
			//and compile
			static Shader LoadShader(std::string const&);
			void DestroyShader();
		};
	}
}

#endif
