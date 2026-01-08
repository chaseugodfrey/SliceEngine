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
#include <Serializer/JSONSerializer.h>


namespace SliceEngine
{
	namespace SliceEngineTypes {


		Material Material::LoadMaterial(std::string const& filepath) {
			Material temp;
			// figure out default textures
			temp.albedo.mGUID = (GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT;
			temp.albedo = Core::GetInstance()->GetResourceManager()->get<Texture>(temp.albedo.mGUID);
			temp.roughness = 0.6f;
			temp.metallic = 0.f;
			temp.color = glm::vec4(1.f);

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

			// cause color is a vec 4
			if (materialJson.contains("color") && materialJson["color"].is_array() && materialJson["color"].size() == 4)
			{
				glm::from_json(materialJson["color"], temp.color);
			}


			return temp;
		}

		void Material::LoadDefault()
		 {
			albedo.mGUID = (GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT;
			albedo = Core::GetInstance()->GetResourceManager()->get<Texture>(albedo.mGUID);
			roughness = 0.6f;
			metallic = 0.f;
			color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		 }

		void Material::DestroyMaterial() {
			albedo.Release();
		}

		
	}
}