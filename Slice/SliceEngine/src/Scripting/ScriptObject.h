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

namespace SliceEngine
{
	// Types that a script field can be
	enum class ScriptFieldType
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
		Entity,
		String
	};

	//struct
	struct ScriptField
	{
		ScriptFieldType mType{ ScriptFieldType::None };
		std::string mName{};
		MonoClassField* mClassField{ nullptr };

		// used purely for prefab serialization
		// if not we'd normally just use name to retrieve
		// the variable and set the variable in runtime
		rttr::variant value;

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