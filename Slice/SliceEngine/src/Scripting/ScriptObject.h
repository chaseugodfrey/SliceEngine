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
#include <mono/metadata/threads.h>
#include "ScriptSystem.h"
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
		MonoMethod* mOnCreate = nullptr;
		MonoMethod* mOnUpdate = nullptr;
		MonoMethod* mOnFixedUpdate = nullptr;

		// Standard object functions
		MonoMethod* mOnCollideEnter = nullptr;
		MonoMethod* mOnCollideStay = nullptr;
		MonoMethod* mOnCollideExit = nullptr;
		MonoMethod* mOnTriggerEnter = nullptr;
		MonoMethod* mOnTriggerStay = nullptr;
		MonoMethod* mOnTriggerExit = nullptr;


		// Mouse functions
		MonoMethod* mOnMouseEnter = nullptr;
		MonoMethod* mOnMouseExit = nullptr;
		MonoMethod* mOnMouseHover = nullptr;

		// UI Functions
		MonoMethod* mOnClick = nullptr;

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

		/// <summary>
		/// Get the mMonoInstance reference to get script instance for C# side
		/// </summary>
		MonoObject* GetInstance();


		/// <summary>
		/// Set up all the internal function calls by getting the method and the number of params
		/// </summary>
		/// <param name="entity"></param>
		void SetUpEntity(Entity entity);


		/// <summary>
		/// For calling the OnConstruct function 
		/// </summary>
		/// <param name="id"></param>
		void InvokeOnConstruct(unsigned int id);

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
		/// Call when obj is clicked, if it has a script with an onClick function then itll run it
		/// </summary>
		void InvokeOnClick();

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
				SLICE_LOG_ERROR("No element info for this array: %s\n", name.c_str());
				return;
			}

			MonoArray* monoArray = mono_array_new(mono_domain_get(), field.mElementClass, val.size());

			for (size_t i = 0; i < val.size(); ++i)
			{
				mono_array_set(monoArray, T, i, val[i]);
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
			result.resize(count);

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

#pragma endregion

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
				SLICE_LOG_ERROR("No element info for this array: %s\n", name.c_str());
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

		std::shared_ptr<ScriptClass> GetScriptClass();

	};



}

#endif