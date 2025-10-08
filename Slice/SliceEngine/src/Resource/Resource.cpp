/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Resource.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Loads resources

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "ResourceManager.h"

#include "Texture.h"
#include "Shader.h"
#include "Model.h"
#include "Scene.h"

namespace SliceEngine
{
	//SliceEngineTypes::Scene* Type<SliceEngineTypes::Scene>::Load(ResourceManager& resourceMgr, const std::string& path)
	//{
	//	return 
	//}
	//SliceEngineTypes::Texture* Type<SliceEngineTypes::Texture>::Load(ResourceManager& resourceMgr, uint64_t resourceID)
	//{

	//}

	//Texture
	std::unique_ptr<SliceEngineTypes::Texture> Type<SliceEngineTypes::Texture>::Load(ResourceManager& resourceMgr, const std::string& path)
	{/*
		unsigned int texture_id = SliceEngineTypes::Texture::LoadTexture(path);
		*/
		
		return  std::make_unique<SliceEngineTypes::Texture>(SliceEngineTypes::Texture::LoadTexture(path));
	}

	void Type<SliceEngineTypes::Texture>::Destroy(SliceEngineTypes::Texture& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyTexture();	//calls glDeleteTextures
	}

	//Shader
	std::unique_ptr<SliceEngineTypes::Shader> Type<SliceEngineTypes::Shader>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		//unsigned int texture_id = SliceEngineTypes::Texture::LoadTexture(path);


		return std::make_unique<SliceEngineTypes::Shader>( SliceEngineTypes::Shader::LoadShader(path));
	}

	void Type<SliceEngineTypes::Shader>::Destroy(SliceEngineTypes::Shader& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyShader();	//calls glDeleteProgram
	}

	//Model
	std::unique_ptr<SliceEngineTypes::Model> Type<SliceEngineTypes::Model>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		auto m = std::make_unique<SliceEngineTypes::Model>();
		std::filesystem::path file(path);
		if (!std::filesystem::exists(path))
		{
			// load default model
			uint64_t defaultID = std::stoull(path);

			switch (defaultID)
			{
			case DefaultResourceIDs::CUBE_DEFAULT:
				m->LoadDefaultCubeModel();
				break;
			case DefaultResourceIDs::QUAD_DEFAULT:
				m->LoadDefaultQuadModel();
				break;
			case DefaultResourceIDs::LINE_DEFAULT:
				m->LoadDefaultLineModel();
				break;
			case DefaultResourceIDs::FRUSTRUM_DEFAULT:
				m->LoadDefaultFrustrumModel();
				break;
			default:
				return nullptr;
				break;
			}
		}
		if (file.extension() == ".mdl") {
			if (!m->LoadModelResource(path)) {
				//delete m;
				return nullptr;
			}
		}
		//return new SliceEngineTypes::Model{SliceEngineTypes::Model::LoadModel(path)};
		return m;
	}

	void Type<SliceEngineTypes::Model>::Destroy(SliceEngineTypes::Model& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyModel();	//calls glDeleteBuffer, glDeleteVertexArray
	}

	std::unique_ptr<SliceEngineTypes::Scene> Type<SliceEngineTypes::Scene>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		auto scene = std::make_unique<SliceEngineTypes::Scene>(path);
		return scene;
	}

	void Type<SliceEngineTypes::Scene>::Destroy(SliceEngineTypes::Scene& resource, ResourceManager& resourceMgr)
	{
		// scene got nth to destroy that resource manager doesn't do for it
		//resource.DestroyScene();	//calls glDeleteBuffer, glDeleteVertexArray
	}
}