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
		class Sound;
	}

	namespace FNVHash
	{
		constexpr uint64_t Prime = 1099511628211ULL;
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
		}
	}

	// type UUIDs 
	namespace ResourceTypeIDs
	{
		constexpr uint64_t TEXTURE = FNVHash::fnv1a("Texture");
		constexpr uint64_t SOUND = FNVHash::fnv1a("Sound");
	}

	template <>
	struct Type<SliceEngineTypes::Texture>
	{
		constexpr static inline uint64_t typeUUID = ResourceTypeIDs::TEXTURE;

		// for when we implement meta data files
		static SliceEngineTypes::Texture* Load(ResourceManager& resourceMgr, uint64_t resourceID);
		// for now load with file name directly
		static SliceEngineTypes::Texture* Load(ResourceManager& resourceMgr, const std::string& path);

		static void Destroy(SliceEngineTypes::Texture& resource, ResourceManager& resourceMgr);

	};

	template <>
	struct Type<SliceEngineTypes::Sound>
	{

	};
}


#endif