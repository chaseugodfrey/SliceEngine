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

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		class Texture;
		class Model;
		struct ModelNode;	//used to represent scene graph
		class Shader;
		class Audio;
		class Scene;
		class Prefab;
	}

	namespace DefaultResourceIDs
	{
		constexpr uint64_t CUBE_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultCube");
		constexpr uint64_t LINE_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultLine");
		constexpr uint64_t QUAD_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultQuad");
		constexpr uint64_t FRUSTRUM_DEFAULT = SliceEngine::FNVHash::fnv1a("DefaultFrustrum");
	}


	template <>
	struct Type<SliceEngineTypes::Texture>
	{
		// for when we implement meta data files
		//static SliceEngineTypes::Texture* Load(ResourceManager& resourceMgr, uint64_t resourceID);
		// for now load with file name directly
		constexpr static inline uint64_t defaultResourceGUID = 18349208178533231704;

		static std::unique_ptr<SliceEngineTypes::Texture> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Texture& resource, ResourceManager& resourceMgr);

	};

	template <>
	struct Type<SliceEngineTypes::Shader>
	{
		constexpr static inline uint64_t defaultResourceGUID = 13303718109627574413;

		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::Shader> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Shader& resource, ResourceManager& resourceMgr);

	};

	template <>
	struct Type<SliceEngineTypes::Model>
	{
		//constexpr static inline uint64_t typeUUID = ResourceTypeIDs::MODEL;
		constexpr static inline uint64_t defaultResourceGUID = DefaultResourceIDs::CUBE_DEFAULT;

		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::Model> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Model& resource, ResourceManager& resourceMgr);

	};

	template <>
	struct Type<SliceEngineTypes::Audio>
	{
		constexpr static inline uint64_t defaultResourceGUID = 13303718109627574413;

		static std::unique_ptr<SliceEngineTypes::Audio> Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Audio& resource, ResourceManager& resourceMgr);
	};

	template <>
	struct Type<SliceEngineTypes::Scene>
	{
		constexpr static inline uint64_t defaultResourceGUID = 13303718109627574413;
		//constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SCENE;
		// for now load with file name directly
		static std::unique_ptr<SliceEngineTypes::Scene> Load(ResourceManager& resourceMgr, const std::string& path);
		static void Destroy(SliceEngineTypes::Scene& resource, ResourceManager& resourceMgr);
	};

	template <>
	struct Type<SliceEngineTypes::Prefab>
	{
		constexpr static inline uint64_t defaultResourceGUID = 0;

		static std::unique_ptr<SliceEngineTypes::Prefab> Load(ResourceManager& resourceMgr, const std::string& path);
		static void Destroy(SliceEngineTypes::Prefab& resource, ResourceManager& resourceMgr);
	};
}


#endif