#ifndef RESOURCE_H
#define RESOURCE_H
//#include "ResourceManager.h"
#include <cstdint>
#include <string_view>

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		class Texture;
		class Model;
		class Shader;
		//class Sound;
		class Scene;
	}

	namespace FNVHash
	{
		//temporarily mvoed to resourcemanager.h


		/*constexpr uint64_t Prime = 1099511628211ULL;
		constexpr uint64_t OffsetBasis = 14695981039346656037ULL;

		constexpr uint64_t fnv1a(const std::string_view str)
		{
			uint64_t hash = OffsetBasis;
			for (char c : str)
			{
				hash ^= static_cast<uint64_t>(c);
				hash *= Prime;
			}

			return hash;
		}*/
	}

	// type UUIDs
	// shift to asset manager in editor
	//namespace ResourceTypeIDs
	//{
	//	constexpr uint64_t TEXTURE = FNVHash::fnv1a("Texture");
	//	constexpr uint64_t SHADER = FNVHash::fnv1a("Shader");
	//	constexpr uint64_t MODEL = FNVHash::fnv1a("Model");
	//	constexpr uint64_t SOUND = FNVHash::fnv1a("Sound");
	//	constexpr uint64_t SCENE = FNVHash::fnv1a("Scene");
	//}

	template <>
	struct Type<SliceEngineTypes::Texture>
	{
		//constexpr static inline uint64_t typeUUID = ResourceTypeIDs::TEXTURE;

		// for when we implement meta data files
		//static SliceEngineTypes::Texture* Load(ResourceManager& resourceMgr, uint64_t resourceID);
		// for now load with file name directly
		static SliceEngineTypes::Texture* Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Texture& resource, ResourceManager& resourceMgr);

	};

	template <>
	struct Type<SliceEngineTypes::Shader>
	{
		//constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SHADER;
		constexpr static inline uint64_t defaultResourceGUID = 13303718109627574413;

		// for now load with file name directly
		static SliceEngineTypes::Shader* Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Shader& resource, ResourceManager& resourceMgr);

	};

	template <>
	struct Type<SliceEngineTypes::Model>
	{
		//constexpr static inline uint64_t typeUUID = ResourceTypeIDs::MODEL;
		constexpr static inline uint64_t defaultResourceGUID = 13303718109627574413;

		// for now load with file name directly
		static SliceEngineTypes::Model* Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Model& resource, ResourceManager& resourceMgr);

	};

	//template <>
	//struct Type<SliceEngineTypes::Sound>
	//{

	//};

	template <>
	struct Type<SliceEngineTypes::Scene>
	{
		constexpr static inline uint64_t defaultResourceGUID = 13303718109627574413;
		//constexpr static inline uint64_t typeUUID = ResourceTypeIDs::SCENE;
		// for now load with file name directly
		static SliceEngineTypes::Scene* Load(ResourceManager& resourceMgr, const std::string& path);
		static void Destroy(SliceEngineTypes::Scene& resource, ResourceManager& resourceMgr);
	};
}


#endif