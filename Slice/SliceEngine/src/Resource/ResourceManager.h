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
		 static ResourceType* Load(ResourceManager&, /*uint64_t guid,*/ const std::string& path);
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

	//used for the hack (see below in resource manager)
	namespace FNVHash
	{
		//temporarily moved from resource.h

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
		};
	}


	class ResourceManager
	{
	public:
		ResourceManager() = default;

		/*
		* Hack number 2 i dont actually know why this is like this
		*/
		~ResourceManager() {
			for (auto& i : mInstances) {
				i.second.destroyer(i.second.data, *this);
				delete i.second.data;	//not sure but 50% sure this is supposed to be here
			}
		}

		void InitResourceManager();

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
			if (mGUIDToResource.count(guid))
			{
				path = mGUIDToResource.at(guid);
			}
			else if (mGUIDToResource.count((GUID)Type<T>::defaultResourceGUID))
			{
				path = mGUIDToResource.at((GUID)Type<T>::defaultResourceGUID);
				SLICE_LOG_WARNING("Resource with GUID {} not found. Using default resource.", guid.GetGUID());
			}
			else
			{
				SLICE_LOG_ERROR("Resource with GUID {} not found and no default resource available.", guid.GetGUID());
				return Handle<T>();
			}

			T* data = Type<T>::Load(*this,/* guid.GetGUID(),*/ path);
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

		void RegisterResourceAsset(const GUID& guid, const std::string& path)
		{
			mGUIDToResource[guid] = path;
		}
		
		std::unordered_map<std::string, GUID> mFileNameToGUID;

	private:
		template<typename T> friend class Handle;


		template<typename T>
		void RegisterAsset(const std::string& path)
		{
			GUID guid = GUID::Generate();
			mGUIDToPath[guid] = path;
		}

	private:
		template<typename T> friend class Handle;



		std::unordered_map<GUID, detail::Instance> mInstances;
		std::unordered_map<GUID, std::string> mGUIDToPath;
		std::unordered_map<GUID, std::string> mGUIDToResource;

		// TODO: Change this to be configurable
		std::filesystem::path mResourcesDirectory = std::filesystem::path("Resources");
	};

	// handle for assets
	template <typename T>
	class Handle
	{
	public:
		// TODO: Move all into cpp file
		Handle() : mManager(nullptr), mPtr(nullptr) {}
		
		Handle(ResourceManager& resourceMgr, T* ptr, GUID guid) : mManager(&resourceMgr), mPtr(ptr), mGUID(guid) {}
		
		~Handle() 
		{ 
			//Release(); //not sure but im like 90% sure this is not supposed to be here
		}

		Handle(const Handle& other) : mManager(&other.mManager), mPtr(other.mPtr), mGUID(other.mGUID)
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


#include "Resource.h"


#endif