/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ScriptObject.h

 author:   Gideon Francis

 email:       g.francis@digipen.edu

 brief:      Script object. Holds the equivalent c# script, references to the standard OnCreate, OnUpdate and OnConstruct functions.
				Used as a shared_ptr in scriptSystem.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


#ifndef SCRIPT_OBJECT_H
#define SCRIPT_OBJECT_H
#include "../ECS/ECSTypes.h"
#include "../ECS/GameObject.h"
#include <mono/metadata/threads.h>
#include "../Core/Core.h"
#include "ScriptSystem.h"
#include "Core/Core.h"
namespace SliceEngine
{
	//class ScriptSystem; // Forward-declare the class
	//extern ScriptSystem* gScriptSystem; // Re-declare the extern global

	// Types that a script field can be
	enum class ScriptFieldType : int
	{
		None = 0,
		Float,
		Double,
		Bool,
		Char,
		Int,
		Short,
		UInt,
		Vector2,
		Vector3,
		GameObject,
		String,
		Audio,
		Prefab,
		Array,
		List
	};



	//struct
	struct ScriptField
	{
		// same as before, keep track o the actual type in teh field
		ScriptFieldType mType{ ScriptFieldType::None };

		// Keep track if it is a list or array
		ScriptFieldType mContainerType{ ScriptFieldType::None };

		std::string mName{};
		MonoClassField* mClassField{ nullptr };

		// used purely for prefab serialization
		// if not we'd normally just use name to retrieve
		// the variable and set the variable in runtime
		rttr::variant value;

		// Keep track if its an array. Will be null if not an array
		MonoClass* mElementClass{ nullptr };

		// For List<T> Only
		MonoClass* mCollectionClass;
		MonoMethod* mListGetCount{ nullptr };
		MonoMethod* mListGetItem{ nullptr };
		MonoMethod* mListSetItem{ nullptr };
		MonoMethod* mListAdd{ nullptr };
		MonoMethod* mListClear{ nullptr };
		MonoMethod* mListCtor{ nullptr };
		MonoMethod* mListRemoveAt{ nullptr };

		//ScriptField() : mType(ScriptFieldType::None), mClassField(nullptr) {}
	};

	class ScriptClass
	{
	public:
		friend class ScriptSystem;

		std::string mNameSpace{};
		std::string mClassName{};

		// Store what fields the script has
		std::map<std::string, ScriptField> mFields;

		ScriptClass() = default;
		ScriptClass(const std::string& nameSpace, const std::string& className);

		MonoClass* mMonoClass = nullptr;

