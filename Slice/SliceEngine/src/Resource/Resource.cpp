#include <pch.h>
#include "ResourceManager.h"

#include "Texture.h"
#include "Shader.h"
#include "Model.h"
#include "Scene.h"

namespace SliceEngine
{
	SliceEngineTypes::Scene* Type<SliceEngineTypes::Scene>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		return 
	}
	//SliceEngineTypes::Texture* Type<SliceEngineTypes::Texture>::Load(ResourceManager& resourceMgr, uint64_t resourceID)
	//{

	//}

	//Texture
	SliceEngineTypes::Texture* Type<SliceEngineTypes::Texture>::Load(ResourceManager& resourceMgr, const std::string& path)
	{/*
		unsigned int texture_id = SliceEngineTypes::Texture::LoadTexture(path);
		*/
		
		return new SliceEngineTypes::Texture{ SliceEngineTypes::Texture::LoadTexture(path) };
	}

	void Type<SliceEngineTypes::Texture>::Destroy(SliceEngineTypes::Texture& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyTexture();	//calls glDeleteTextures
	}

	//Shader
	SliceEngineTypes::Shader* Type<SliceEngineTypes::Shader>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		//unsigned int texture_id = SliceEngineTypes::Texture::LoadTexture(path);


		return new SliceEngineTypes::Shader{ SliceEngineTypes::Shader::LoadShader(path) };
	}

	void Type<SliceEngineTypes::Shader>::Destroy(SliceEngineTypes::Shader& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyShader();	//calls glDeleteProgram
	}

	//Model
	SliceEngineTypes::Model* Type<SliceEngineTypes::Model>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		auto* m = new SliceEngineTypes::Model();
		if (!m->LoadModel(path)) {
			delete m;
			return nullptr;
		}
		//return new SliceEngineTypes::Model{SliceEngineTypes::Model::LoadModel(path)};
		return m;
	}

	void Type<SliceEngineTypes::Model>::Destroy(SliceEngineTypes::Model& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyModel();	//calls glDeleteBuffer, glDeleteVertexArray
	}


}