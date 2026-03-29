/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ScriptObject.cpp

 author:   Gideon Francis

 email:       g.francis@digipen.edu

 brief:      Script object. Holds the equivalent c# script, references to the standard OnCreate, OnUpdate and OnConstruct functions.
				Used as a shared_ptr in scriptSystem.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


#include <pch.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include "ScriptSystem.h"
#include "ScriptObject.h"

namespace SliceEngine
{
#pragma region ScriptClass

	ScriptClass::ScriptClass(const std::string& nameSpace, const std::string& className) : mNameSpace(nameSpace), mClassName(className)
	{
		mMonoClass = mono_class_from_name(gScriptSystem->mCoreAssemblyImage, nameSpace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		MonoObject* monoInstance = mono_object_new(gScriptSystem->mAppDomain, mMonoClass);

		MonoMethod* ctor = mono_class_get_method_from_name(mMonoClass, ".ctor", 0);
		if (ctor)
		{
			MonoObject* exception = nullptr;
			mono_runtime_invoke(ctor, monoInstance, nullptr, &exception);
			if (exception)
			{
				MonoString* exceptionMsg = mono_object_to_string(exception, nullptr);
				char* errorMsg = mono_string_to_utf8(exceptionMsg);
				std::cerr << "Mono Exception: " << errorMsg << std::endl;
				mono_free(errorMsg);
				return nullptr;
			}
		}
		else
		{
			SLICE_LOG_ERROR("Unable to default construct class " + mClassName);
		}

		
		return monoInstance;
		//monoInstance = instance;
		//return instance;
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int varCount)
	{
		MonoClass* currentClass = mMonoClass;
		MonoMethod* method = nullptr;

		while (currentClass != nullptr && method == nullptr)
		{
			const char* className = mono_class_get_name(currentClass);
			////std::cout << "Looking for : " << name << " in " << className << std::endl;
			// dont get the very base slice behaviour
			if (std::string(className) == "SliceBehaviour" && name != ".ctor")
				break;

			method = mono_class_get_method_from_name(currentClass, name.c_str(), varCount);

			if (!method)
			{
				currentClass = mono_class_get_parent(currentClass);
			}
		}
		return method;
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		if (!method)
		{
			SLICE_LOG_ERROR("Instance something something");
			return nullptr;
		}

		if (!instance)
		{
			SLICE_LOG_ERROR("Instance something something");
			return nullptr;
		}

		if (mono_domain_get() != gScriptSystem->mAppDomain)
		{
			// Attach the current C++ thread to the Mono JIT runtime
			// This function is idempotent (safe to call if already attached),
			// but we must use the root domain.
			mono_thread_attach(gScriptSystem->mRootDomain);

			// Set the current AppDomain for this thread
			mono_domain_set(gScriptSystem->mAppDomain, false);
		}


		/*if (instance->synchronisation == nullptr || instance->vtable == nullptr)
		{
			SLICE_LOG_ERROR("Instance something something");
			return nullptr;
		}*/

		if (mono_object_get_class(instance) == nullptr)
		{
			SLICE_LOG_ERROR("Instance something something");
			return nullptr;
		}

		MonoClass* methodClass = mono_method_get_class(method);
		const char* className = methodClass ? mono_class_get_name(methodClass) : "UnknownClass";
		const char* methodName = mono_method_get_name(method);

		//UNUSED(className);
		//UNUSED(methodName);

		int paramCount = mono_signature_get_param_count(mono_method_signature(method));
		if (paramCount > 0 && !params)
		{
			SLICE_LOG_ERROR("Instance something something");
			return nullptr;
		}




		// Exception so that we can check if any invoke fails
		MonoObject* exception = nullptr;
		MonoObject* result = nullptr;
		try
		{
			result = mono_runtime_invoke(method, instance, params, &exception);
		}
		catch (...)
		{
			SLICE_LOG_ERROR("InvokeMethod: Unknown exception occurred while invoking {} in Class: {}" + std::string(methodName) + std::string(className));
			return nullptr;

		}

		if (exception)
		{
			MonoString* exceptionMsg = mono_object_to_string(exception, nullptr);
			char* errorMsg = mono_string_to_utf8(exceptionMsg);
			std::cerr << "Mono Exception: " << errorMsg << std::endl;
			mono_free(errorMsg);
			return nullptr;
		}

		return result;
	}
#pragma endregion 

#pragma region ScriptObject

	ScriptObject::ScriptObject(std::shared_ptr<ScriptClass> scClass, Entity entity) : mScriptClass(scClass)
	{

		////std::cout << "Initializing script object for entity " << (uint32_t)entity << std::endl;
		//UNUSED(entity);
		mMonoInstance = scClass->Instantiate();
		ScriptClass mEntityClass = ScriptClass("SliceEngine", "SliceBehaviour");
		// Need to call constructor of entity by getting the entity class thats storing Entity.cs
		mConstruct = mEntityClass.GetMethod(".ctor", 1);

		// These are the other functions that every other script that inherits Entity will have
		mOnAwake = scClass->GetMethod("OnAwake", 0);
		mOnCreate = scClass->GetMethod("OnCreate", 0);
		mOnUpdate = scClass->GetMethod("OnUpdate", 1);
		mOnFixedUpdate = scClass->GetMethod("OnFixedUpdate", 1);
		mOnLateUpdate = scClass->GetMethod("OnLateUpdate", 1);
		mOnEntityDestroy = scClass->GetMethod("OnEntityDestroy", 1);
		//mOnClick = scClass->GetMethod("OnClick", 0);
		mOnEntityEnabled = scClass->GetMethod("OnEnabled", 0);
		mOnEntityDisabled = scClass->GetMethod("OnDisabled", 0);

		//// Collision functions
		mOnCollideEnter = scClass->GetMethod("OnCollideEnter", 1);
		mOnCollideStay = scClass->GetMethod("OnCollideStay", 1);
		mOnCollideExit = scClass->GetMethod("OnCollideExit", 1);			
		mOnTriggerEnter = scClass->GetMethod("OnTriggerEnter", 1);
		mOnTriggerStay = scClass->GetMethod("OnTriggerStay", 1);
		mOnTriggerExit = scClass->GetMethod("OnTriggerExit", 1);

		//// Mouse functions
		//mOnMouseEnter = scClass->GetMethod("OnMouseEnter", 0);
		//mOnMouseExit = scClass->GetMethod("OnMouseExit", 0);
		//mOnMouseHover = scClass->GetMethod("OnMouseHover", 0);

		//UI Functions
		mOnButtonClick = scClass->GetMethod("OnButtonClick", 0);
		mOnButtonHover = scClass->GetMethod("OnButtonHover", 0);
		mOnButtonExitHover = scClass->GetMethod("OnButtonExitHover", 0);
		mOnButtonRelease = scClass->GetMethod("OnButtonRelease", 0);
		mOnSliderValue = scClass->GetMethod("OnSliderValue", 1);
		mOnSAnimStop = scClass->GetMethod("OnSpriteAnimStop", 0);
		mOnSAnimLoop = scClass->GetMethod("OnSpriteAnimLoop", 0);

		//mOnStateEnter = scClass->GetMethod("OnStateEnter", 1);
		//mOnStateUpdate = scClass->GetMethod("OnStateUpdate", 2);
		//mOnStateExit = scClass->GetMethod("OnStateExit", 1);

		mHandle = mono_gchandle_new(mMonoInstance, true);
	}

	void ScriptObject::Destroy()
	{
		if (mHandle)
		{
			mono_gchandle_free(mHandle);
			mHandle = 0;
			mMonoInstance = nullptr;
		}
	}

	ScriptObject::~ScriptObject()
	{
		Destroy();
	}

	MonoObject* ScriptObject::GetInstance()
	{
		return mMonoInstance;
	}

	void ScriptObject::InvokeOnConstruct(unsigned int id)
	{
		if (mConstruct)
		{
			// Invoke constructor when setting up entity
			void* param = &id;
			mScriptClass->InvokeMethod(mMonoInstance, mConstruct, &param);

		}

	}

	void ScriptObject::InvokeOnAwake()
	{
		if (mOnAwake)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnAwake);
		}
	}

	void ScriptObject::InvokeOnCreate()
	{
		if (mOnCreate)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnCreate);
		}
	}

	void ScriptObject::InvokeOnUpdate(float dt)
	{
		if (mOnUpdate)
		{
			void* param = &dt;
			mScriptClass->InvokeMethod(mMonoInstance, mOnUpdate, &param);
		}
	}

	void ScriptObject::InvokeOnFixedUpdate(float dt)
	{
		if (mOnFixedUpdate)
		{
			void* param = &dt;
			mScriptClass->InvokeMethod(mMonoInstance, mOnFixedUpdate, &param);
		}
	}

	void ScriptObject::InvokeOnLateUpdate(float dt)
	{
		if (mOnLateUpdate)
		{
			void* param = &dt;
			mScriptClass->InvokeMethod(mMonoInstance, mOnLateUpdate, &param);
		}
	}

	void ScriptObject::InvokeOnEntityDestroy(unsigned int id)
	{
		if (mOnEntityDestroy)
		{
			void* param = &id;
			mScriptClass->InvokeMethod(mMonoInstance, mOnEntityDestroy, &param);
		}
	}


	void ScriptObject::InvokeOnClick()
	{
		if (mOnClick)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnClick);
		}
	}

	void ScriptObject::InvokeButtonOnClick()
	{
		if (mOnButtonClick)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnButtonClick);
		}
	}
	void ScriptObject::InvokeButtonOnHover()
	{
		if (mOnButtonHover)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnButtonHover);
		}
	}
	void ScriptObject::InvokeButtonOnExitHover()
	{
		if (mOnButtonExitHover)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnButtonExitHover);
		}
	}
	void ScriptObject::InvokeButtonOnRelease()
	{
		if (mOnButtonRelease)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnButtonRelease);
		}
	}

	void ScriptObject::InvokeOnSliderValue(float val)
	{
		if (mOnSliderValue)
		{
			void* param = &val;
			mScriptClass->InvokeMethod(mMonoInstance, mOnSliderValue, &param);
		}
	}
	void ScriptObject::InvokeSAnimStop()
	{
		if (mOnSAnimStop)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnSAnimStop);
		}
	}
	void ScriptObject::InvokeSAnimLoop()
	{
		if (mOnSAnimLoop)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnSAnimLoop);
		}
	}

	void ScriptObject::InvokeOnCollideEnter(unsigned int id)
	{
		//UNUSED(otherID);
		if (mOnCollideEnter)
		{
			void* param = &id;
			mScriptClass->InvokeMethod(mMonoInstance, mOnCollideEnter, &param);
		}
	}

	void ScriptObject::InvokeOnCollideStay(unsigned int id)
	{
		//UNUSED(otherID);
		if (mOnCollideStay)
		{
			void* param = &id;
			mScriptClass->InvokeMethod(mMonoInstance, mOnCollideStay, &param);
		}
	}

	void ScriptObject::InvokeOnCollideExit(unsigned int id)
	{
		//UNUSED(otherID);
		if (mOnCollideExit)
		{
			void* param = &id;
			mScriptClass->InvokeMethod(mMonoInstance, mOnCollideExit, &param);
		}
	}

	void ScriptObject::InvokeOnTriggerEnter(unsigned int id)
	{
		if (mOnTriggerEnter)
		{
			void* param = &id;
			//CM_CORE_INFO("Calling the Invoke Method onTriggerEnter");
			mScriptClass->InvokeMethod(mMonoInstance, mOnTriggerEnter, &param);

		}
	}

	void ScriptObject::InvokeOnTriggerStay(unsigned int id)
	{
		if (mOnTriggerStay)
		{
			void* param = &id;

			mScriptClass->InvokeMethod(mMonoInstance, mOnTriggerStay, &param);
		}
	}

	void ScriptObject::InvokeOnTriggerExit(unsigned int id)
	{
		if (mOnTriggerExit)
		{
			void* param = &id;
			//CM_CORE_INFO("Calling the Invoke Method onTriggerExit");
			mScriptClass->InvokeMethod(mMonoInstance, mOnTriggerExit, &param);
		}
	}


	/// <summary>
	/// Call when a mouse enters the collider box of an object
	/// </summary>
	void ScriptObject::InvokeOnMouseEnter()
	{
		//CM_CORE_INFO("Testing");

		if (mOnMouseEnter)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnMouseEnter);
		}
	}

	/// <summary>
	/// Call when a mouse leaves the collider box of an object
	/// </summary>
	void ScriptObject::InvokeOnMouseExit()
	{
		//CM_CORE_INFO("Testing");
		if (mOnMouseExit)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnMouseExit);
		}
	}

	/// <summary>
	/// Call when a mouse is hovering the collider box of an object
	/// </summary>
	void ScriptObject::InvokeOnMouseHover()
	{
		if (mOnMouseHover)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnMouseHover);
		}
	}

	void ScriptObject::InvokeOnEnabled()
	{
		if (mOnEntityEnabled)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnEntityEnabled);
		}
	}

	void ScriptObject::InvokeOnDisabled()
	{
		if (mOnEntityDisabled)
		{
			mScriptClass->InvokeMethod(mMonoInstance, mOnEntityDisabled);
		}
	}

	void ScriptObject::InvokeOnStateEnter(std::string stateName)
	{
		if (mOnStateEnter)
		{
			MonoString* monoStateName = mono_string_new(mono_domain_get(), stateName.c_str());

			void* param = monoStateName;
			mScriptClass->InvokeMethod(mMonoInstance, mOnStateEnter, &param);

		}
	}

	void ScriptObject::InvokeOnStateUpdate(std::string stateName, float dt)
	{
		if (mOnStateUpdate && mMonoInstance)
		{
			void* param[2];
			MonoString* monoStateName = mono_string_new(mono_domain_get(), stateName.c_str());
			param[0] = monoStateName;
			param[1] = &dt;

			mScriptClass->InvokeMethod(mMonoInstance, mOnStateUpdate, param);
		}
	}

	void ScriptObject::InvokeOnStateExit(std::string stateName)
	{
		if (mOnStateExit)
		{
			MonoString* monoStateName = mono_string_new(mono_domain_get(), stateName.c_str());

			void* param = monoStateName;
			mScriptClass->InvokeMethod(mMonoInstance, mOnStateExit, &param);

		}
	}

	/// <summary>
	/// Use this to get out all the variables of this script obj for editor use
	/// </summary>
	/// <returns></returns>
	std::map<std::string, rttr::variant> ScriptObject::GetAllFields()
	{
		std::map<std::string, rttr::variant> fieldsMap;
		const auto& fieldVariables = mScriptClass->mFields;

		for (const auto& [name, variable] : fieldVariables)
		{
			fieldsMap[name] = GetFieldValue(name);
		}

		return fieldsMap;
	}


	// NOTE: This probably wont work with arrays now
	// this isnt even being used atm so ill jus leave this here and delete before M3
	// if i rmb
	void ScriptObject::ExposeForEditor(const std::function<void(const std::string&, rttr::variant&)>& editorCall)
	{
		const auto& fieldVariables = mScriptClass->mFields;
		for (const auto& [name, field] : fieldVariables)
		{
			rttr::variant currValue = GetFieldValue(name);
			if (!currValue.is_valid())
				continue;

			rttr::variant originalVal = currValue;

			editorCall(name, currValue);

			/*
			if editor want to use this
			std::shared_ptr<SliceEngine::ScriptObject> scriptRef to get the reference to the script instance
			then 
			scriptRef->ExposeForEditor([](const std::string& name, rttr::variant& var)
			{
				// you can do w/e u need to with the variable
				// get the type to check how to display it
				// i.e rttr::type type = var.get_type();

				// this function will handle updating the script instance after setting value in editor

			});

			
			*/

			if (currValue != originalVal)
			{
				SetFieldValue(name, currValue);
			}
		}

	}

	rttr::variant ScriptObject::GetFieldValue(const std::string& name)
	{
		const auto& fields = mScriptClass->mFields;
		auto it = fields.find(name);
		if (it == mScriptClass->mFields.end())
		{
			return {};
		}

		const ScriptField& field = it->second;

		return GetMonoFieldValue(mMonoInstance, field.mClassField);
	}

	void ScriptObject::SetFieldValue(const std::string& name, rttr::variant val)
	{
		auto& fields = mScriptClass->mFields;
		auto it = fields.find(name);
		if (it == mScriptClass->mFields.end())
		{
			return;
		}

		ScriptField& field = it->second;
		field.value = val;
		SetMonoFieldValue(mMonoInstance, field.mClassField, val);
	}

	rttr::variant ScriptObject::GetMonoFieldValue(MonoObject* scriptInstance, MonoClassField* field)
	{
		MonoObject* valueObj = mono_field_get_value_object(mono_object_get_domain(scriptInstance), field, scriptInstance);
		
		if (!valueObj)
		{
			return {};
		}

		void* unboxPtr = mono_object_unbox(valueObj);

		MonoType* type = mono_field_get_type(field);
		int monoTypeEnum = mono_type_get_type(type);
		std::string result;

		switch (monoTypeEnum)
		{
		case MONO_TYPE_BOOLEAN:
			return *(bool*)unboxPtr;
			break;
		case MONO_TYPE_I4:
			return *(int32_t*)unboxPtr;
			break;
		case MONO_TYPE_U4:
			return *(uint32_t*)unboxPtr;
			break;
		case MONO_TYPE_R4:
			return *(float*)unboxPtr;
			break;
		case MONO_TYPE_R8:
			return *(double*)unboxPtr;
			break;
		case MONO_TYPE_STRING:
			{
				MonoString* monoStr = reinterpret_cast<MonoString*>(mono_field_get_value_object(mono_domain_get(), field, scriptInstance));
				if (monoStr != nullptr)
				{
					char* utf8str = mono_string_to_utf8(monoStr);
					result = utf8str;
					mono_free(utf8str);
				}
				return result;
			}
			break;
		case MONO_TYPE_CLASS:
		{
			MonoObject* obj = mono_field_get_value_object(mono_domain_get(), field, scriptInstance);
			if (!obj) return {};

			MonoClass* objClass = mono_object_get_class(obj);
			std::string className = mono_class_get_name(objClass);

			if (className == "Prefab") 
			{
				// Extract the string field (e.g., "prefabPath") from the C# Prefab class
				MonoClassField* pathField = mono_class_get_field_from_name(objClass, "prefabName");
				MonoString* monoStr = nullptr;
				mono_field_get_value(obj, pathField, &monoStr);
				MonoString* strVal = reinterpret_cast<MonoString*>(mono_field_get_value_object(mono_domain_get(), field, scriptInstance));
				if (strVal != nullptr)
				{
					char* utf8str = mono_string_to_utf8(strVal);
					result = utf8str;
					mono_free(utf8str);
				}

				return PrefabVar{ result };
			}
		}
		break;
		case MONO_TYPE_VALUETYPE:
			std::string typeName = mono_type_get_name(type);

			// Check for value type like vectors and stuff
			if (typeName == "SliceEngine.Prefab")
			{
				MonoObject* monoValueObj = mono_field_get_value_object(mono_domain_get(), field, scriptInstance);
				
				if (monoValueObj == nullptr)
				{
					return PrefabVar{ "" };
				}
				
				void* monoUnboxPtr = mono_object_unbox(monoValueObj);

				MonoClass* prefabClass = mono_type_get_class(type);

				// get the field related to "prefabName" from c#'s prefab.cs
				MonoClassField* nameField = mono_class_get_field_from_name(prefabClass, "prefabName");

				MonoString* monoStr = nullptr;
				mono_field_get_value((MonoObject*)monoUnboxPtr, nameField, &monoStr);

				if (monoStr)
				{
					char* utf8 = mono_string_to_utf8(monoStr);
					std::string name(utf8);
					mono_free(utf8);
					return PrefabVar{ name };
				}

				return PrefabVar{ "" };
			}
			break;
		}

		// if nth then just return a empty variant
		return {};
	}

	void ScriptObject::SetMonoFieldValue(MonoObject* scriptInstance, MonoClassField* field, rttr::variant& value)
	{
		if (!value.is_valid())
		{
			return;
		}

		rttr::type type = value.get_type();

		// if its an array
		if (type.is_sequential_container())
		{
			auto view = value.create_sequential_view();
			size_t size = view.get_size();

			MonoType* fieldType = mono_field_get_type(field);
			MonoClass* elementClass = mono_class_get_element_class(mono_type_get_class(fieldType));

			MonoArray* monoArray = mono_array_new(mono_domain_get(), elementClass, size);
			rttr::type elementType = view.get_value_type();

			// if its a string
			if (elementType == rttr::type::get<std::string>())
			{
				// for how many strings are in the array
				for (size_t i = 0; i < size; ++i)
				{
					rttr::variant elementVal = view.get_value(i);

					if (elementVal.get_type().is_wrapper())
					{
						elementVal = elementVal.extract_wrapped_value();
					}

					std::string& str = elementVal.get_value<std::string>();
					MonoString* monoStr = mono_string_new(mono_domain_get(), str.c_str());
					mono_array_setref(monoArray, i, monoStr);
				}
			}
			else
			{
				int elementSize = mono_class_array_element_size(elementClass); // was uintptr_t
				char* bufferStart = mono_array_addr_with_size(monoArray, elementSize, 0);

				// for how many are in the array
				for (size_t i = 0; i < size; ++i)
				{
					rttr::variant elementVal = view.get_value(i);

					if (elementVal.get_type().is_wrapper())
					{
						elementVal = elementVal.extract_wrapped_value();
					}

					void* dataPtr = nullptr;
					// TODO: add more variables, but im only gonna do these 4 for now to test if it works
					if (elementType == rttr::type::get<float>())
					{
						dataPtr = &elementVal.get_value<float>();
					}
					else if (elementType == rttr::type::get<int>())
					{
						dataPtr = &elementVal.get_value<int>();
					}
					else if (elementType == rttr::type::get<glm::vec3>())
					{
						dataPtr = &elementVal.get_value<glm::vec3>();
					}
					else if (elementType == rttr::type::get<glm::vec2>())
					{
						dataPtr = &elementVal.get_value<glm::vec2>();
					}

					if (dataPtr)
					{
						memcpy(bufferStart + (i * elementSize), dataPtr, elementSize);
					}
				}
			}

			mono_field_set_value(scriptInstance, field, monoArray);
		}
		// if its a non array
		else if (type == rttr::type::get<std::string>())
		{
			// needh andle with MonoString
			std::string& strVal = value.get_value <std::string>();
			MonoString* monoStr = mono_string_new(mono_domain_get(), strVal.c_str());
			mono_field_set_value(scriptInstance, field, monoStr);
		}
		else if (type == rttr::type::get<int>())
		{
			// THE FIX: Get a reference, then take its address
			mono_field_set_value(scriptInstance, field, &value.get_value<int>());
		}
		else if (type == rttr::type::get<float>())
		{
			// THE FIX: Get a reference, then take its address
			mono_field_set_value(scriptInstance, field, &value.get_value<float>());
		}
		else if (type == rttr::type::get<bool>())
		{
			// Bools are special, they must be converted to MonoBoolean (int)
			bool val = value.get_value<bool>();
			MonoBoolean mono_bool = val ? 1 : 0;
			mono_field_set_value(scriptInstance, field, &mono_bool);
		}
		else if (type == rttr::type::get<glm::vec3>())
		{
			mono_field_set_value(scriptInstance, field, &value.get_value<glm::vec3>());
		}
		else if (type == rttr::type::get<glm::vec2>())
		{
			mono_field_set_value(scriptInstance, field, &value.get_value<glm::vec2>());
		}		
		else if (type == rttr::type::get<PrefabVar>())
		{
			MonoClass* prefabClass = mono_class_from_name(gScriptSystem->mCoreAssemblyImage, "SliceEngine", "Prefab");
			MonoObject* prefabInstance = mono_object_new(mono_domain_get(), prefabClass);

			std::string path = value.get_value<PrefabVar>().prefabFileName;
			MonoString* monoStr = mono_string_new(mono_domain_get(), path.c_str());

			// Manual field set or call a constructor
			MonoClassField* pathField = mono_class_get_field_from_name(prefabClass, "prefabName");
			mono_field_set_value(prefabInstance, pathField, monoStr);

			mono_field_set_value(scriptInstance, field, prefabInstance);
		}
	}

	MonoObject* ScriptObject::GetListObject(const std::string& name)
	{
		if (mono_domain_get() != gScriptSystem->mAppDomain)
		{
			mono_thread_attach(gScriptSystem->mRootDomain);
			mono_domain_set(gScriptSystem->mAppDomain, false);
		}
		
		const ScriptField& field = mScriptClass->mFields.at(name);
		return mono_field_get_value_object(mono_domain_get(), field.mClassField, mMonoInstance);
	}

	std::shared_ptr<ScriptClass> ScriptObject::GetScriptClass()
	{

		return mScriptClass;
	}

#pragma endregion
}