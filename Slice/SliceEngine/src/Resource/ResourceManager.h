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
			std::unique_ptr<void, std::function<void(void*)>> data = {nullptr, nullptr};
			int refCount = 1;
			std::string filePath;
			std::function<std::unique_ptr<void, std::function<void(void*)>>(ResourceManager&, const std::string&)> reload;

			std::function<void(void* data, ResourceManager&, const std::string&)> reload_in_place;
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
			////	i.second.destroyer(i.second.data, *this);
			////	delete i.second.data;	//not sure but 50% sure this is supposed to be here
			//}
		}

		void ReleaseResource(const GUID& guid);

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


			// Asset not loaded, so load the asset
			std::string path = "";
			if (mGUIDToResource.count(assetGUID))
			{
				path = mGUIDToResource.at(assetGUID);
			}
			else if (mGUIDToResource.count((GUID)Type<T>::defaultResourceGUID))
			{
				path = mGUIDToResource.at((GUID)Type<T>::defaultResourceGUID);

				assetGUID = (GUID)Type<T>::defaultResourceGUID;

				SLICE_LOG_WARNING("Resource with GUID {} not found. Using default resource." + std::to_string(assetGUID.GetGUID()));
			}
			else
			{
				//SLICE_LOG_ERROR("Resource with GUID {} not found and no default resource available." + std::to_string(assetGUID.GetGUID()));
				return Handle<T>();
			}
			
			auto it = mInstances.find(assetGUID);
			if (it != mInstances.end())
			{
				//it->second.refCount++;
				return Handle<T>(*this, &it->second, assetGUID);
			}

			std::unique_ptr<T> data = Type<T>::Load(*this,/* guid.GetGUID(),*/ path);
			if (!data)
			{
				SLICE_LOG_ERROR("Unable to load resource");
				return Handle<T>();
			}

			auto& instance = mInstances[assetGUID];
			instance.filePath = path;
			std::function<void(void*)> deleter = [this](void* ptr) mutable
			{ 
					T* resourcePtr = static_cast<T*>(ptr);
					Type<T>::Destroy(*resourcePtr, *this);

					delete resourcePtr;
			};
			instance.data = std::unique_ptr<void, std::function<void(void*)>>(data.release(), deleter);

			instance.reload = [this](ResourceManager& mgr, const std::string& path) -> std::unique_ptr<void, std::function<void(void*)>>
			{
					std::unique_ptr<T> newData = Type<T>::Load(mgr, path);
					if (!newData)
					{
						SLICE_LOG_ERROR("Unable to load resource");
						return std::unique_ptr<void, std::function<void(void*)>>(nullptr, nullptr);
					}

					std::function<void(void*)> newDeleter = [this](void* ptr) mutable
						{
							T* resourcePtr = static_cast<T*>(ptr);
							Type<T>::Destroy(*resourcePtr, *this);

							delete resourcePtr;
						};

					return std::unique_ptr<void, std::function<void(void*)>>(newData.release(), newDeleter);
			};

			instance.reload_in_place = [this](void* data, ResourceManager& mgr, const std::string& path)
				{
					if (!data)
					{
						SLICE_LOG_ERROR("Cannot reload resource: data is null");
						return;
					}

					// Cast the void pointer back to the correct resource type
					T* resourceToReload = static_cast<T*>(data);

					// Call the specific Reload function for this resource type
					Type<T>::Reload(resourceToReload, mgr, path);
				};
			

			return Handle<T>(*this, &instance, assetGUID);
		}

		void ReloadResource(const GUID& guid);

		void ReloadResourceInPlace(const GUID& guid);

		template<typename T>
		Handle<T> get(const std::string& fileName)
		{
			auto it = mFileNameToGUID.find(fileName);

			if (it == mFileNameToGUID.end())
			{
				SLICE_LOG_ERROR("ResourceManager: file name not registered");
				return Handle<T>();
			}

			const GUID& guid = it->second;

			return get<T>(guid);
		}

		void RegisterResourceAsset(const GUID& guid, const std::string& path);

		void RegisterResourceAsset(const std::string& path);

		//void UpdateEntityResources();	//this func is litearlly not called, also removed any usage of handle.filename - elton

		void Shutdown();

		GUID GetSkeletonGUIDFromModel(GUID modelGUID);

		GUID GetAnimationGUIDFromModel(GUID modelGUID);

		bool CheckResource(GUID guid);

		std::optional<std::filesystem::path> GetResourcePath(std::string filename);
		
		std::unordered_map<std::string, GUID> mFileNameToGUID;

		// Option 1:
		// I'm thinking when deserializing a scene, pull out all the GUIDs used in the scene
		// and store it in here to create all the necessary handles?
		std::set<GUID> mSceneGUIDs; 

		// Option 2: 
		// When serializing the scene, i'll serialize all the scene resources separately
		// then deserialize the scene resources before the scene itself
		std::set<GUID> mGUIDToSerialize;
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
			////std::cout << "Resource being released " << mGUID.GetGUID() << " : ";
			//if (!mManager)
			//	return;

			//for (const auto& [key, val] : mManager->mFileNameToGUID)
			//{
			//	if (val == mGUID)
			//	{
			//		//std::cout << key << std::endl;
			//	}
			//}
			Release(); //not sure but im like 90% sure this is not supposed to be here
		}

		Handle(const Handle& other) : mManager(other.mManager), mInstance(other.mInstance), mGUID(other.mGUID)
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

		//bool operator<(const Handle<T>& other) const
		//{
		//	return mGUID < other.mGUID;
		//}

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
			//return mInstance ? static_cast<T*>(mInstance->data.get()) : nullptr;

			if (mInstance)
			{
				return static_cast<T*>(mInstance->data.get());
			}
			else
				return nullptr;
		}

		const T* get() const
		{
			return mInstance ? static_cast<T*>(mInstance->data.get()) : nullptr;
		}

		bool IsValid() const
		{
			return mInstance != nullptr && mInstance->data != nullptr;
		}

		GUID getGUID() const
		{
			return mGUID;
		}

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
		//std::string fileName;
	};

}
#endif