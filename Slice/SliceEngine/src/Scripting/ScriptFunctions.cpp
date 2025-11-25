/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ScriptFunctions.cpp

 author:   Gideon Francis

 email:       g.francis@digipen.edu

 brief:       Contains the static functions that are meant to be linked to mono c# so that c# scripts can call. Function Names have to match
				the C# equivalent so that it can bind

Copyright (C) 2025 DigiPen Institute of Technology.
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
#include "../Systems/PrefabSystem.h"
#include "ScriptObject.h"
#include "../Audio/AudioManager.h"
#include "../Configuration/AudioSettings.h"
#include "../Input/ActionMapping.h"

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
		transform.eulerAnglesHint = *rotation;
		// leaving blank for now cause i think i ahve to return as euler not quaternion
	}

	static void Transform_GetRotationQuat(unsigned int entity, glm::quat* outRotation)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		*outRotation = transform.rotation; // REAL QUATERNION
	}

	static void Transform_SetRotationQuat(unsigned int entity, const glm::quat* rotation)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		transform.rotation = glm::normalize(*rotation);

		// update Euler hint only for inspector UI
		transform.eulerAnglesHint = SliceEngine::QuatToVec3(transform.rotation);
	}


#pragma endregion

#pragma region INPUT & ACTIONMAPPING FUNCTIONS

	static bool IsKeyPressed(Keys keyCode)
	{
		return Core::GetInstance()->GetInputSystem()->IsKeyPressed(keyCode);
	}

	static bool IsKeyDown(Keys keyCode)
	{
		return Core::GetInstance()->GetInputSystem()->IsKeyDown(keyCode);
	}

	static bool IsKeyReleased(Keys keyCode)
	{
		return Core::GetInstance()->GetInputSystem()->IsKeyReleased(keyCode);
	}

	static bool IsMousePressed(MouseButtons button)
	{
		return Core::GetInstance()->GetInputSystem()->IsMousePressed(button);
	}

	static bool IsMouseDown(MouseButtons button)
	{
		return Core::GetInstance()->GetInputSystem()->IsMouseDown(button);
	}

	static bool IsMouseReleased(MouseButtons button)
	{
		return Core::GetInstance()->GetInputSystem()->IsMouseReleased(button);
	}

	static glm::vec2 GetMousePosition()
	{
		return Core::GetInstance()->GetInputSystem()->GetMousePosition();
	}

	// allow enabling/disabling action maps from c#
	static void AM_EnableMap(MonoString* map, bool enable)
	{
		auto name = MonoToString(map);
		GetActionMappingSystem().enableMap(name, enable);
	}

	// allow checking if action was performed this frame from c#
	bool AM_PerformedThisFrame(MonoString* map, MonoString* action)
	{
		auto mapName = MonoToString(map);
		auto actionName = MonoToString(action);
		return GetActionMappingSystem().PerformedThisFrame(mapName, actionName);
	}

	// get 1D value from action mapping
	float AM_GetValue1D(MonoString* map, MonoString* action)
	{
		auto mapName = MonoToString(map);
		auto actionName = MonoToString(action);
		return GetActionMappingSystem().GetValue1D(mapName, actionName);
	}

	// get 2D value from action mapping
	glm::vec2 AM_GetValue2D(MonoString* map, MonoString* action)
	{
		auto mapName = MonoToString(map);
		auto actionName = MonoToString(action);
		auto value = GetActionMappingSystem().GetValue2D(mapName, actionName);
		return glm::vec2(value.first, value.second);
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
	static AudioSource* GetAudioComponent(unsigned int entity)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<AudioSource>())
		{
			return &go.GetComponent<AudioSource>();
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no AudioSource component.", entity);
		return nullptr;
	}

	static Transform* GetTransformComponent(unsigned int entity)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<Transform>())
		{
			return &go.GetComponent<Transform>();
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no AudioSource component.", entity);
		return nullptr;
	}

	//Return a filepath
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

	static void Audio_Play(unsigned int entity)
	{
		auto* audioComp = GetAudioComponent(entity);
		auto* transformComp = GetTransformComponent(entity);
		
		if (audioComp)
		{
			
			audioComp->channel = Core::GetInstance()->GetAudioManager()->PlaySound(*(audioComp), transformComp->position, glm::vec3(0.f));
		}
	}

	static void Audio_PlaySFX(MonoString* string)
	{
		std::string key = MonoToString(string);
		Core::GetInstance()->GetAudioSettings()->PlaySFX(key);
	}

	static void Audio_Stop(unsigned int entity)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			if (audioComp->channel)
			{
				Core::GetInstance()->GetAudioManager()->StopSound(audioComp->channel);
				audioComp->channel = nullptr;
			}
		}
	}

	static bool Audio_IsPlaying(unsigned int entity)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			if (audioComp->channel)
			{
				return Core::GetInstance()->GetAudioManager()->IsChannelPlaying(audioComp->channel);
			}
		}
		return false;
	}

	static void Audio_SetPaused(unsigned int entity, bool paused)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			audioComp->isPaused = paused;
			// SoundSystem::UpdateChannelFromComponent will sync this
		}
	}

	static bool Audio_GetPaused(unsigned int entity)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			return audioComp->isPaused;
		}
		return false;
	}

	static void Audio_SetLoop(unsigned int entity, bool loop)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			audioComp->isLoop = loop;
		}
	}

	static bool Audio_GetLoop(unsigned int entity)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			return audioComp->isLoop;
		}
		return false;
	}

	static void Audio_SetVolume(unsigned int entity, float volume)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			audioComp->currentVolume = volume;
		}
	}

	static float Audio_GetVolume(unsigned int entity)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			return audioComp->currentVolume;
		}
		return 0.0f;
	}

	static void Audio_SetPitch(unsigned int entity, float pitch)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			audioComp->pitch = pitch;
		}
	}

	static float Audio_GetPitch(unsigned int entity)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			return audioComp->pitch;
		}
		return 1.0f;
	}

	static void Audio_SetSpatialBlend(unsigned int entity, float blend)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			audioComp->spatialBlend = blend;
		}
	}

	static float Audio_GetSpatialBlend(unsigned int entity)
	{
		if (auto* audioComp = GetAudioComponent(entity))
		{
			return audioComp->spatialBlend;
		}
		return 0.0f;
	}

	static void Audio_SetPan(unsigned int entity, float pan)
	{
		if (auto* audioComp = GetAudioComponent(entity)) audioComp->stereoPan = pan;
	}

	static float Audio_GetPan(unsigned int entity)
	{
		if (auto* audioComp = GetAudioComponent(entity)) return audioComp->stereoPan;
		return 0.0f;
	}

	static void Audio_SetMute(unsigned int entity, bool mute)
	{
		if (auto* audioComp = GetAudioComponent(entity)) audioComp->isMute = mute;
	}

	static bool Audio_GetMute(unsigned int entity)
	{
		if (auto* audioComp = GetAudioComponent(entity)) return audioComp->isMute;
		return false;
	}

	static MonoObject* GetScriptInstance(unsigned int entityID, MonoString* baseName)
	{
		if (gScriptSystem->mEntityInstances.count((Entity)entityID) == 0)
		{
			SLICE_LOG_ERROR("Entity does not have script attached");
			return nullptr;
		}

		std::string cStrName = MonoToString(baseName);

		if (gScriptSystem->mEntityInstances.count((Entity)entityID) > 0)
		{
			return gScriptSystem->mEntityInstances[(Entity)entityID]->GetInstance();
		}


		return nullptr;
	}

	static bool HasScriptInstance(unsigned int entityID, MonoString* baseName)
	{
		if (gScriptSystem->mEntityInstances.count((Entity)entityID) == 0)
		{
			//CM_CORE_ERROR("Entity does not have script attached");
			return false;
		}

		std::string cStrName = MonoToString(baseName);
		if (gScriptSystem->mEntityInstances.count((Entity)entityID) > 0)
		{
			if (gScriptSystem->mEntityInstances[(Entity)entityID]->GetScriptClass()->mClassName == cStrName)
				return true;
		}

		return false;
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

	static unsigned int CreateNewGameObject(MonoString* prefabName)
	{
		std::string cStrName = MonoToString(prefabName);

		auto& prefabSys = Core::GetInstance()->GetSystem<PrefabSystem>();
		auto rm = Core::GetInstance()->GetResourceManager();
		auto it = rm->mFileNameToGUID.find(cStrName);
		if (it != rm->mFileNameToGUID.end())
		{
			GameObject newGO = prefabSys.CreatePrefab((GUID)it->second);
			return(unsigned int)newGO.GetEntity();
		}
			//mono_free(cStrName);

		return entt::null;
	}

	static unsigned int CloneGO(MonoString* GoName)
	{
		std::string cStrName = MonoToString(GoName);
		auto GO = FactoryInstance.GetGOByName(cStrName);

		auto newGO = FactoryInstance.CloneGO(GO);
		return (unsigned int)newGO.GetEntity();
	}

	static uint32_t Entity_FindEntityWithName(MonoString* name)
	{
		std::string cStrName = MonoToString(name);
		auto go = FactoryInstance.GetGOByName(cStrName);

		if (go.IsValid())
		{
			return static_cast<uint32_t>(go.GetEntity());
		}
		else
		{
			return 0;
		}
	}

	static MonoString* Entity_GetTag(unsigned int entityID)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (go.HasComponent<SliceEntity>())
		{
			return mono_string_new(mono_domain_get(), go.GetComponent<SliceEntity>().mTag.c_str());			
		}
	}

	static void Entity_SetTag(unsigned int entityID, MonoString* tag)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (go.HasComponent<SliceEntity>())
		{
			go.GetComponent<SliceEntity>().mTag = MonoToString(tag);
		}
	}

	static void Destroy(unsigned int entity)
	{
		FactoryInstance.Destroy((Entity)entity);
	}
