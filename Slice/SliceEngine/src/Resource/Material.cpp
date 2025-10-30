/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Material.cpp
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Materials

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Material.h"
#include <fstream>
#include <GL/glew.h>
#include <sstream>

namespace SliceEngine
{
	namespace SliceEngineTypes {


		Material Material::LoadMaterial(std::string const& filepath) {
			// filepath to material.mat in resource folder

			Material temp;
			temp.albedo = (GUID)10452617659721414527;
			temp.roughness = 0.6f;
			temp.metallic = 1.7f;

			return temp;
		}

		void Material::DestroyMaterial() {
		
		}
	}
}