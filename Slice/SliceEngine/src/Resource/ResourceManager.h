#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <rttr/type.h>
#include "GUID.h"

namespace SliceEngine
{

	class ResourceManager;

	template <typename T> class Handle;

	template <typename ResourceType>
	struct Type
	{
		// Each specialization MUST provide:
		// constexpr static inline uint64_t typeUUID = YOUR_UNIQUE_ID;
		// constexpr static inline uint64_t defaultResourceGUID = YOUR_DEFAULT_ID;
		// static ResourceType* Load(ResourceManager&, uint64_t guid, const std::string& path);
		// static void Destroy(ResourceType&, ResourceManager&);
	};

	namespace detail
	{
		struct Instance
		{
			void* data = nullptr;
			int refCount = 1;
			void (*destroyer)(void*, ResourceManager&) = nullptr;
		};
	}

	class ResourceManager
	{
	public:
		ResourceManager() = default;
		~ResourceManager();
	private:
	};
}




#endif