		/// <summary>
		/// Instantiates the script object. 
		/// </summary>
		MonoObject* Instantiate();
		/// <summary>
		/// Used to get the method/function out of the C# side script. For example, OnUpdate is the name. Param count is 1 cause float dt
		/// </summary>
		/// <param name="name">Name of the function</param>
		/// <param name="parameterCount">Number of parameters it needs</param>
		/// <returns></returns>
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);


	};

	class ScriptObject
	{
	private:
		std::shared_ptr<ScriptClass> mScriptClass;

		MonoObject* mMonoInstance = nullptr;

		// Core functions
		MonoMethod* mConstruct = nullptr;
		MonoMethod* mOnAwake = nullptr;
		MonoMethod* mOnCreate = nullptr;
		MonoMethod* mOnUpdate = nullptr;
		MonoMethod* mOnFixedUpdate = nullptr;
		MonoMethod* mOnLateUpdate = nullptr;
		MonoMethod* mOnEntityDestroy = nullptr;
		MonoMethod* mOnEntityEnabled = nullptr;
		MonoMethod* mOnEntityDisabled = nullptr;

		// Standard object functions
		MonoMethod* mOnCollideEnter = nullptr;
		MonoMethod* mOnCollideStay = nullptr;
		MonoMethod* mOnCollideExit = nullptr;
		MonoMethod* mOnTriggerEnter = nullptr;
		MonoMethod* mOnTriggerStay = nullptr;
		MonoMethod* mOnTriggerExit = nullptr;


		// Mouse functions
		MonoMethod* mOnMouseEnter = nullptr;	//idk who wrote these 3 funcs but ok
		MonoMethod* mOnMouseExit = nullptr;
		MonoMethod* mOnMouseHover = nullptr;


		// UI Functions
		MonoMethod* mOnClick = nullptr;			//this too
		MonoMethod* mOnButtonClick{};
		MonoMethod* mOnButtonHover{};
		MonoMethod* mOnButtonExitHover{};
		MonoMethod* mOnButtonRelease{};
		MonoMethod* mOnSliderValue{};
		MonoMethod* mOnSAnimStop{};
		MonoMethod* mOnSAnimLoop{};

		// FSM Functions
		MonoMethod* mOnStateEnter = nullptr;
		MonoMethod* mOnStateUpdate = nullptr;
		MonoMethod* mOnStateExit = nullptr;


	public:
		uint32_t mHandle;

		friend class ScriptSystem;

		/// <summary>
		/// Default constructor
		/// </summary>
		ScriptObject() = default;

		/// <summary>
		/// Overloaded constructor
		/// </summary>
		/// <param name="nameSpace">Name space of the C# script. Usually always Carmicah</param>
		/// <param name="className">name of the class</param>
		ScriptObject(std::shared_ptr<ScriptClass> scClass, Entity entity);

		~ScriptObject();

		void Destroy();

		/// <summary>
		/// Get the mMonoInstance reference to get script instance for C# side
		/// </summary>
		MonoObject* GetInstance();


		/// <summary>
		/// Set up all the internal function calls by getting the method and the number of params
		/// </summary>
		/// <param name="entity"></param>
		//void SetUpEntity(Entity entity);


		/// <summary>
		/// For calling the OnConstruct function 
		/// </summary>
		/// <param name="id"></param>
		void InvokeOnConstruct(unsigned int id);

		void InvokeOnAwake();

		/// <summary>
		/// Calls the OnCreate function. This is only called for all entity scripts, when the game is ran in engine
		/// </summary>
		void InvokeOnCreate();

		/// <summary>
		/// Calls the OnUpdate function. This is done every loop when the game is in runtime for every entity
		/// </summary>
		/// <param name="dt">Delta time</param>
		void InvokeOnUpdate(float dt);

		/// <summary>
		/// Calls the OnUpdate function. This is done every loop when the game is in runtime for every entity
		/// </summary>
		/// <param name="dt">Delta time</param>
		void InvokeOnFixedUpdate(float dt);

		/// <summary>
		/// Function for delaying update
		/// </summary>
		/// <param name="dt">Delta time</param>
		void InvokeOnLateUpdate(float dt);

		/// <summary>
		/// Calls the OnEntityDestroy function. This is done every loop when the game is in runtime for every entity destroyed
		/// </summary>
		void InvokeOnEntityDestroy(unsigned int id);

		/******************** UI Invokes *****************/
		/// <summary>
		/// Call when obj is clicked, if it has a script with an onClick function then itll run it
		/// </summary>
		void InvokeOnClick();

		void InvokeButtonOnClick();
		void InvokeButtonOnHover();
		void InvokeButtonOnExitHover();
		void InvokeButtonOnRelease();
		void InvokeOnSliderValue(float);

		void InvokeSAnimStop();
		void InvokeSAnimLoop();

		/// <summary>
		/// Call when obj collides, if it has a script with an onCollide function
		/// </summary>
		void InvokeOnCollideEnter(unsigned int id);

		void InvokeOnCollideStay(unsigned int id);

		void InvokeOnCollideExit(unsigned int id);

		/// <summary>
		/// Call when kinematic vs kinematic trigger
		/// </summary>
		/// <param name="id"></param>
		void InvokeOnTriggerEnter(unsigned int id);

		void InvokeOnTriggerStay(unsigned int id);

		void InvokeOnTriggerExit(unsigned int id);

		/// <summary>
		/// Call when a mouse enters the collider box of an object
		/// </summary>
		void InvokeOnMouseEnter();

		/// <summary>
		/// Call when a mouse leaves the collider box of an object
		/// </summary>
		void InvokeOnMouseExit();

		/// <summary>
		/// Call when a mouse is hovering the collider box of an object
		/// </summary>
		void InvokeOnMouseHover();

		/// <summary>
		/// Call when the entity is enabled again
		/// </summary>
		void InvokeOnEnabled();

		/// <summary>
		/// Call when the entity is disabled
		/// </summary>
		void InvokeOnDisabled();

		/// <summary>
		/// Call when entering a state
		/// </summary>
		/// <param name="stateName"></param>
		void InvokeOnStateEnter(std::string stateName);

		/// <summary>
		/// Call when updating a state
		/// </summary>
		/// <param name="stateName"></param>
		/// <param name="dt"></param>
		void InvokeOnStateUpdate(std::string stateName, float dt);

		/// <summary>
		/// Call when exiting a state
		/// </summary>
		/// <param name="stateName"></param>
		void InvokeOnStateExit(std::string stateName);

		// option 1, use a map then set as well
		std::map<std::string, rttr::variant> GetAllFields();

		// option 2, lambda this b****
		void ExposeForEditor(const std::function<void(const std::string&, rttr::variant&)>& editorCall);

		rttr::variant GetMonoFieldValue(MonoObject* scriptInstance, MonoClassField* field);

		void SetMonoFieldValue(MonoObject* scriptInstance, MonoClassField* field, rttr::variant& value);

		rttr::variant GetFieldValue(const std::string& name);

		void SetFieldValue(const std::string& name, rttr::variant val);

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0)
			{
				return T();
			}
			auto iter = fields.find(name);
			const ScriptField& field = iter->second;
			char fieldBuffer[32];
			mono_field_get_value(mMonoInstance, field.mClassField, fieldBuffer);
			//mono_string_to_utf8
			// cast it to the T that is trying to be retrieved and derefence it
			return *(T*)fieldBuffer;
		}

		template<>
		std::string GetFieldValue(const std::string& name)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0)
			{
				return std::string();
			}
			auto iter = fields.find(name);
			const ScriptField& field = iter->second;

			std::string result;
			MonoString* monoStr = reinterpret_cast<MonoString*>(mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance));
			if (monoStr != nullptr)
			{
				char* utf8str = mono_string_to_utf8(monoStr);
				result = utf8str;
				mono_free(utf8str);
			}

			return result;
			//char fieldBuffer[128];
			//mono_field_get_value(mMonoInstance, field.mClassField, fieldBuffer);
			////mono_string_to_utf8
			//// cast it to the T that is trying to be retrieved and derefence it
			//return fieldBuffer;

		}

		template<>
		PrefabVar GetFieldValue<PrefabVar>(const std::string& name)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.contains(name) == 0)
			{
				return PrefabVar();
			}

			const ScriptField& field = fields.at(name);

			MonoObject* instance = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			if (instance == nullptr)
			{
				return PrefabVar();
			}

			MonoClass* prefabClass = mono_object_get_class(instance);
			MonoClassField* idField = mono_class_get_field_from_name(prefabClass, "prefabName");

			MonoString* monoStr = reinterpret_cast<MonoString*>(mono_field_get_value_object(mono_domain_get(), idField, instance));
			std::string result;

			if (monoStr != nullptr)
			{
				char* utf8str = mono_string_to_utf8(monoStr);
				result = utf8str;
				mono_free(utf8str);
				return PrefabVar{ result };
			}

			return PrefabVar();
		}

		template <>
		GameObject GetFieldValue<GameObject>(const std::string& name)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0)
			{
				return GameObject(); // Return an invalid/null GameObject
			}

			const ScriptField& field = fields.at(name);

			// 1. Retrieve the pointer to the C# object (MonoObject*)
			MonoObject* instance = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// 2. Handle null references in C#
			if (instance == nullptr)
			{
				return GameObject();
			}

			// 3. Get the 'mID' field from the C# GameObject class
			// Optimization: You should cache this MonoClassField* in your ScriptSystem
			MonoClass* gameObjectClass = mono_object_get_class(instance);
			MonoClassField* idField = mono_class_get_field_from_name(gameObjectClass, "mID");

			// 4. Extract the uint32_t value from the mID field
			uint32_t entityID = 0;
			mono_field_get_value(instance, idField, &entityID);

			// 5. Construct and return the C++ GameObject wrapper
			// Note: Since ScriptObject doesn't store the Registry, 
			// you must use your global gScriptSystem to access the current registry.
			return GameObject(*gScriptSystem->mRegistry, (Entity)(entityID));
		}

