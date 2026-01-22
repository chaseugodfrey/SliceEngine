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
			temp.shader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::CustomShader>("CustomShader/default.cshader");
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
			temp.shader.mGUID = (GUID)materialJson["shader"].get<uint64_t>();
			temp.shader = Core::GetInstance()->GetResourceManager()->get<CustomShader>(temp.shader.mGUID);

			if (materialJson.contains("color") && materialJson["color"].is_array() && materialJson["color"].size() == 4) // cause color is a vec 4
				glm::from_json(materialJson["color"], temp.color);

			materialJson["floats"].get_to(temp.floatDat);
			materialJson["ints"].get_to(temp.intDat);
			materialJson["uints"].get_to(temp.uintDat);
			materialJson["bools"].get_to(temp.boolDat);

			return temp;
		}

		void Material::LoadDefault()
		 {
			albedo.mGUID = (GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT;
			albedo = Core::GetInstance()->GetResourceManager()->get<Texture>(albedo.mGUID);
			shader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::CustomShader>("CustomShader/default.cshader");
			color = glm::vec4(1.f, 1.f, 1.f, 1.f);
			for (auto& i : shader.get()->dataIn)
			{
				switch (i.dataType)
				{
				case CustomShader::SP_TYPE::BOOL:
					boolDat.push_back(i.baseData.sp_bool);
					break;
				case CustomShader::SP_TYPE::UINT:
					uintDat.push_back(i.baseData.sp_uint);
					break;
				case CustomShader::SP_TYPE::INT:
					intDat.push_back(i.baseData.sp_int);
					break;
				case CustomShader::SP_TYPE::FLOAT:
					floatDat.push_back(i.baseData.sp_float);
					break;
				}
			}
		 }

		void Material::DestroyMaterial() {
			albedo.Release();
		}

		
	}
}