#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
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

		//template<typename T>
		//Handle<T> get(const std::string& path)
		//{
		//	uint64_t typeID = Type<T>::typeUUID;

		//}

		template<typename T>
		Handle<T> get(const GUID& guid)
		{
			auto it = mInstances.find(guid);
			if (it != mInstances.end())
			{
				it->second.refCount++;
				return Handle<T>(*this, static_cast<T*>(it->second.data), guid);
			}

			// Asset not loaded, so load the asset
			std::string path = "";
			if (mGUIDToPath.count(guid))
			{
				path = mGUIDToPath.at(guid);
			}

			T* data = Type<T>::Load(*this, guid.GetGUID(), path);
			if (!data)
			{
				SLICE_LOG_ERROR("Unable to load resource");
				return Handle<T>();
			}

			mInstances[guid] =
			{
				data,
				1,
				[](void* d, ResourceManager& resourceMgr)
				{
					Type<T>::Destroy(*static_cast<T*>(d), resourceMgr);
				}
			};

			return Handle<T>(*this, data, guid);
		}

	private:
		template<typename T> friend class Handle;


		template<typename T>
		void RegisterAsset(const std::string& path)
		{
			GUID guid = GUID::Generate();
			mGUIDToPath[guid] = path;
		}

		std::unordered_map<GUID, detail::Instance> mInstances;
		std::unordered_map<GUID, std::string> mGUIDToPath;

	};

	// handle for assets
	template <typename T>
	class Handle
	{
	public:
		// TODO: Move all into cpp file
		Handle() : mManager(nullptr), mPtr(nullptr) {}
		
		Handle(ResourceManager& resourceMgr, T* ptr, GUID guid) : mManager(resourceMgr), mPtr(ptr), mGUID(guid) {}
		
		~Handle() 
		{ 
			Release(); 
		}

		Handle(const Handle& other) : mManager(other.mManager), mPtr(other.mPtr), mGUID(other.mGUID)
		{
			AddRef();
		}

		Handle& operator = (const Handle& other)
		{
			if (this != &other)
			{
				Release();
				mManager = other.mManager;
				mPtr = other.mPtr;
				mGUID = other.mGUID;
				AddRef();
			}

			return *this;
		}

		Handle(Handle&& other) noexcept : mManager(other.mManager), mPtr(other.mPtr), mGUID(other.mGUID)
		{
			other.mManager = nullptr;
			other.mPtr = nullptr;
		}

		Handle& operator=(Handle&& other) noexcept
		{
			if (this != &other)
			{
				Release();
				mManager = other.mManager;
				mPtr = other.mPtr;
				mGUID = other.mGUID;
				other.mManager = nullptr;
				other.mPtr = nullptr;
			}

			return *this;
		}

		T* operator->()
		{
			return mPtr;
		}

		const T* operator->() const
		{
			return mPtr;
		}

		T* get()
		{
			return mPtr;
		}

		const T* get() const
		{
			return mPtr;
		}

		bool IsValid() const
		{
			return mPtr != nullptr;
		}

	private:
		void AddRef()
		{
			if (mManager && mPtr)
			{
				mManager->mInstances[mGUID].refCount++;
			}
		}

		void Release()
		{
			if (mManager && mPtr)
			{
				auto& instances = mManager->mInstances;
				auto it = instances.find(mGUID);
				if (it != instances.end())
				{
					it->second.refCount--;
					if (it->second.refCount <= 0)
					{
						it->second.destroyer(it->second.data, *mManager);
						instances.erase(it);
					}
				}
			}
		}

		// 24 bytes 
		ResourceManager* mManager;
		T* mPtr;
		GUID mGUID;
	};
}




#endif