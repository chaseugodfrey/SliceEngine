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
			//temp.albedo.mGUID = (GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT;
			//temp.albedo = Core::GetInstance()->GetResourceManager()->get<Texture>(temp.albedo.mGUID);
			temp.shader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::CustomShader>((GUID)DefaultResourceIDs::CSHADER_DEFAULT);
			temp.color = temp.color2 = glm::vec4(1.f);
			temp.isTranslucent = false;
			temp.isIgnoreLighting = false;

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
				file.close();
				SLICE_LOG_ERROR("Invalid material JSON file" + std::string(e.what()));

				return temp;
			}
			file.close();
			// shouldn't need a [0]. Need check how the material file is created
			//temp.albedo.mGUID = (GUID)materialJson["albedo"].get<uint64_t>();
			//temp.albedo = Core::GetInstance()->GetResourceManager()->get<Texture>(temp.albedo.mGUID);
			auto newShadrGUID = (GUID)materialJson["shader"].get<uint64_t>();
			temp.shader = Core::GetInstance()->GetResourceManager()->get<CustomShader>(newShadrGUID);

			if (materialJson.contains("color") && materialJson["color"].is_array() && materialJson["color"].size() == 4) // cause color is a vec 4
				glm::from_json(materialJson["color"], temp.color);

			if (materialJson.contains("color2") && materialJson["color2"].is_array() && materialJson["color2"].size() == 4) // cause color is a vec 4
				glm::from_json(materialJson["color2"], temp.color2);

			if (materialJson.contains("translucency"))
				temp.isTranslucent = materialJson["translucency"];

			if (materialJson.contains("ignoreLights"))
				temp.isIgnoreLighting = materialJson["ignoreLights"];

			for (auto& i : temp.shader.get()->dataIn)
			{
				if (materialJson["data"].contains(i.name))
				{
					switch (i.dataType)
					{
					case SliceEngineTypes::CustomShader::SP_TYPE::BOOL:
					{
						bool b = materialJson["data"][i.name];
						temp.data[i.name] = b;
						break;
					}
					case SliceEngineTypes::CustomShader::SP_TYPE::UINT:
					{
						uint32_t b = materialJson["data"][i.name];
						temp.data[i.name] = b;
						break;
					}
					case SliceEngineTypes::CustomShader::SP_TYPE::INT:
					{
						int32_t b = materialJson["data"][i.name];
						temp.data[i.name] = b;
						break;
					}
					case SliceEngineTypes::CustomShader::SP_TYPE::FLOAT:
					{
						float b = materialJson["data"][i.name];
						temp.data[i.name] = b;
						break;
					}
					case SliceEngineTypes::CustomShader::SP_TYPE::TEXTURE:
					{
						uint64_t b = materialJson["data"][i.name];
						temp.data[i.name] = b;
						break;
					}
					}
				}
				else
					temp.data[i.name] = i.baseData;
			}
			return temp;
		}

		void Material::LoadDefault()
		 {
			//albedo.mGUID = (GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT;
			//albedo = Core::GetInstance()->GetResourceManager()->get<Texture>(albedo.mGUID);
			shader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::CustomShader>((GUID)DefaultResourceIDs::CSHADER_DEFAULT);
			color = color2 = glm::vec4(1.f, 1.f, 1.f, 1.f);
			isTranslucent = false;
			isIgnoreLighting = false;
			for (auto& i : shader.get()->dataIn)
				data[i.name] = i.baseData;
		 }

		void Material::DestroyMaterial() {
			//albedo.Release();
		}
	}
}