#pragma endregion

#pragma region ANIMATION FUNCTIONS
	static void ChangeAnim(unsigned int entityID, unsigned int animID)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			//auto& anim = GO.GetComponent<Animator>();
		//	anim.stateMachine.EFSM.currState->curr_anim_idx = animID;
		}
		else
		{
			SLICE_LOG_DEBUG("Entity does not have animator");
		}
	}

	static void SetBool(unsigned int entityID, MonoString* string, bool val)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			std::string cStrName = MonoToString(string);

			GO.GetComponent<Animator>().stateMachine.SetBool(cStrName, val);
		}

	}

	static void SetInt(unsigned int entityID, MonoString* string, int val)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			std::string cStrName = MonoToString(string);
			GO.GetComponent<Animator>().stateMachine.SetInt(cStrName, val);

		}
	}

	static void SetFloat(unsigned int entityID, MonoString* string, float val)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			std::string cStrName = MonoToString(string);
			GO.GetComponent<Animator>().stateMachine.SetFloat(cStrName, val);

		}
	}

	static MonoString* GetCurrAnimName(unsigned int entityID)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		std::string tmp;

		if (GO.HasComponent<Animator>())
		{
			if (GO.GetComponent<Animator>().IsValid())
				tmp = GO.GetComponent<Animator>().stateMachine.GetCurrAnimName();
		}

		if(tmp == "")
			return nullptr;

		return mono_string_new(mono_domain_get(), tmp.c_str());
	}

	static bool IsCurrAnimFin(unsigned int entityID)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			return GO.GetComponent<Animator>().stateMachine.IsCurrAnimFin();
		}

		return false;
	}

	static float GetCurrAnimFPS(unsigned int entityID)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			return GO.GetComponent<Animator>().stateMachine.GetCurrAnimFPS();
		}

		return false;
	}

	static float GetCurrAnimTime(unsigned int entityID)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			return GO.GetComponent<Animator>().current_time;
		}

		return 0.0f;
	}

