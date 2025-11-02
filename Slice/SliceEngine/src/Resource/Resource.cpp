/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Resource.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Loads resources

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "ResourceManager.h"

#include "Texture.h"
#include "Shader.h"
#include "Material.h"
#include "Model.h"
#include "Scene.h"
#include "Audio.h"
#include "Prefab.h"

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
		
		//return  std::make_unique<SliceEngineTypes::Texture>(SliceEngineTypes::Texture::LoadTexture(path));
		auto t = std::make_unique<SliceEngineTypes::Texture>();
		std::filesystem::path file(path);
		if (!std::filesystem::exists(path))
		{
			// load default model
			uint64_t defaultID = std::stoull(path);

			switch (defaultID)
			{
			case DefaultResourceIDs::COLOR_DEADED_DEFAULT:
				t->LoadColorTexture(0.8705882f, 0.678431f, 0.92941f, 1.f);
				break;
			default:
				return nullptr;
				break;
			}
		}
		else
			t->LoadTexture(path);
		return t;
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

	//Material
	std::unique_ptr<SliceEngineTypes::Material> Type<SliceEngineTypes::Material>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		return std::make_unique<SliceEngineTypes::Material>( SliceEngineTypes::Material::LoadMaterial(path));
	}

	void Type<SliceEngineTypes::Material>::Destroy(SliceEngineTypes::Material& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyMaterial();
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
			case DefaultResourceIDs::SPHERE_DEFAULT:
				m->LoadDefaultSphereModel();
				break;
			case DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT:
				m->LoadDefaultSphereModel(5,7);
				break;
			case DefaultResourceIDs::CAPSULE_DEFAULT:
				m->LoadDefaultCapsuleModel();
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

	//Audio
	std::unique_ptr<SliceEngineTypes::Audio> Type<SliceEngineTypes::Audio>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		auto audio = std::make_unique<SliceEngineTypes::Audio>();
		
		if (std::filesystem::exists(path))
		{
			audio->LoadAudioResource(path);
		}
		
		return audio;
	}

	void Type<SliceEngineTypes::Audio>::Destroy(SliceEngineTypes::Audio& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyAudio();
	}
	
	std::unique_ptr<SliceEngineTypes::Prefab> Type<SliceEngineTypes::Prefab>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
			std::filesystem::path file(path);
			GUID guid = GUID::FromString(file.stem().string());
			
			auto prefab = std::make_unique<SliceEngineTypes::Prefab>(guid, path);
			return prefab;
	}

	void Type<SliceEngineTypes::Prefab>::Destroy(SliceEngineTypes::Prefab& resource, ResourceManager& resourceMgr)
	{
		// nth to destroy
	}

	//Skeleton
	std::unique_ptr<SliceEngineTypes::Skeleton> Type<SliceEngineTypes::Skeleton>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		auto skele = std::make_unique<SliceEngineTypes::Skeleton>();
		if (!skele->LoadSkeletonResource(path)) {
			return nullptr;
		}
		return skele;
	}

	void Type<SliceEngineTypes::Skeleton>::Destroy(SliceEngineTypes::Skeleton& resource, ResourceManager& resourceMgr)
	{
		//resource.d();	//nothing to delete really
	}

	//Animation Package
	std::unique_ptr<SliceEngineTypes::AnimationPackage> Type<SliceEngineTypes::AnimationPackage>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		auto anim = std::make_unique<SliceEngineTypes::AnimationPackage>();
		if (!anim->LoadAnimPackageResource(path)) {
			return nullptr;
		}
		return anim;
	}

	void Type<SliceEngineTypes::Texture>::Destroy(SliceEngineTypes::Texture& resource, ResourceManager& resourceMgr)
	{
		//nothing to really delete too
	}
}