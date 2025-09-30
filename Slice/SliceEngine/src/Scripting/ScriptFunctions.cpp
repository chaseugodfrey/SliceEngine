#include <pch.h>
#include "ScriptFunctions.h"
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include "ScriptSystem.h"
#include "../Core/Core.h"
#include "../Input/InputSystem.h"

namespace SliceEngine
{
	std::string MonoToString(MonoString* monoStr)
	{
		if (!monoStr) return "";

		char* utf8Str = mono_string_to_utf8(monoStr);
		if (!utf8Str) return "";

		std::string result(utf8Str);
		mono_free(utf8Str);

		return result;
	}

	// Define to make it easier to add internal function calls
	#define ADD_INTERNAL_CALL(Name) mono_add_internal_call("SliceEngine.FunctionCalls::" #Name, Name)
	
	static void Transform_GetPosition(unsigned int entity, glm::vec3* outPosition)
	{
		//SLICE_LOG("Getting position from C++ for entity: {}", entity);
		// note if we add UI objects
		// this might need to be modified to check which transform it has
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		
		*outPosition = transform.position;
	}

	static void Transform_SetPosition(unsigned int entity, glm::vec3* position)
	{
		//SLICE_LOG("Setting position from C++ for entity: {}", entity);
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		transform.position = *position;
	}

	static bool IsKeyPressed(Keys keyCode)
	{

		return Core::GetInstance()->GetInputSystem()->IsKeyPressed(keyCode);
	}

	static bool IsKeyDown(Keys keyCode)
	{
		return Core::GetInstance()->GetInputSystem()->IsKeyDown(keyCode);
	}

#pragma region Console Logging functions

	static void Log(MonoString* string)
	{
		std::string cStrName = MonoToString(string);
		SLICE_LOG_DEBUG(cStrName);
		//CM_CORE_INFO(cStrName);
		//mono_free(cStr);
	}

	static void LogWarn(MonoString* string)
	{
		std::string cStrName = MonoToString(string);
		SLICE_LOG_WARNING(cStrName);
		//CM_CORE_WARN(cStrName);
		//mono_free(cStr);
	}

	static void LogError(MonoString* string)
	{
		std::string cStrName = MonoToString(string);
		SLICE_LOG_ERROR(cStrName);
		//CM_CORE_ERROR(cStrName);
		//mono_free(cStr);
	}
#pragma endregion



	template <typename T>
	static void RegisterComponent()
	{
		std::string_view typeName = typeid(T).name();
		size_t pos = typeName.find_last_of(':');
		std::string_view structName = typeName.substr(pos + 1);
		// so that we can match the C# equivalent of the component
		std::string modifiedTypename = std::format("SliceEngine.{}", structName);

		MonoType* monoType = mono_reflection_type_from_name(modifiedTypename.data(), gScriptSystem->mCoreAssemblyImage);
		if (!monoType)
		{
			SLICE_LOG_ERROR("Couldn't find component");
			assert("Can't find component");
			return;
		}
		// Old method of storing has component functions
		// mGameObjectHasComponentFuncs[monoType] = [](GameObject go) { return go.HasComponent<T>();  };

	}

		/// <summary>
	/// Register the component. Clear the map before registering
	/// </summary>
	void ScriptFunctions::RegisterComponents()
	{
		// if we hotload and need to rerun the linking and reinit mono
		// then we might need to clear the map before registering again
		//mGameObjectHasComponentFuncs.clear();
		//// Only these 2 for now
		RegisterComponent<Transform>();
		//RegisterComponent<Collider2D>();
		//RegisterComponent<RigidBody>();
		//RegisterComponent<Animation>();
		//RegisterComponent<StateMachine>();
		//RegisterComponent<Renderer>();
		//RegisterComponent<TextRenderer>();
		//RegisterComponent<ParticleEmitter>();
	}

	/// <summary>
	/// Register functions to C# side by using the internal mono call
	/// </summary>
	void ScriptFunctions::RegisterFunctions()
	{
		ADD_INTERNAL_CALL(Transform_GetPosition);
		ADD_INTERNAL_CALL(Transform_SetPosition);

		// Key input
		ADD_INTERNAL_CALL(IsKeyPressed);
		ADD_INTERNAL_CALL(IsKeyDown);

		// Console logging
		ADD_INTERNAL_CALL(Log);
		ADD_INTERNAL_CALL(LogWarn);
		ADD_INTERNAL_CALL(LogError);
	}

}