#pragma region For Arrays
		template<typename T>
		std::vector<T> GetArrayFieldValue(const std::string& name)
		{
			std::vector<T> result;

			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0)
			{
				return result;
			}

			auto iter = fields.find(name);
			const ScriptField& field = iter->second;


			MonoObject* arrayObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			if (arrayObject == nullptr)
				return result;

			MonoArray* monoArray = (MonoArray*)arrayObject;
			uintptr_t length = mono_array_length(monoArray);
			result.resize(length);

			for (uintptr_t i = 0; i < length; ++i)
			{
				result[i] = mono_array_get(monoArray, T, i);
			}

			return result;
		}

		template<>
		std::vector<GameObject> GetArrayFieldValue(const std::string& name)
		{
			std::vector<GameObject> result;

			std::vector<std::string> ids = GetArrayFieldValue<std::string>(name);

			for (const auto& id : ids)
			{
				if (id.empty())
				{
					result.emplace_back();
					continue;
				}

				//FactoryInstance.GetGOByEntity(static_cast<uint64_t>())
			}

			return result;
		}

		template <>
		std::vector<std::string> GetArrayFieldValue<std::string>(const std::string& name)
		{
			std::vector<std::string> result;

			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0)
				return result;

			auto iter = fields.find(name);
			const ScriptField& field = iter->second;

			MonoObject* arrayObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			if (arrayObject == nullptr)
				return result;

			MonoArray* monoArray = (MonoArray*)arrayObject;
			uintptr_t length = mono_array_length(monoArray);
			result.reserve(length);

			for (uintptr_t i = 0; i < length; ++i)
			{
				MonoString* monoStr = (MonoString*)mono_array_get(monoArray, MonoObject*, i);
				if (monoStr)
				{
					char* utf8 = mono_string_to_utf8(monoStr);
					result.push_back(utf8);
					mono_free(utf8);
				}
				else
				{
					result.push_back(std::string());
				}
			}

			return result;
		}

		template <typename T>
		void SetArrayFieldValue(const std::string& name, const std::vector<T>& val)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0)
			{
				return;
			}

			auto iter = fields.find(name);
			const ScriptField& field = iter->second;

			if (field.mElementClass == nullptr)
			{
				std::stringstream ss;
				ss << "No element info for this array: " << name << "\n";
				SLICE_LOG_ERROR(ss.str().c_str());
				return;
			}

			MonoArray* monoArray = mono_array_new(mono_domain_get(), field.mElementClass, val.size());

			for (size_t i = 0; i < val.size(); ++i)
			{
				mono_array_set(monoArray, T, i, val[i]);
			}

			mono_field_set_value(mMonoInstance, field.mClassField, monoArray);
		}

		template<>
		void SetArrayFieldValue<std::string>(const std::string& name, const std::vector<std::string>& val)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0)
			{
				return;
			}

			auto iter = fields.find(name);
			const ScriptField& field = iter->second;

			if (field.mElementClass == nullptr)
			{
				std::stringstream ss;
				ss << "No element info for this array: " << name << "\n";
				SLICE_LOG_ERROR(ss.str().c_str());
				return;
			}

			MonoArray* monoArray = mono_array_new(mono_domain_get(), field.mElementClass, val.size());

			for (size_t i = 0; i < val.size(); ++i)
			{
				MonoString* monoStr = mono_string_new(mono_domain_get(), val[i].c_str());
				mono_array_setref(monoArray, i, monoStr);
			}

			mono_field_set_value(mMonoInstance, field.mClassField, monoArray);
		}

