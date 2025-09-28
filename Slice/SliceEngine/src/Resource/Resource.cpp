#include <pch.h>
#include "Resource.h"

#include "Texture.h"
#include "Shader.h"
#include "Model.h"

namespace SliceEngine
{
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
		return new SliceEngineTypes::Model{ SliceEngineTypes::Model::LoadModel(path) };
	}

	void Type<SliceEngineTypes::Model>::Destroy(SliceEngineTypes::Model& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyModel();	//calls glDeleteBuffer, glDeleteVertexArray
	}
}