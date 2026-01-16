/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Resource.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Loads resources

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef RESOURCE_H
#define RESOURCE_H
#include "ResourceManager.h"
#include <cstdint>
#include <string_view>
#include "Texture.h"
#include "Model.h"
#include "Material.h"
#include "Prefab.h"
#include "Scene.h"
#include "Shader.h"
#include "Audio.h"
#include "Skeleton.h"
#include "StateMachine.h"
#include "Font.h"

namespace SliceEngine
{
	namespace DefaultResourceIDs
	{
		constexpr uint64_t CUBE_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultCube");
		constexpr uint64_t SPHERE_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultSphere");
		constexpr uint64_t SPHERE_LOW_POLY_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultLowPolySphere");
		constexpr uint64_t CAPSULE_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultCapsule");
		constexpr uint64_t LINE_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultLine");
		constexpr uint64_t QUAD_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultQuad");
		constexpr uint64_t FRUSTRUM_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultFrustrum");
		
		constexpr uint64_t COLOR_DEADED_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultColorDEADED");

		constexpr uint64_t FONT_BLANK_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultFont");
	}


	template <>
	struct Type<SliceEngineTypes::Texture>
	{
		// for when we implement meta data files
		//static SliceEngineTypes::Texture* Load(ResourceManager& resourceMgr, uint64_t resourceID);
		// for now load with file name directly
		constexpr static inline uint64_t defaultResourceGUID = DefaultResourceIDs::COLOR_DEADED_DEFAULT;

		static std::unique_ptr<SliceEngineTypes::Texture> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Texture& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::Texture* resource, ResourceManager& mgr, const std::string& path);

	};

	template <>
	struct Type<SliceEngineTypes::Shader>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::Shader> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Shader& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::Shader* resource, ResourceManager& mgr, const std::string& path);

	};

	template <>
	struct Type<SliceEngineTypes::CustomShader>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::CustomShader> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::CustomShader& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::CustomShader* resource, ResourceManager& mgr, const std::string& path);

	};

	template <>
	struct Type<SliceEngineTypes::VertShader>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::VertShader> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::VertShader& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::VertShader* resource, ResourceManager& mgr, const std::string& path);

	};

	template <>
	struct Type<SliceEngineTypes::GeomShader>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::GeomShader> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::GeomShader& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::GeomShader* resource, ResourceManager& mgr, const std::string& path);

	};

	template <>
	struct Type<SliceEngineTypes::FragShader>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::FragShader> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::FragShader& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::FragShader* resource, ResourceManager& mgr, const std::string& path);

	};

	template <>
	struct Type<SliceEngineTypes::Material>
	{
		constexpr static inline uint64_t defaultResourceGUID = 10819322238111217941;

		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::Material> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Material& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::Material* materialToReload, ResourceManager& mgr, const std::string& path);

	};

	template <>
	struct Type<SliceEngineTypes::Model>
	{
		//constexpr static inline uint64_t typeUUID = ResourceTypeIDs::MODEL;
		constexpr static inline uint64_t defaultResourceGUID = DefaultResourceIDs::CUBE_DEFAULT;

		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::Model> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Model& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::Model* resource, ResourceManager& mgr, const std::string& path);

	};

	template <>
	struct Type<SliceEngineTypes::Audio>
	{
		constexpr static inline uint64_t defaultResourceGUID = 13303718109627574413;

		static std::unique_ptr<SliceEngineTypes::Audio> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Audio& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::Audio* resource, ResourceManager& mgr, const std::string& path);
	};

	template <>
	struct Type<SliceEngineTypes::Scene>
	{
		constexpr static inline uint64_t defaultResourceGUID = 13303718109627574413;
		//constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SCENE;
		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::Scene> Load(ResourceManager& resourceMgr, const std::string& path);
		static void Destroy(SliceEngineTypes::Scene& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::Scene* resource, ResourceManager& mgr, const std::string& path);
	};

	template <>
	struct Type<SliceEngineTypes::Prefab>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		static std::unique_ptr<SliceEngineTypes::Prefab> Load(ResourceManager& resourceMgr, const std::string& path);
		static void Destroy(SliceEngineTypes::Prefab& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::Prefab* resource, ResourceManager& mgr, const std::string& path);
	};

	template <>
	struct Type<SliceEngineTypes::Skeleton>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		static std::unique_ptr<SliceEngineTypes::Skeleton> Load(ResourceManager& resourceMgr, const std::string& path);
		static void Destroy(SliceEngineTypes::Skeleton& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::Skeleton* resource, ResourceManager& mgr, const std::string& path);
	};

	template <>
	struct Type<SliceEngineTypes::AnimationPackage>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		static std::unique_ptr<SliceEngineTypes::AnimationPackage> Load(ResourceManager& resourceMgr, const std::string& path);
		static void Destroy(SliceEngineTypes::AnimationPackage& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::AnimationPackage* resource, ResourceManager& mgr, const std::string& path);
	};

	template <>
	struct Type<SliceEngineTypes::StateMachine>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		static std::unique_ptr<SliceEngineTypes::StateMachine> Load(ResourceManager& resourceMgr, const std::string& path);
		static void Destroy(SliceEngineTypes::StateMachine& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::StateMachine* resource, ResourceManager& mgr, const std::string& path);
	};

	template <>
	struct Type<SliceEngineTypes::Font_Data>
	{
		constexpr static inline uint64_t defaultResourceGUID = DefaultResourceIDs::FONT_BLANK_DEFAULT;

		static std::unique_ptr<SliceEngineTypes::Font_Data> Load(ResourceManager& resourceMgr, const std::string& path);
		static void Destroy(SliceEngineTypes::Font_Data& resource, ResourceManager& resourceMgr);

		static void Reload(SliceEngineTypes::Font_Data* resource, ResourceManager& mgr, const std::string& path);
	};
}


#endif