#pragma endregion

#pragma region For Lists

		/// <summary>
		/// For generic types like float, int, bool, etc
		/// </summary>
		/// <typeparam name="T">Type of primitive</typeparam>
		/// <param name="name">Name of the variable</param>
		/// <returns>A vector containing the values of the list</returns>
		template <typename T>
		std::vector<T> GetListFieldValue(const std::string& name)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			const ScriptField& field = mScriptClass->mFields.at(name);

			std::vector<T> result;

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if any of these aren't ready then dont continue w anything
			if (listObject == nullptr || field.mListGetCount == nullptr || field.mListGetItem == nullptr)
				return result;

			MonoObject* exception = nullptr;

			MonoObject* countObj = mono_runtime_invoke(field.mListGetCount, listObject, nullptr, &exception);

			// TODO: add in exception handling like in my other invoke stuff

			int count = *(int*)mono_object_unbox(countObj);
			result.resize(count);

			void* params[1];
			for (int i = 0; i < count; ++i)
			{
				params[0] = &i;
				MonoObject* itemObj = mono_runtime_invoke(field.mListGetItem, listObject, params, &exception);
				// TODO: same as above, add exception handling maybe

				result[i] = *(T*)mono_object_unbox(itemObj);
			}

			return result;
		}

		template <>
		std::vector<std::string> GetListFieldValue(const std::string& name)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			const ScriptField& field = mScriptClass->mFields.at(name);

			std::vector<std::string> result;

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if any of these aren't ready then dont continue w anything
			if (listObject == nullptr || field.mListGetCount == nullptr || field.mListGetItem == nullptr)
				return result;

			MonoObject* exception = nullptr;

			MonoObject* countObj = mono_runtime_invoke(field.mListGetCount, listObject, nullptr, &exception);

			// TODO: add in exception handling like in my other invoke stuff

			int count = *(int*)mono_object_unbox(countObj);
			result.reserve(count);

			void* params[1];
			for (int i = 0; i < count; ++i)
			{
				params[0] = &i;
				MonoString* monoStr = (MonoString*)mono_runtime_invoke(field.mListGetItem, listObject, params, &exception);
				// TODO: same as above, add exception handling maybe

				if (monoStr)
				{
					char* utf8 = mono_string_to_utf8(monoStr);
					result.push_back(utf8);
					mono_free(utf8);
				}
				else
				{
					// if cant retrieve monoStr then use an empty str 
					result.push_back(std::string());
				}
			}

			return result;
		}

		template <>
		std::vector<GameObject> GetListFieldValue(const std::string& name)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			const ScriptField& field = mScriptClass->mFields.at(name);

			std::vector<GameObject> result;

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if any of these aren't ready then dont continue w anything
			if (listObject == nullptr || field.mListGetCount == nullptr || field.mListGetItem == nullptr)
				return result;

			MonoObject* exception = nullptr;

			MonoObject* countObj = mono_runtime_invoke(field.mListGetCount, listObject, nullptr, &exception);

			// TODO: add in exception handling like in my other invoke stuff
			if (exception || !countObj) {
				std::stringstream ss;
				ss << "C# Exception or null returned while getting count for list: " << name << "\n";
				SLICE_LOG_ERROR(ss.str().c_str());
				return result;
			}

			int count = *(int*)mono_object_unbox(countObj);
			result.reserve(count);

			//GameObject GetFieldValue bs
			MonoClass* gameObjectClass = mono_class_from_name(gScriptSystem->mCoreAssemblyImage, "SliceEngine", "GameObject");
			MonoClassField* idField = mono_class_get_field_from_name(gameObjectClass, "mID");

			void* params[1];
			for (int i = 0; i < count; ++i)
			{
				params[0] = &i;

				// This returns a MonoObject* representing the specific GameObject instance at index [i]
				MonoObject* gameObjectInstance = (MonoObject*)mono_runtime_invoke(field.mListGetItem, listObject, params, &exception);

				if (gameObjectInstance)
				{
					// Extract the uint32_t ID from this specific instance
					uint32_t entityID = 0;
					mono_field_get_value(gameObjectInstance, idField, &entityID);

					// Reconstruct the C++ GameObject wrapper and add to vector
					result.emplace_back(*gScriptSystem->mRegistry, (Entity)entityID);
				}
				else
				{
					// If the element in the C# list is null, add an invalid/empty GameObject
					result.emplace_back();
				}
			}
			return result;
		}

		template <>
		std::vector<PrefabVar> GetListFieldValue(const std::string& name)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			const ScriptField& field = mScriptClass->mFields.at(name);

			std::vector<PrefabVar> result;

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if any of these aren't ready then dont continue w anything
			if (listObject == nullptr || field.mListGetCount == nullptr || field.mListGetItem == nullptr)
				return result;

			MonoObject* exception = nullptr;

			MonoObject* countObj = mono_runtime_invoke(field.mListGetCount, listObject, nullptr, &exception);

			// TODO: add in exception handling like in my other invoke stuff
			if (exception || !countObj) {

				std::stringstream ss;
				ss << "C# Exception or null returned while getting count for list: " << name << "\n";
				SLICE_LOG_ERROR(ss.str().c_str());
				return result;
			}
			int count = *(int*)mono_object_unbox(countObj);
			result.reserve(count);

			//Prefab GetFieldValue bs
			MonoClass* prefabClass = mono_class_from_name(gScriptSystem->mCoreAssemblyImage, "SliceEngine", "Prefab");
			MonoClassField* idField = mono_class_get_field_from_name(prefabClass, "prefabName");

			void* params[1];
			for (int i = 0; i < count; ++i)
			{
				params[0] = &i;

				// This returns a MonoObject* representing the specific Prefab instance at index [i]
				MonoObject* prefabInstance = (MonoObject*)mono_runtime_invoke(field.mListGetItem, listObject, params, &exception);

				if (prefabInstance)
				{
					// Extract the uint32_t ID from this specific instance
					MonoString* monoStr = reinterpret_cast<MonoString*>(mono_field_get_value_object(mono_domain_get(), idField, prefabInstance));
					std::string strResult;

					if (monoStr != nullptr)
					{
						char* utf8str = mono_string_to_utf8(monoStr);
						strResult = utf8str;
						mono_free(utf8str);
						PrefabVar value(strResult);
						result.emplace_back(strResult);
					}
					else
					{
						// if cant get str out
						result.emplace_back();
					}
				}
				else
				{
					// If the element in the C# list is null, add an invalid/empty Prefab
					result.emplace_back();
				}
			}
			return result;
		}


		template <typename T>
		void AddListFieldValue(const std::string& name, T value)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// get the script field
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if it failed to get a list object or listAdd wasn't initialized
			if (listObject == nullptr || field.mListAdd == nullptr)
				return;

			void* params[1];
			params[0] = &value;

			MonoObject* exception = nullptr;
			mono_runtime_invoke(field.mListAdd, listObject, params, &exception);
			// TODO: handle exceptions ill do it aft everything works
		}

		template <>
		void AddListFieldValue<std::string>(const std::string& name, std::string value)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// get the script field
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if it failed to get a list object or listAdd wasn't initialized
			if (listObject == nullptr || field.mListAdd == nullptr)
				return;

			MonoString* monoStr = mono_string_new(mono_domain_get(), value.c_str());

			void* params[1];
			params[0] = monoStr;

			MonoObject* exception = nullptr;
			mono_runtime_invoke(field.mListAdd, listObject, params, &exception);
			// TODO: handle exceptions ill do it aft everything works

		}

		template <>
		void AddListFieldValue<PrefabVar>(const std::string& name, PrefabVar value)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// get the script field
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if it failed to get a list object or listAdd wasn't initialized
			if (listObject == nullptr || field.mListAdd == nullptr)
				return;
			MonoClass* prefabClass = mono_class_from_name(gScriptSystem->mCoreAssemblyImage, "SliceEngine", "Prefab");
			MonoObject* managedPrefabObj = mono_object_new(mono_domain_get(), prefabClass);

			//Initialising  the C# Prefab 
			MonoString* monoStr = mono_string_new(mono_domain_get(), value.prefabFileName.c_str());
			void* ctorArgs[1];
			ctorArgs[0] = monoStr;

			MonoMethod* ctor = mono_class_get_method_from_name(prefabClass, ".ctor", 1);
			mono_runtime_invoke(ctor, managedPrefabObj, ctorArgs, nullptr);

			void* addArgs[1];
			addArgs[0] = managedPrefabObj;

			MonoObject* exception = nullptr;
			mono_runtime_invoke(field.mListAdd, listObject, addArgs, &exception);

		}

		template <>
		void AddListFieldValue<GameObject>(const std::string& name, GameObject value)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// get the script field
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if it failed to get a list object or listAdd wasn't initialized
			if (listObject == nullptr || field.mListAdd == nullptr)
				return;

			//MonoImage* coreImage = mono_assembly_get_image(gScriptSystem->mCoreAssembly);
			//MonoClass* gameObjectClass = mono_class_from_name(coreImage, "SliceEngine", "GameObject");

			//MonoObject* managedGameObject = mono_object_new(mono_domain_get(), gameObjectClass);

			//MonoMethod* ctor = mono_class_get_method_from_name(gameObjectClass, ".ctor", 1);
			//uint32_t entityID = (uint32_t)value.GetEntity();
			//void* ctorArgs[1];
			//ctorArgs[0] = &entityID;

			//mono_runtime_invoke(ctor, managedGameObject, ctorArgs, nullptr);

			MonoObject* managedGameObject = gScriptSystem->GetOrCreateManagedObject(value.GetEntity());

			void* addArgs[1];
			addArgs[0] = managedGameObject;

			MonoObject* exception = nullptr;
			mono_runtime_invoke(field.mListAdd, listObject, addArgs, &exception);

		}

		template <typename T>
		void SetListFieldValue(const std::string& name, int index, const T& value)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// get the script field
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if it failed to get a list object or listAdd wasn't initialized
			if (listObject == nullptr || field.mListAdd == nullptr)
				return;

			void* params[2];
			params[0] = &index;
			params[1] = (void*)&value;

			MonoObject* exception = nullptr;
			mono_runtime_invoke(field.mListSetItem, listObject, params, &exception);
			// TODO: handle exceptions ill do it aft everything works
		}

		template <>
		void SetListFieldValue<std::string>(const std::string& name, int index, const std::string& value)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// get the script field
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if it failed to get a list object or listAdd wasn't initialized
			if (listObject == nullptr || field.mListAdd == nullptr)
				return;

			MonoString* monoStr = mono_string_new(mono_domain_get(), value.c_str());

			void* params[2];
			params[0] = &index;
			params[1] = monoStr;

			MonoObject* exception = nullptr;
			mono_runtime_invoke(field.mListSetItem, listObject, params, &exception);
			// TODO: handle exceptions ill do it aft everything works
		}

		template<>
		void SetListFieldValue<PrefabVar>(const std::string& name, int index, const PrefabVar& value)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// get the script field
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if it failed to get a list object or listAdd wasn't initialized
			if (listObject == nullptr || field.mListAdd == nullptr)
				return;
			MonoClass* prefabClass = mono_class_from_name(gScriptSystem->mCoreAssemblyImage, "SliceEngine", "Prefab");
			MonoObject* managedPrefabObj = mono_object_new(mono_domain_get(), prefabClass);

			//Initialising  the C# prefab
			MonoString* monoStr = mono_string_new(mono_domain_get(), value.prefabFileName.c_str());
			void* ctorArgs[1];
			ctorArgs[0] = monoStr;

			MonoMethod* ctor = mono_class_get_method_from_name(prefabClass, ".ctor", 1);
			mono_runtime_invoke(ctor, managedPrefabObj, ctorArgs, nullptr);

			//Setting the item in the list to its index
			void* params[2];
			params[0] = &index;
			params[1] = managedPrefabObj;

			MonoObject* exception = nullptr;
			mono_runtime_invoke(field.mListSetItem, listObject, params, &exception);
		}

		template<>
		void SetListFieldValue<GameObject>(const std::string& name, int index, const GameObject& value)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// get the script field
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if it failed to get a list object or listAdd wasn't initialized
			if (listObject == nullptr || field.mListAdd == nullptr)
				return;
			//MonoClass* gameObjectClass = mono_class_from_name(gScriptSystem->mCoreAssemblyImage, "SliceEngine", "GameObject");
			//MonoObject* managedGameObject = mono_object_new(mono_domain_get(), gameObjectClass);

			////Initialising  the C# GameObject
			//uint32_t entityID = (uint32_t)value.GetEntity();
			//void* ctorArgs[1];
			//ctorArgs[0] = &entityID;

			//MonoMethod* ctor = mono_class_get_method_from_name(gameObjectClass, ".ctor", 1);
			//mono_runtime_invoke(ctor, managedGameObject, ctorArgs, nullptr);

			MonoObject* managedGameObject = gScriptSystem->GetOrCreateManagedObject(value.GetEntity());

			//Setting the item in the list to its index
			void* params[2];
			params[0] = &index;
			params[1] = managedGameObject;

			MonoObject* exception = nullptr;
			mono_runtime_invoke(field.mListSetItem, listObject, params, &exception);
		}
		
		void RemoveListField(const std::string& name, int index)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}
			
			if (mScriptClass->mFields.count(name) == 0)
				return;

			const ScriptField& field = mScriptClass->mFields.at(name);
			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if any of these aren't ready then dont continue w anything
			if (listObject == nullptr || field.mListGetCount == nullptr || field.mListGetItem == nullptr)
				return;

			// parameters to pass in
			void* params[1];
			params[0] = &index;

			MonoObject* exception = nullptr;
			mono_runtime_invoke(field.mListRemoveAt, listObject, params, &exception);

			// TODO: handle exceptions like the other ones :pepepray:
			if (exception)
			{

			}
		}

		template <typename T>
		void SetListField(const std::string& name, const std::vector<T>& val)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// Some fail safes i guess
			// incase its out of sync 
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if any of these aren't ready then dont continue w anything
			if (listObject == nullptr || field.mListGetCount == nullptr || field.mListGetItem == nullptr)
				return;

			MonoObject* exception = nullptr;

			MonoObject* countObj = mono_runtime_invoke(field.mListGetCount, listObject, nullptr, &exception);

			// TODO: add in exception handling like in my other invoke stuff

			int count = *(int*)mono_object_unbox(countObj);

			if (count != val.size())
			{
				SLICE_LOG_ERROR("C# List " + name + " is not in sync");
			}

			for (size_t i = 0; i < val.size(); ++i)
			{
				SetListFieldValue(name, static_cast<int>(i), val[i]);
			}
		}

		template<>
		void SetListField<std::string>(const std::string& name, const std::vector<std::string>& val)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// Some fail safes i guess
			// incase its out of sync 
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if any of these aren't ready then dont continue w anything
			if (listObject == nullptr || field.mListGetCount == nullptr || field.mListGetItem == nullptr)
				return;

			MonoObject* exception = nullptr;

			MonoObject* countObj = mono_runtime_invoke(field.mListGetCount, listObject, nullptr, &exception);

			// TODO: add in exception handling like in my other invoke stuff

			int count = *(int*)mono_object_unbox(countObj);

			if (count != val.size())
			{
				SLICE_LOG_ERROR("C# List " + name + " is not in sync");
			}

			for (size_t i = 0; i < val.size(); ++i)
			{
				SetListFieldValue<std::string>(name, static_cast<int>(i), val[i]);
			}

		}

		template<>
		void SetListField<GameObject>(const std::string& name, const std::vector<GameObject>& val)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// Some fail safes i guess
			// incase its out of sync 
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if any of these aren't ready then dont continue w anything
			if (listObject == nullptr || field.mListGetCount == nullptr || field.mListGetItem == nullptr)
				return;

			MonoObject* exception = nullptr;

			MonoObject* countObj = mono_runtime_invoke(field.mListGetCount, listObject, nullptr, &exception);

			// TODO: add in exception handling like in my other invoke stuff

			int count = *(int*)mono_object_unbox(countObj);

			if (count != val.size())
			{
				SLICE_LOG_ERROR("C# List " + name + " is not in sync");
			}

			for (size_t i = 0; i < val.size(); ++i)
			{
				SetListFieldValue<GameObject>(name, static_cast<int>(i), val[i]);
			}

		}

		template<>
		void SetListField<PrefabVar>(const std::string& name, const std::vector<PrefabVar>& val)
		{
			if (mono_domain_get() != gScriptSystem->mAppDomain)
			{
				mono_thread_attach(gScriptSystem->mRootDomain);
				mono_domain_set(gScriptSystem->mAppDomain, false);
			}

			// Some fail safes i guess
			// incase its out of sync 
			const ScriptField& field = mScriptClass->mFields.at(name);

			MonoObject* listObject = mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);

			// if any of these aren't ready then dont continue w anything
			if (listObject == nullptr || field.mListGetCount == nullptr || field.mListGetItem == nullptr)
				return;

			MonoObject* exception = nullptr;

			MonoObject* countObj = mono_runtime_invoke(field.mListGetCount, listObject, nullptr, &exception);

			// TODO: add in exception handling like in my other invoke stuff

			int count = *(int*)mono_object_unbox(countObj);

			if (count != val.size())
			{
				SLICE_LOG_ERROR("C# List " + name + " is not in sync");
			}

			for (size_t i = 0; i < val.size(); ++i)
			{
				SetListFieldValue<PrefabVar>(name, static_cast<int>(i), val[i]);
			}

		}


