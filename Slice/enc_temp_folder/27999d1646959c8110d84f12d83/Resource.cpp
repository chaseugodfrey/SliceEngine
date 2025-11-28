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
#include <Serializer/JSONSerializer.h>

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
				t->LoadColorTexture(1.f, 1.f, 1.f, 1.f);
				break;
			default:
				return nullptr;
				break;
			}
		}
		else
			SliceEngineTypes::Texture::LoadTexture(t.get(), path);
		return t;
	}

	void Type<SliceEngineTypes::Texture>::Destroy(SliceEngineTypes::Texture& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyTexture();	//calls glDeleteTextures
	}

	void Type<SliceEngineTypes::Texture>::Reload(SliceEngineTypes::Texture* resource, ResourceManager& mgr, const std::string& path)
	{
	}

	//Shader
	std::unique_ptr<SliceEngineTypes::Shader> Type<SliceEngineTypes::Shader>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		return std::make_unique<SliceEngineTypes::Shader>( SliceEngineTypes::Shader::LoadShader(path));
	}

	void Type<SliceEngineTypes::Shader>::Destroy(SliceEngineTypes::Shader& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyShader();	//calls glDeleteProgram
	}

	void Type<SliceEngineTypes::Shader>::Reload(SliceEngineTypes::Shader* resource, ResourceManager& mgr, const std::string& path)
	{
	}
	// Vertex Shader
	std::unique_ptr<SliceEngineTypes::VertShader> Type<SliceEngineTypes::VertShader>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		return std::make_unique<SliceEngineTypes::VertShader>( SliceEngineTypes::VertShader::LoadVertShader(path));
	}

	void Type<SliceEngineTypes::VertShader>::Destroy(SliceEngineTypes::VertShader& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyVertShader();	//calls glDeleteShader
	}

	void Type<SliceEngineTypes::VertShader>::Reload(SliceEngineTypes::VertShader* resource, ResourceManager& mgr, const std::string& path)
	{
	}
	// Geometry Shader
	std::unique_ptr<SliceEngineTypes::GeomShader> Type<SliceEngineTypes::GeomShader>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		return std::make_unique<SliceEngineTypes::GeomShader>( SliceEngineTypes::GeomShader::LoadGeomShader(path));
	}

	void Type<SliceEngineTypes::GeomShader>::Destroy(SliceEngineTypes::GeomShader& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyGeomShader();	//calls glDeleteShader
	}

	void Type<SliceEngineTypes::GeomShader>::Reload(SliceEngineTypes::GeomShader* resource, ResourceManager& mgr, const std::string& path)
	{
	}
	// Fragment Shader
	std::unique_ptr<SliceEngineTypes::FragShader> Type<SliceEngineTypes::FragShader>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		return std::make_unique<SliceEngineTypes::FragShader>( SliceEngineTypes::FragShader::LoadFragShader(path));
	}

	void Type<SliceEngineTypes::FragShader>::Destroy(SliceEngineTypes::FragShader& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyFragShader();	//calls glDeleteShader
	}

	void Type<SliceEngineTypes::FragShader>::Reload(SliceEngineTypes::FragShader* resource, ResourceManager& mgr, const std::string& path)
	{
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

	void Type<SliceEngineTypes::Material>::Reload(SliceEngineTypes::Material* materialToReload, ResourceManager& mgr, const std::string& path)
	{
		std::ifstream file(path);
		if (!file.is_open())
		{
			SLICE_LOG_ERROR("Could not open material file for reload: " + path);
			return;
		}

		nlohmann::json materialJson;
		try
		{
			materialJson = nlohmann::json::parse(file);
		}
		catch (nlohmann::json::parse_error& e)
		{
			SLICE_LOG_ERROR("Invalid material JSON file for reload: " + path + e.what());
			return;
		}

		try
		{
			
			GUID newAlbedoGUID = (GUID)materialJson["albedo"].get<uint64_t>();
			float newRoughness = materialJson["roughness"].get<float>();
			float newMetallic = materialJson["metallic"].get<float>();
			glm::from_json(materialJson["color"], materialToReload->color);

			
			materialToReload->roughness = newRoughness;
			materialToReload->metallic = newMetallic;

			
			GUID oldAlbedoGUID = materialToReload->albedo.getGUID();

			
			if (oldAlbedoGUID != newAlbedoGUID)
			{
				materialToReload->albedo = mgr.get<SliceEngineTypes::Texture>(newAlbedoGUID);
			}
			else
			{
				// The texture is the same. DO NOTHING to the handle.
			}
		}
		catch (nlohmann::json::exception& e)
		{
			SLICE_LOG_ERROR("Error parsing reloaded material properties from file: " + path + ". " + e.what());
		}
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

	void Type<SliceEngineTypes::Model>::Reload(SliceEngineTypes::Model* resource, ResourceManager& mgr, const std::string& path)
	{
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

	void Type<SliceEngineTypes::Scene>::Reload(SliceEngineTypes::Scene* resource, ResourceManager& mgr, const std::string& path)
	{
	}

	//Audio
	std::unique_ptr<SliceEngineTypes::Audio> Type<SliceEngineTypes::Audio>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		auto audio = std::make_unique<SliceEngineTypes::Audio>();
		
		if (std::filesystem::exists(path) && audio->GetSound() == nullptr)
		{
			audio->LoadAudioResource(path);
		}
		
		return audio;
	}

	void Type<SliceEngineTypes::Audio>::Destroy(SliceEngineTypes::Audio& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyAudio();
	}

	void Type<SliceEngineTypes::Audio>::Reload(SliceEngineTypes::Audio* resource, ResourceManager& mgr, const std::string& path)
	{
		resource->DestroyAudio();

		resource->LoadAudioResource(path);


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

	void Type<SliceEngineTypes::Prefab>::Reload(SliceEngineTypes::Prefab* resource, ResourceManager& mgr, const std::string& path)
	{
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

	void Type<SliceEngineTypes::Skeleton>::Reload(SliceEngineTypes::Skeleton* resource, ResourceManager& mgr, const std::string& path)
	{
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

	void Type<SliceEngineTypes::AnimationPackage>::Destroy(SliceEngineTypes::AnimationPackage& resource, ResourceManager& resourceMgr)
	{
		//nothing to really delete too
	}

	void Type<SliceEngineTypes::AnimationPackage>::Reload(SliceEngineTypes::AnimationPackage* resource, ResourceManager& mgr, const std::string& path)
	{
	}

	//Controller
	std::unique_ptr<SliceEngineTypes::StateMachine> Type<SliceEngineTypes::StateMachine>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		return std::make_unique<SliceEngineTypes::StateMachine>(SliceEngineTypes::StateMachine::LoadStateMachineResource(path));
	}

	void Type<SliceEngineTypes::StateMachine>::Destroy(SliceEngineTypes::StateMachine& resource, ResourceManager& resourceMgr)
	{
		//nothing to really delete too
	}
	void Type<SliceEngineTypes::StateMachine>::Reload(SliceEngineTypes::StateMachine* resource, ResourceManager& mgr, const std::string& path)
	{
	}
}