#pragma endregion
	
#pragma region SCENE FUNCTIONS


#pragma endregion

#pragma region COMPONENT REGISTRATION
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
		RegisterComponent<Animator>();
		//RegisterComponent<Collider2D>();
		RegisterComponent<RigidBody>();
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
		ADD_INTERNAL_CALL(Entity_FindEntitiesWithTag);
		ADD_INTERNAL_CALL(CreateNewGameObject);
		ADD_INTERNAL_CALL(Entity_FindEntityWithName);
		ADD_INTERNAL_CALL(Destroy);
		ADD_INTERNAL_CALL(GetScriptInstance);
		ADD_INTERNAL_CALL(HasScriptInstance);
		ADD_INTERNAL_CALL(Entity_GetTag);
		ADD_INTERNAL_CALL(Entity_SetTag);
		ADD_INTERNAL_CALL(CloneGO);

		// Transforms
		ADD_INTERNAL_CALL(Transform_GetPosition);
		ADD_INTERNAL_CALL(Transform_SetPosition);
		ADD_INTERNAL_CALL(Transform_GetScale);
		ADD_INTERNAL_CALL(Transform_SetScale);
		ADD_INTERNAL_CALL(Transform_GetRotation);
		ADD_INTERNAL_CALL(Transform_SetRotation);
		ADD_INTERNAL_CALL(Transform_GetRotationQuat);
		ADD_INTERNAL_CALL(Transform_SetRotationQuat);

		// Key input & action mapping functions, idrk whhat exact functions the designers want so i'll just put down whateva
		ADD_INTERNAL_CALL(IsKeyPressed);
		ADD_INTERNAL_CALL(IsKeyDown);
		ADD_INTERNAL_CALL(IsKeyReleased);
		ADD_INTERNAL_CALL(IsMousePressed);
		ADD_INTERNAL_CALL(IsMouseDown);
		ADD_INTERNAL_CALL(IsMouseReleased);
		ADD_INTERNAL_CALL(GetMousePosition);
		ADD_INTERNAL_CALL(AM_EnableMap);
		ADD_INTERNAL_CALL(AM_PerformedThisFrame);
		ADD_INTERNAL_CALL(AM_GetValue2D);
		ADD_INTERNAL_CALL(AM_GetValue1D);

		// Mouse input
		ADD_INTERNAL_CALL(IsMousePressed);
		ADD_INTERNAL_CALL(IsMouseDown);
		ADD_INTERNAL_CALL(IsMouseReleased);

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
		ADD_INTERNAL_CALL(Audio_Play);
		ADD_INTERNAL_CALL(Audio_PlaySFX);
		ADD_INTERNAL_CALL(Audio_Stop);
		ADD_INTERNAL_CALL(Audio_IsPlaying);
		ADD_INTERNAL_CALL(Audio_SetPaused);
		ADD_INTERNAL_CALL(Audio_GetPaused);
		ADD_INTERNAL_CALL(Audio_SetLoop);
		ADD_INTERNAL_CALL(Audio_GetLoop);
		ADD_INTERNAL_CALL(Audio_SetVolume);
		ADD_INTERNAL_CALL(Audio_GetVolume);
		ADD_INTERNAL_CALL(Audio_SetPitch);
		ADD_INTERNAL_CALL(Audio_GetPitch);
		ADD_INTERNAL_CALL(Audio_SetSpatialBlend);
		ADD_INTERNAL_CALL(Audio_GetSpatialBlend);
		ADD_INTERNAL_CALL(Audio_SetMute);
		ADD_INTERNAL_CALL(Audio_GetMute);
		ADD_INTERNAL_CALL(Audio_SetPan);
		ADD_INTERNAL_CALL(Audio_GetPan);

		// Animator
		ADD_INTERNAL_CALL(ChangeAnim);
		ADD_INTERNAL_CALL(SetBool);
		ADD_INTERNAL_CALL(SetInt);
		ADD_INTERNAL_CALL(SetFloat);
		ADD_INTERNAL_CALL(GetCurrAnimName);
		ADD_INTERNAL_CALL(IsCurrAnimFin);
		ADD_INTERNAL_CALL(GetCurrAnimTime);
		ADD_INTERNAL_CALL(GetCurrAnimFPS);
	}

#pragma endregion

}
