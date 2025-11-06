/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ScriptFunctions.cpp

 author:   Gideon Francis

 email:       g.francis@digipen.edu

 brief:       Contains the static functions that are meant to be linked to mono c# so that c# scripts can call. Function Names have to match
				the C# equivalent so that it can bind

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "ScriptFunctions.h"
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include "ScriptSystem.h"
#include "../Core/Core.h"
#include "../Input/InputSystem.h"
#include "../Physics/PhysicsSystem.h"
#include "../Logger/Logger.h"
#include "../Graphics/TransformHelper.h"

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

	static std::unordered_map<MonoType*, std::function<bool(GameObject)>> mGameObjectHasComponentFuncs;

	// Define to make it easier to add internal function calls
	#define ADD_INTERNAL_CALL(Name) mono_add_internal_call("SliceEngine.FunctionCalls::" #Name, Name)

#pragma region ENTITY FUNCTIONS
	static MonoArray* Entity_FindEntitiesWithTag(MonoString* tag)
	{
		std::string cStrName = MonoToString(tag);

		std::vector<Entity> entityIDs = FactoryInstance.GetEntitiesWithTag(cStrName);


		MonoDomain* domain = mono_domain_get();
		MonoArray* monoArray = mono_array_new(domain, mono_get_uint32_class(), entityIDs.size());

		for (size_t i = 0; i < entityIDs.size(); ++i)
		{
			mono_array_set(monoArray, uint32_t, i, static_cast<uint32_t>(entityIDs[i]));
		}

		return monoArray;
	}
#pragma endregion

#pragma region TRANSFORM FUNCTIONS

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

	static void Transform_GetScale(unsigned int entity, glm::vec3* outScale)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		*outScale = transform.scale;
	}
	
	static void Transform_SetScale(unsigned int entity, glm::vec3* scale)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		transform.scale = *scale;
	}

	static void Transform_GetRotation(unsigned int entity, glm::vec3* outRotation)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		glm::vec3 euler = SliceEngine::QuatToVec3(transform.rotation);
		*outRotation = euler;
		// leaving blank for now cause i think i ahve to return as euler not quaternion
	}

	static void Transform_SetRotation(unsigned int entity, glm::vec3* rotation)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		transform.rotation = SliceEngine::Vec3ToQuat(*rotation);
		// leaving blank for now cause i think i ahve to return as euler not quaternion
	}

#pragma endregion

	static bool IsKeyPressed(Keys keyCode)
	{

		return Core::GetInstance()->GetInputSystem()->IsKeyPressed(keyCode);
	}

	static bool IsKeyDown(Keys keyCode)
	{
		return Core::GetInstance()->GetInputSystem()->IsKeyDown(keyCode);
	}

#pragma region CONSOLE LOGGING FUNCTIONS

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

#pragma region RIGIDBODY FUNCTIONS

	static void RigidBody_GetVelocity(unsigned int entity, glm::vec3* outPosition)
	{
		//SLICE_LOG("Getting velocity from C++ for entity: {}", entity);

		*outPosition = Core::GetInstance()->GetSystem<PhysicsSystem>().GetLinearVelocity((Entity)entity);
	}

	static void RigidBody_SetVelocity(unsigned int entity, glm::vec3* position)
	{
		//SLICE_LOG("Setting velocity from C++ for entity: {}", entity);

		JPH::Vec3 vel(position->x, position->y, position->z);

		Core::GetInstance()->GetSystem<PhysicsSystem>().SetLinearVelocity((Entity)entity, vel);
	}

	static void RigidBody_AddForce(unsigned int entity, JPH::Vec3* force, int mode)
	{
		switch (mode)
		{
		case 0: // Force
			Core::GetInstance()->GetSystem<PhysicsSystem>().AddForceToEntity((Entity)entity, *force);
			break;
		case 1: // Impulse
			Core::GetInstance()->GetSystem<PhysicsSystem>().AddImpulseToEntity((Entity)entity, *force);
			break;
		case 2: // Velocity Change
			Core::GetInstance()->GetSystem<PhysicsSystem>().AddVelocityChangeToEntity((Entity)entity, *force);
			break;
		case 3: // Acceleration
			Core::GetInstance()->GetSystem<PhysicsSystem>().AddAccelerationToEntity((Entity)entity, *force);
			break;
		default:
			SLICE_LOG_ERROR("if u somehow made it come here i'll be dissapointed");
			break;
		}
	}

