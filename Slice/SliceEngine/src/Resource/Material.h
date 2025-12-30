/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Material.h
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Material

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef MATERIALS_H
#define MATERIALS_H

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
		class Material {
		public:
			Handle<Texture> albedo;

			//GUID albedo;
			glm::vec3 color;
			float roughness;
			float metallic;

			//takes in a shader resource file that links to shader files
			//and compile
			static Material LoadMaterial(std::string const&);
			void LoadDefault();
			void DestroyMaterial();

		};
	}
}

#endif
