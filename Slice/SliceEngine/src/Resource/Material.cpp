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
#include "Core/Core.h"


namespace SliceEngine
{
	namespace SliceEngineTypes {


		Material Material::LoadMaterial(std::string const& filepath) {
			Material temp;
			// figure out default textures
			temp.albedo.mGUID = (GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT;
			temp.albedo = Core::GetInstance()->GetResourceManager()->get<Texture>(temp.albedo.mGUID);
			temp.roughness = 0.6f;
			temp.metallic = 1.7f;

			// filepath to material.mat in resource folder
			std::ifstream file(filepath);
			if (!file.is_open())
			{
				return temp;
			}

			nlohmann::json materialJson;
			try
			{
				materialJson = nlohmann::json::parse(file);
			}
			catch (nlohmann::json::parse_error& e)
			{
				SLICE_LOG_ERROR("Invalid material JSON file" + std::string(e.what()));

				return temp;
			}
			// shouldn't need a [0]. Need check how the material file is created
			temp.albedo.mGUID = (GUID)materialJson["albedo"].get<uint64_t>();
			temp.albedo = Core::GetInstance()->GetResourceManager()->get<Texture>(temp.albedo.mGUID);
			temp.roughness = materialJson["roughness"].get<float>();
			temp.metallic = materialJson["metallic"].get<float>();

			return temp;
		}

		void Material::DestroyMaterial() {
			albedo.Release();
		}

		
	}
}