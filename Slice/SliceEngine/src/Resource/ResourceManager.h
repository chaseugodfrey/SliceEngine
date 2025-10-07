/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ResourceManager.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles all resources

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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
		 static std::unique_ptr<ResourceType> Load(ResourceManager&, /*uint64_t guid,*/ const std::string& path);
		// static void Destroy(ResourceType&, ResourceManager&);
	};

	namespace detail
	{
		struct Instance
		{
			std::unique_ptr<void, void(*)(void*)> data = { nullptr, nullptr };
			int refCount = 1;
			std::string filePath;
			std::function<std::unique_ptr<void, void(*)(void*)>(ResourceManager&, const std::string&)> reload;
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
			//for (auto& i : mInstances) {
			//	i.second.destroyer(i.second.data, *this);
			//	delete i.second.data;	//not sure but 50% sure this is supposed to be here
			//}
		}

		inline void ReleaseResource(const GUID& guid)
		{
			auto it = mInstances.find(guid);
			if (it != mInstances.end())
			{
				it->second.refCount--;
				if (it->second.refCount <= 0)
				{
					mInstances.erase(it);
				}
			}
		}

		void InitResourceManager();

		template<typename T>
		Handle<T> get(const GUID& guid)
		{
			GUID assetGUID = guid;
			if (assetGUID == (GUID)0)
			{
				assetGUID = (GUID)Type<T>::defaultResourceGUID;
				//SLICE_LOG_WARNING("Attempted to get resource with null GUID.");
				//return Handle<T>();
			}

			auto it = mInstances.find(assetGUID);
			if (it != mInstances.end())
			{
				//it->second.refCount++;
				return Handle<T>(*this, &it->second, assetGUID);
			}

			// Asset not loaded, so load the asset
			std::string path = "";
			if (mGUIDToResource.count(assetGUID))
			{
				path = mGUIDToResource.at(assetGUID);
			}
			else if (mGUIDToResource.count((GUID)Type<T>::defaultResourceGUID))
			{
				path = mGUIDToResource.at((GUID)Type<T>::defaultResourceGUID);
				SLICE_LOG_WARNING("Resource with GUID {} not found. Using default resource." + std::to_string(assetGUID.GetGUID()));
			}
			else
			{
				SLICE_LOG_ERROR("Resource with GUID {} not found and no default resource available." + std::to_string(assetGUID.GetGUID()));
				return Handle<T>();
			}

			std::unique_ptr<T> data = Type<T>::Load(*this,/* guid.GetGUID(),*/ path);
			if (!data)
			{
				SLICE_LOG_ERROR("Unable to load resource");
				return Handle<T>();
			}

			auto& instance = mInstances[assetGUID];
			instance.filePath = path;
			auto deleter = [](void* ptr) 
			{ 
				delete static_cast<T*>(ptr); 
			};
			instance.data = std::unique_ptr<void, void(*)(void*)>(data.release(), deleter);

			instance.reload = [](ResourceManager& mgr, const std::string& path) {
				std::unique_ptr<T> newData = Type<T>::Load(mgr, path);
				auto deleter = [](void* ptr) { delete static_cast<T*>(ptr); };
				return std::unique_ptr<void, void(*)(void*)>(newData.release(), deleter);
			};
			

			return Handle<T>(*this, &instance, assetGUID);
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
		Handle() : mManager(nullptr), mInstance(nullptr) {}
		
		Handle(ResourceManager& resourceMgr, detail::Instance* ptr, GUID guid) : mManager(&resourceMgr), mInstance(ptr), mGUID(guid) 
		{
			AddRef();
		}
		
		~Handle() 
		{ 
			//Release(); //not sure but im like 90% sure this is not supposed to be here
		}

		Handle(const Handle& other) : mManager(&other.mManager), mInstance(other.mInstance), mGUID(other.mGUID)
		{
			AddRef();
		}

		Handle& operator = (const Handle& other)
		{
			if (this != &other)
			{
				Release();
				mManager = other.mManager;
				mInstance = other.mInstance;
				mGUID = other.mGUID;
				AddRef();
			}

			return *this;
		}

		Handle(Handle&& other) noexcept : mManager(other.mManager), mInstance(other.mInstance), mGUID(other.mGUID)
		{
			other.mManager = nullptr;
			other.mInstance = nullptr;
		}

		Handle& operator=(Handle&& other) noexcept
		{
			if (this != &other)
			{
				Release();
				mManager = other.mManager;
				mInstance = other.mInstance;
				mGUID = other.mGUID;
				other.mManager = nullptr;
				other.mInstance = nullptr;
			}

			return *this;
		}

		T* operator->()
		{
			return get();
		}

		const T* operator->() const
		{
			return get();
		}

		T* get()
		{
			return mInstance ? static_cast<T*>(mInstance->data.get()) : nullptr;
		}

		const T* get() const
		{
			return mInstance ? static_cast<T*>(mInstance->data.get()) : nullptr;
		}

		bool IsValid() const
		{
			return mInstance != nullptr && mInstance->data != nullptr;
		}

	private:
		void AddRef()
		{
			if (mInstance)
			{
				mInstance->refCount++;
			}
		}

		void Release()
		{
			if (mManager && mInstance)
			{
				mManager->ReleaseResource(mGUID);
				//auto& instances = mManager->mInstances;
				//auto it = instances.find(mGUID);
				//if (it != instances.end())
				//{
				//	it->second.refCount--;
				//	if (it->second.refCount <= 0)
				//	{
				//		it->second.destroyer(it->second.data, *mManager);
				//		instances.erase(it);
				//	}
				//}
			}
			mManager = nullptr;
			mInstance = nullptr;
		}

		// 24 bytes 
		ResourceManager* mManager;
		detail::Instance* mInstance;
		GUID mGUID;
	};

}


#include "Resource.h"


#endif