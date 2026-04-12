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
#include "Core/Core.h"
#include "Systems/SceneSystem.h"
#include "Font.h"

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
			case DefaultResourceIDs::COLOR_NORMAL_DEFAULT:
				t->LoadColorTexture(0.5f, 0.5f, 1.f, 1.f);
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
		resource->DestroyTexture();
		SliceEngineTypes::Texture::LoadTexture(resource, path);
		//resource->LoadTexture
		//resource.load
	//	resource = Load(mgr, path);
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

	// Custom Shader
	std::unique_ptr<SliceEngineTypes::CustomShader> Type<SliceEngineTypes::CustomShader>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		std::filesystem::path file(path);

		if (std::filesystem::exists(path))
			return std::make_unique<SliceEngineTypes::CustomShader>(SliceEngineTypes::CustomShader::LoadCShader(path));

		// load default model
		uint64_t defaultID = std::stoull(path);

		if (defaultID == Type<SliceEngineTypes::CustomShader>::defaultResourceGUID)
		{
			auto t = std::make_unique<SliceEngineTypes::CustomShader>();
			t->LoadDefault();
			return t;
		}

		return nullptr;
	}

	void Type<SliceEngineTypes::CustomShader>::Destroy(SliceEngineTypes::CustomShader& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyCShader();	//calls glDeleteProgram
	}

	void Type<SliceEngineTypes::CustomShader>::Reload(SliceEngineTypes::CustomShader* resource, ResourceManager& mgr, const std::string& path)
	{
		resource->DestroyCShader();	//calls glDeleteProgram
		auto newResource = resource->LoadCShader(path);
		resource->opaqueS = newResource.opaqueS;
		resource->translucentS = newResource.translucentS;
		resource->dataIn = newResource.dataIn;

		std::string shdrGUID = path.substr(path.find_first_of('/') + 1);
		shdrGUID = shdrGUID.substr(0, shdrGUID.find_first_of('.'));

		for (auto& [name, id] : mgr.mFileNameToGUID)
		{
			if (name.find(".mat") != std::string::npos)
			{
				auto mat = mgr.get<SliceEngineTypes::Material>(id);
				if (mat.get()->shader.getGUID().GetGUID() == std::stoull(shdrGUID))
				{
					mat->isShaderUpdated = true;
					mgr.ReloadResourceInPlace(id);
				}
			}
		}
		// safety check for default resource ID for material
		mgr.ReloadResourceInPlace((GUID)Type<SliceEngineTypes::Material>::defaultResourceGUID);
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
		auto t = std::make_unique<SliceEngineTypes::Material>();
		std::filesystem::path file(path);

		if (!std::filesystem::exists(path))
		{
			// load default model
			uint64_t defaultID = std::stoull(path);

			switch (defaultID)
			{
			case Type<SliceEngineTypes::Material>::defaultResourceGUID:
				t->LoadDefault();
				
				break;
			default:
				return nullptr;
				break;
			}
		}
		else
			return std::make_unique<SliceEngineTypes::Material>(SliceEngineTypes::Material::LoadMaterial(path));

		return t;
	}

	void Type<SliceEngineTypes::Material>::Destroy(SliceEngineTypes::Material& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyMaterial();
	}

	void Type<SliceEngineTypes::Material>::Reload(SliceEngineTypes::Material* materialToReload, ResourceManager& mgr, const std::string& path)
	{
		// the path passed in is just the defualt GUID 10819322238111217941
		// so itll fail to open
		// so i'm gonna add some check if its default
		// and change to use LoadMaterial and LoadDefault in material so taht I can reuse the code below w/o copy pasting it all
		SliceEngineTypes::Material loadedMaterialData;

		std::ifstream file(path);
		if (!file.is_open())
		{
			// if path doesnt exist check if its a default resource
			if (path == std::to_string(Type<SliceEngineTypes::Material>::defaultResourceGUID))
			{
				SLICE_LOG("Loading default material for reload: " + path);
				loadedMaterialData.LoadDefault();
			}
			else
				SLICE_LOG_ERROR("Could not open material file for reload: " + path);
		}
		else
		{
			// if file can open then its a valid material so load material from json
			loadedMaterialData = SliceEngineTypes::Material::LoadMaterial(path);
			file.close();
		}

		GUID newShaderGUID = loadedMaterialData.shader.getGUID();//(GUID)materialJson["shader"].get<uint64_t>();

		materialToReload->color = loadedMaterialData.color;
		materialToReload->color2 = loadedMaterialData.color2;
		materialToReload->isTranslucent = loadedMaterialData.isTranslucent;
		materialToReload->isIgnoreLighting = loadedMaterialData.isIgnoreLighting;
		auto oldData = materialToReload->data; // Do I even need old Data? This whole reload function calls when shader change, and when material changes
		materialToReload->data.clear();

		if (newShaderGUID != materialToReload->shader.getGUID())
			materialToReload->shader = mgr.get<SliceEngineTypes::CustomShader>(newShaderGUID);

		for (auto& [name, data] : loadedMaterialData.data)
		{
			if (materialToReload->isShaderUpdated && oldData.contains(name))
				materialToReload->data[name] = oldData[name];
			else
				materialToReload->data[name] = data;
		}
		materialToReload->isShaderUpdated = false;
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
			case DefaultResourceIDs::CYLINDER_DEFAULT:
				m->LoadDefaultCylinderModel();
				break;
			case DefaultResourceIDs::QUAD_DEFAULT:
				m->LoadDefaultQuadModel();
				break;
			case DefaultResourceIDs::PLANE_DEFAULT:
				m->LoadDefaultTerrain(100);
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
		resource->DestroyModel();

		std::filesystem::path file(path);
		if (!std::filesystem::exists(path))
		{
			// load default model
			uint64_t defaultID = std::stoull(path);

			switch (defaultID)
			{
			case DefaultResourceIDs::CUBE_DEFAULT:
				resource->LoadDefaultCubeModel();
				break;
			case DefaultResourceIDs::SPHERE_DEFAULT:
				resource->LoadDefaultSphereModel();
				break;
			case DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT:
				resource->LoadDefaultSphereModel(5, 7);
				break;
			case DefaultResourceIDs::CAPSULE_DEFAULT:
				resource->LoadDefaultCapsuleModel();
				break;
			case DefaultResourceIDs::CYLINDER_DEFAULT:
				resource->LoadDefaultCylinderModel();
				break;
			case DefaultResourceIDs::QUAD_DEFAULT:
				resource->LoadDefaultQuadModel();
				break;
			case DefaultResourceIDs::PLANE_DEFAULT:
				resource->LoadDefaultTerrain(100);
				break;
			case DefaultResourceIDs::LINE_DEFAULT:
				resource->LoadDefaultLineModel();
				break;
			case DefaultResourceIDs::FRUSTRUM_DEFAULT:
				resource->LoadDefaultFrustrumModel();
				break;
			default:
				delete resource;
				break;
			}
		}
		if (file.extension() == ".mdl") {
			if (!resource->LoadModelResource(path)) {
				//delete m;
				delete resource;
			}
		}
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
		auto sceneSystem = Core::GetInstance()->GetSceneSystem();

		if (sceneSystem->mCurrentState == SceneState::PLAY_SCENE)
		{
			SLICE_LOG_WARNING("Ignored Scene Hot-Reload because Engine is in Play Mode: " + path);
			return;
		}

		std::filesystem::path reloadedPath(path);
		std::filesystem::path currentPath = sceneSystem->GetCurrentScenePath();

		bool isCurrentScene = false;
		try
		{
			if (std::filesystem::exists(reloadedPath) && !currentPath.empty())
			{
				isCurrentScene = std::filesystem::equivalent(reloadedPath, currentPath);
			}
		}
		catch (...)
		{
			// Handle edge cases where paths might be invalid
			isCurrentScene = (reloadedPath == currentPath);
		}

		if (isCurrentScene)
		{
			SLICE_LOG("Hot-Reloading Current Scene: " + path);

			Core::GetInstance()->mFactory.ClearGameObjects();
			Core::GetInstance()->mFactory.UpdateDestroyed(); // Force immediate cleanup

			auto map = JSONSerializer::DeserializeScene(path);

			Core::GetInstance()->mFactory.BuildSceneGraph(map);

			OnSceneLoadedEvent event;
			event.isSceneLoaded = true;
			event.scenePath = path;
			EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(event);
		}
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

	//Anims Package
	std::unique_ptr<SliceEngineTypes::SequencePackage> Type<SliceEngineTypes::SequencePackage>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		auto anim = std::make_unique<SliceEngineTypes::SequencePackage>();
		if (!anim->LoadSequencePkgResource(path)) {
			SLICE_LOG("Load seq pkg failed");
			return nullptr;
		}
		SLICE_LOG("Load seq pkg success");
		return anim;
	}

	void Type<SliceEngineTypes::SequencePackage>::Destroy(SliceEngineTypes::SequencePackage& resource, ResourceManager& resourceMgr)
	{
		//nothing to really delete too
	}

	void Type<SliceEngineTypes::SequencePackage>::Reload(SliceEngineTypes::SequencePackage* resource, ResourceManager& mgr, const std::string& path)
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

	//Font
	std::unique_ptr<SliceEngineTypes::Font_Data> Type<SliceEngineTypes::Font_Data>::Load(ResourceManager& resourceMgr, const std::string& path)
	{
		auto font = std::make_unique<SliceEngineTypes::Font_Data>();
		if (!std::filesystem::exists(path)) {
			font->InitializeDefault();
		}
		else {
			if (!font->LoadFontResource(path)) {
				return nullptr;
			}
		}
		return font;
	}

	void Type<SliceEngineTypes::Font_Data>::Destroy(SliceEngineTypes::Font_Data& resource, ResourceManager& resourceMgr)
	{
		resource.DestroyFontResource();
	}

	void Type<SliceEngineTypes::Font_Data>::Reload(SliceEngineTypes::Font_Data* resource, ResourceManager& mgr, const std::string& path)
	{
		resource->DestroyFontResource();
		if (!std::filesystem::exists(path)) {
			resource->InitializeDefault();
		}
		else {
			resource->LoadFontResource(path);
		}
	}

	/*void Type<SliceEngineTypes::Font_Data>::Reload(SliceEngineTypes::Font_Data* resource, ResourceManager& mgr, const std::string& path)
	{
	}*/
}