#pragma endregion


		template <typename T>
		void SetFieldValue(const std::string& name, T val)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0)
			{
				return;
			}
			auto iter = fields.find(name);
			const ScriptField& field = iter->second;

			//void* valPtr =;
			mono_field_set_value(mMonoInstance, field.mClassField, (void*)&val);
		}

		template <>
		void SetFieldValue<std::string>(const std::string& name, std::string val)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0)
			{
				return;
			}
			auto iter = fields.find(name);
			const ScriptField& field = iter->second;

			MonoString* monoStr = mono_string_new(mono_domain_get(), val.c_str());
			if (monoStr != nullptr) {
				// Set the MonoString as the value of the field
				mono_field_set_value(mMonoInstance, field.mClassField, monoStr);
			}
		}
		template <>
		void SetFieldValue<PrefabVar>(const std::string& name, PrefabVar val)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0) return;

			const ScriptField& field = fields.at(name);

			// 1. Get the MonoClass for the C# Prefab
			// Note: You should ideally cache this MonoClass* in your ScriptSystem to avoid lookups
			MonoClass* prefabClass = mono_class_from_name(gScriptSystem->mCoreAssemblyImage, "SliceEngine", "Prefab");

			if (!prefabClass) return;

			// 2. Create a new managed instance of the C# Prefab
			MonoObject* managedInstance = mono_object_new(mono_domain_get(), prefabClass);

			// 3. Initialize the object (Calls the constructor)
			// We can call the constructor that takes a uint ID
			void* args[1];
			MonoString* monoStr = mono_string_new(mono_domain_get(), val.prefabFileName.c_str());
			args[0] = monoStr;

			// Find the constructor: GameObject(uint id)
			MonoMethod* ctor = mono_class_get_method_from_name(prefabClass, ".ctor", 1);
			mono_runtime_invoke(ctor, managedInstance, args, nullptr);

			// 4. Set the field in your ScriptObject to this new C# object reference
			// Since it's a reference type, we pass the pointer to the MonoObject itself
			mono_field_set_value(mMonoInstance, field.mClassField, managedInstance);
		}


		template <>
		void SetFieldValue<GameObject>(const std::string& name, GameObject val)
		{
			const auto& fields = mScriptClass->mFields;
			if (fields.count(name) == 0) return;

			const ScriptField& field = fields.at(name);

			// 1. Get the MonoClass for the C# GameObject
			// Note: You should ideally cache this MonoClass* in your ScriptSystem to avoid lookups
			MonoClass* gameObjectClass = mono_class_from_name(gScriptSystem->mCoreAssemblyImage, "SliceEngine", "GameObject");

			if (!gameObjectClass) return;

			//// 2. Create a new managed instance of the C# GameObject
			//MonoObject* managedInstance = mono_object_new(mono_domain_get(), gameObjectClass);

			//// 3. Initialize the object (Calls the constructor)
			//// We can call the constructor that takes a uint ID
			//void* args[1];
			//uint32_t entityID = (uint32_t)val.GetEntity();
			//args[0] = &entityID;

			//// Find the constructor: GameObject(uint id)
			//MonoMethod* ctor = mono_class_get_method_from_name(gameObjectClass, ".ctor", 1);
			//mono_runtime_invoke(ctor, managedInstance, args, nullptr);

			MonoObject* managedInstance = gScriptSystem->GetOrCreateManagedObject(val.GetEntity());

			// 4. Set the field in your ScriptObject to this new C# object reference
			// Since it's a reference type, we pass the pointer to the MonoObject itself
			mono_field_set_value(mMonoInstance, field.mClassField, managedInstance);
		}

		MonoObject* GetListObject(const std::string& name);


		std::shared_ptr<ScriptClass> GetScriptClass();

	};



}

#endif