#pragma endregion

#pragma region AUDIO FUNCTIONS

	static MonoString* Audio_GetSoundName(unsigned int entity)
	{
		//SLICE_LOG("Getting audio name from C++ for entity: {}", entity);
		
		auto& audio = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<AudioSource>();

		std::string test;

		for (auto pair : Core::GetInstance()->GetResourceManager()->mFileNameToGUID)
		{
			if(audio.soundGUID == pair.second)
			{
				test = pair.first;
				break;
			}
		}

		if(test == "")
		{
			return nullptr;
		}

		return mono_string_new(mono_domain_get(), test.c_str());
		
	}

	//static void Audio_SetSoundName(unsigned int entity, MonoString* string)
	//{
	//	//SLICE_LOG("Setting audio name from C++ for entity: {}", entity);

	//	std::string str = MonoToString(string);

	//	auto& audio = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<AudioSource>();
	//	audio.soundName = str;

	//}


#pragma endregion

#pragma region ENTITY FUNCTIONS
	static bool Entity_HasComponent(unsigned int entityID, MonoReflectionType* componentType)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		MonoType* monoType = mono_reflection_type_get_type(componentType);

		if (mGameObjectHasComponentFuncs.count(monoType) <= 0)
		{
			// component not registered
			SLICE_LOG_ERROR("Component Not Registered");
			assert("Component not registered");
		}

		return mGameObjectHasComponentFuncs[monoType](GO);

	}
#pragma endregion

#pragma region ANIMATION FUNCTIONS
	static void ChangeAnim(unsigned int entityID, unsigned int animID)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			auto& anim = GO.GetComponent<Animator>();
		//	anim.stateMachine.EFSM.currState->curr_anim_idx = animID;
		}
		else
		{
			SLICE_LOG_DEBUG("Entity does not have animator");
		}
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
		 mGameObjectHasComponentFuncs[monoType] = [](GameObject go) { return go.HasComponent<T>();  };
	}

		/// <summary>
	/// Register the component. Clear the map before registering
	/// </summary>
	void ScriptFunctions::RegisterComponents()
	{
		// if we hotload and need to rerun the linking and reinit mono
		// then we might need to clear the map before registering again
		mGameObjectHasComponentFuncs.clear();
		//// Only these 2 for now
		RegisterComponent<Transform>();
		RegisterComponent< Animator>();
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
		// Entity 
		ADD_INTERNAL_CALL(Entity_HasComponent);

		// Transforms
		ADD_INTERNAL_CALL(Transform_GetPosition);
		ADD_INTERNAL_CALL(Transform_SetPosition);
		ADD_INTERNAL_CALL(Transform_GetScale);
		ADD_INTERNAL_CALL(Transform_SetScale);
		ADD_INTERNAL_CALL(Transform_GetRotation);
		ADD_INTERNAL_CALL(Transform_SetRotation);

		// Key input
		ADD_INTERNAL_CALL(IsKeyPressed);
		ADD_INTERNAL_CALL(IsKeyDown);

		// Console logging
		ADD_INTERNAL_CALL(Log);
		ADD_INTERNAL_CALL(LogWarn);
		ADD_INTERNAL_CALL(LogError);

		//Physics
		ADD_INTERNAL_CALL(RigidBody_GetVelocity);
		ADD_INTERNAL_CALL(RigidBody_SetVelocity);
		ADD_INTERNAL_CALL(RigidBody_AddForce);

		// Audio
		ADD_INTERNAL_CALL(Audio_GetSoundName);
		//ADD_INTERNAL_CALL(Audio_SetSoundName);

		// Animator
		ADD_INTERNAL_CALL(ChangeAnim);
	}

}
