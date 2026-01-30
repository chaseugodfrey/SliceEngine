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
#include "../Configuration/ProjectSettingsManager.h"
#include "../Input/ActionMapping.h"
#include "Graphics/RenderManager.h"
#include "../Systems/LayerManager.h"

namespace SliceEngine
{
	std::string MonoToString(MonoString *monoStr)
	{
		if (!monoStr) return "";

		char *utf8Str = mono_string_to_utf8(monoStr);
		if (!utf8Str) return "";

		std::string result(utf8Str);
		mono_free(utf8Str);

		return result;
	}

	static std::unordered_map<MonoType *, std::function<bool(GameObject)>> mGameObjectHasComponentFuncs;

	// Define to make it easier to add internal function calls
#define ADD_INTERNAL_CALL(Name) mono_add_internal_call("SliceEngine.FunctionCalls::" #Name, Name)

#pragma region DEBUGGING FUNCTIONS

	static void Debug_Console(MonoArray* monoStrArray, MonoString* monoMsg, int level)
	{

		if (monoStrArray == nullptr)
		{
			SLICE_LOG_ERROR("Received null MonoArray (string[]) from C#.");
			return;
		}

		uintptr_t length = mono_array_length(monoStrArray);

		std::vector<std::string> callStack;

		for (uintptr_t i = 0; i < length; ++i)
		{
			// Use MonoObject* or MonoString* to retrieve the string reference
			// String is a reference type, so mono_array_get returns the object reference.
			MonoString* monoStr = (MonoString*)mono_array_get(monoStrArray, MonoObject*, i);

			callStack.push_back(MonoToString(monoStr));
		}

		Logger::ConsoleMessage consoleMsg{ std::move(callStack), MonoToString(monoMsg) };

		SLICE_LOG_CONSOLE(level, std::move(consoleMsg));
	}

#pragma endregion

#pragma region TRANSFORM FUNCTIONS

	static void Transform_GetPosition(unsigned int entity, glm::vec3 *outPosition)
	{
		//SLICE_LOG("Getting position from C++ for entity: {}", entity);
		// note if we add UI objects
		// this might need to be modified to check which transform it has
		auto &transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();

		*outPosition = transform.position;
	}

	static void Transform_GetWorldPosition(unsigned int entity, glm::vec3* outPosition)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();

		*outPosition = transform.GetWorldPosition();
	}

	static void Transform_SetPosition(unsigned int entity, glm::vec3 *position)
	{
		//SLICE_LOG("Setting position from C++ for entity: {}", entity);
		auto &transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		transform.position = *position;
	}

	static void Transform_GetScale(unsigned int entity, glm::vec3 *outScale)
	{
		auto &transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		*outScale = transform.scale;
	}

	static void Transform_SetScale(unsigned int entity, glm::vec3 *scale)
	{
		auto &transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		transform.scale = *scale;
	}

	static void Transform_GetRotation(unsigned int entity, glm::vec3 *outRotation)
	{
		auto &transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		glm::vec3 euler = SliceEngine::QuatToVec3(transform.rotation);
		*outRotation = euler;
		// leaving blank for now cause i think i ahve to return as euler not quaternion
	}

	static void Transform_SetRotation(unsigned int entity, glm::vec3 *rotation)
	{
		auto &transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		transform.rotation = SliceEngine::Vec3ToQuat(*rotation);
		transform.eulerAnglesHint = *rotation;
		// leaving blank for now cause i think i ahve to return as euler not quaternion
	}

	static void Transform_GetRotationQuat(unsigned int entity, glm::quat* outRotation)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		*outRotation = transform.rotation; // REAL QUATERNION
	}

	static void Transform_GetWorldRotationQuat(unsigned int entity, glm::quat* outRotation)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		*outRotation = transform.GetWorldRotation(); // REAL QUATERNION	
	}

	static void Transform_SetRotationQuat(unsigned int entity, const glm::quat* rotation)
	{
		auto& transform = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<Transform>();
		transform.rotation = glm::normalize(*rotation);

		// update Euler hint only for inspector UI
		transform.eulerAnglesHint = SliceEngine::QuatToVec3(transform.rotation);
	}


	static Transform* GetTransformComponent(unsigned int entity)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<Transform>())
		{
			return &go.GetComponent<Transform>();
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Transform component.", entity);
		return nullptr;
	}

#pragma endregion

#pragma region INPUT & ACTIONMAPPING FUNCTIONS

	static bool Input_IsKeyPressed(Keys keyCode)
	{
		return Core::GetInstance()->GetInputSystem()->IsKeyPressed(keyCode);
	}

	static bool Input_IsKeyDown(Keys keyCode)
	{
		return Core::GetInstance()->GetInputSystem()->IsKeyDown(keyCode);
	}

	static bool Input_IsKeyReleased(Keys keyCode)
	{
		return Core::GetInstance()->GetInputSystem()->IsKeyReleased(keyCode);
	}

	static bool Input_IsMousePressed(MouseButtons button)
	{
		return Core::GetInstance()->GetInputSystem()->IsMousePressed(button);
	}

	static bool Input_IsMouseDown(MouseButtons button)
	{
		return Core::GetInstance()->GetInputSystem()->IsMouseDown(button);
	}

	static bool Input_IsMouseReleased(MouseButtons button)
	{
		return Core::GetInstance()->GetInputSystem()->IsMouseReleased(button);
	}

	static void Input_GetMousePosition(glm::vec2* outPosition)
	{
		*outPosition = Core::GetInstance()->GetInputSystem()->GetMousePosition();
	}

	static void Input_GetMouseDelta(glm::vec2* outDelta)
	{
		*outDelta = Core::GetInstance()->GetInputSystem()->GetMouseDelta();
	}

	static int Input_GetCursorState()
	{
		return static_cast<int>(Core::GetInstance()->GetInputSystem()->GetCursorState());
	}

	static void Input_SetCursorState(int lockState)
	{
		Core::GetInstance()->GetInputSystem()->SetCursorState(static_cast<CursorState>(lockState));
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
#pragma endregion

#pragma region PARTICLE SYSTEM FUNCTIONS

	static void ParticleSystem_GetDuration(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().duration;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);		
	}
	static void ParticleSystem_SetDuration(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().duration = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetRepeating(unsigned int entity, bool* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().isRepeating;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_SetRepeating(unsigned int entity, bool* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().isRepeating = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetLocalSpace(unsigned int entity, bool* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().isLocalSpace;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetLocalSpace(unsigned int entity, bool* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().isLocalSpace = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetDestroyOnExpire(unsigned int entity, bool* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().destroyOnExpire;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetDestroyOnExpire(unsigned int entity, bool* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().destroyOnExpire = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}


	static void ParticleSystem_GetMaxParticles(unsigned int entity, unsigned long long* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().maxParticles;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetMaxParticles(unsigned int entity, unsigned long long* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().maxParticles = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetGForce(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().gForce;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetGForce(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().gForce = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetEmissionRate(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().emissionRate;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetEmissionRate(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().emissionRate = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetConeArc(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().coneArc;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetConeArc(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().coneArc = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_GetConeRadius(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().coneRadius;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetConeRadius(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().coneRadius = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}


	static void ParticleSystem_GetSphereRadius(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().sphereRadius;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetSphereRadius(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().sphereRadius = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetScaleValueType(unsigned int entity, ParticleSystem::ValueType* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().scaleType;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetScaleValueType(unsigned int entity, ParticleSystem::ValueType* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().scaleType = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetScale(unsigned int entity, glm::vec3* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().scale;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetScale(unsigned int entity, glm::vec3* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().scale = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetScaleMin(unsigned int entity, glm::vec3* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().minRandomScale;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetScaleMin(unsigned int entity, glm::vec3* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().minRandomScale = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetScaleMax(unsigned int entity, glm::vec3* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().maxRandomScale;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetScaleMax(unsigned int entity, glm::vec3* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().maxRandomScale = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetLifetimeValueType(unsigned int entity, ParticleSystem::ValueType* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().initialLifetimeType;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetLifetimeValueType(unsigned int entity, ParticleSystem::ValueType* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().initialLifetimeType = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetLifetime(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().lifetime;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetLifetime(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().lifetime = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetMinLifetime(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().minParticleLifetime;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetMinLifetime(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().minParticleLifetime = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetMaxLifetime(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().maxParticleLifetime;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetMaxLifetime(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().maxParticleLifetime = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetRotationValueType(unsigned int entity, ParticleSystem::ValueType* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().initialRotationType;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetRotationValueType(unsigned int entity, ParticleSystem::ValueType* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().initialRotationType = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetRotation(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().rotation;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetRotation(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().rotation = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetRotationMin(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().minRandomRotation;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetRotationMin(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().minRandomRotation = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetRotationMax(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().maxRandomRotation;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetRotationMax(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().maxRandomRotation = *value;
			return;
		}
	
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	// 3D Rotation
	static void ParticleSystem_GetIsRotation3D(unsigned int entity, bool* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().isRotation3D;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetIsRotation3D(unsigned int entity, bool* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().isRotation3D = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_GetRotation3DHint(unsigned int entity, glm::vec3* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().rotation3DHint;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_SetRotation3DHint(unsigned int entity, glm::vec3* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().rotation3DHint = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_GetMinRotation3DHint(unsigned int entity, glm::vec3* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().minRotation3DHint;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_SetMinRotation3DHint(unsigned int entity, glm::vec3* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().minRotation3DHint = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_GetMaxRotation3DHint(unsigned int entity, glm::vec3* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().maxRotation3DHint;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_SetMaxRotation3DHint(unsigned int entity, glm::vec3* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().maxRotation3DHint = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetSpawnPosValueType(unsigned int entity, ParticleSystem::ValueType* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().posValueType;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetSpawnPosValueType(unsigned int entity, ParticleSystem::ValueType* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().posValueType = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetSpawnPos(unsigned int entity, glm::vec3* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().spawnPos;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetSpawnPos(unsigned int entity, glm::vec3* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().spawnPos = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetSpawnPosMin(unsigned int entity, glm::vec3* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().minRandomSpawnPos;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetSpawnPosMin(unsigned int entity, glm::vec3* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().minRandomSpawnPos = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetSpawnPosMax(unsigned int entity, glm::vec3* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().maxRandomSpawnPos;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetSpawnPosMax(unsigned int entity, glm::vec3* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().maxRandomSpawnPos = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetColourValueType(unsigned int entity, ParticleSystem::ValueType* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().colourValueType;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetColourValueType(unsigned int entity, ParticleSystem::ValueType* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().colourValueType = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetColour(unsigned int entity, glm::vec4* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().colour;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetColour(unsigned int entity, glm::vec4* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().colour = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetColourMin(unsigned int entity, glm::vec4* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().minRandomColour;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetColourMin(unsigned int entity, glm::vec4* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().minRandomColour = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetColourMax(unsigned int entity, glm::vec4* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().maxRandomColour;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetColourMax(unsigned int entity, glm::vec4* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().maxRandomColour = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetSpeedValueType(unsigned int entity, ParticleSystem::ValueType* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().speedValueType;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_SetSpeedValueType(unsigned int entity, ParticleSystem::ValueType* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().speedValueType = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetSpeed(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().speed;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_SetSpeed(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().speed = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetSpeedMin(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().minRandomSpeed;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_SetSpeedMin(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().minRandomSpeed = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetSpeedMax(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().maxRandomSpeed;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}
	static void ParticleSystem_SetSpeedMax(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().maxRandomSpeed = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetColourOverLifetime(unsigned int entity, bool* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().colourOverLifetime;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetColourOverLifetime(unsigned int entity, bool* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().colourOverLifetime = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetCollision(unsigned int entity, bool* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().hasCollision;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetCollision(unsigned int entity, bool* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().hasCollision = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetTextureID(unsigned int entity, unsigned int* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().GetTextureID();
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetTextureID(unsigned int entity, unsigned int* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().textureGUID = GUID(*value);
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetShapeType(unsigned int entity, ParticleSystem::ShapeType* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().shapeType;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetShapeType(unsigned int entity, ParticleSystem::ShapeType* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().shapeType = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_IsExpired(unsigned int entity, bool* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().expired;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_GetSystemTimer(unsigned int entity, float* out)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			*out = go.GetComponent<ParticleSystem>().systemTimer;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

	static void ParticleSystem_SetSystemTimer(unsigned int entity, float* value)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (go.IsValid() && go.HasComponent<ParticleSystem>())
		{
			go.GetComponent<ParticleSystem>().systemTimer = *value;
			return;
		}
		SLICE_LOG_ERROR("Scripting: Entity %u has no Particle System component.", entity);
	}

#pragma endregion

#pragma region CONSOLE LOGGING FUNCTIONS

	static void Log(MonoString *string)
	{
		std::string cStrName = MonoToString(string);
		SLICE_LOG_DEBUG(cStrName);
		//CM_CORE_INFO(cStrName);
		//mono_free(cStr);
	}

	static void LogWarn(MonoString *string)
	{
		std::string cStrName = MonoToString(string);
		SLICE_LOG_WARNING(cStrName);
		//CM_CORE_WARN(cStrName);
		//mono_free(cStr);
	}

	static void LogError(MonoString *string)
	{
		std::string cStrName = MonoToString(string);
		SLICE_LOG_ERROR(cStrName);
		//CM_CORE_ERROR(cStrName);
		//mono_free(cStr);
	}
#pragma endregion

#pragma region RIGIDBODY FUNCTIONS

	static void RigidBody_GetVelocity(unsigned int entity, glm::vec3 *outPosition)
	{
		//SLICE_LOG("Getting velocity from C++ for entity: {}", entity);

		*outPosition = Core::GetInstance()->GetSystem<PhysicsSystem>().GetLinearVelocity((Entity)entity);
	}

	static void RigidBody_SetVelocity(unsigned int entity, glm::vec3 *position)
	{
		//SLICE_LOG("Setting velocity from C++ for entity: {}", entity);

		JPH::Vec3 vel(position->x, position->y, position->z);

		Core::GetInstance()->GetSystem<PhysicsSystem>().SetLinearVelocity((Entity)entity, vel);
	}

	static void RigidBody_AddForce(unsigned int entity, glm::vec3 *force, int mode)
	{
		JPH::Vec3 f(force->x, force->y, force->z);

		switch (mode)
		{
		case 0: // Force
			Core::GetInstance()->GetSystem<PhysicsSystem>().AddForceToEntity((Entity)entity, f);
			break;
		case 1: // Impulse
			Core::GetInstance()->GetSystem<PhysicsSystem>().AddImpulseToEntity((Entity)entity, f);
			break;
		case 2: // Velocity Change
			Core::GetInstance()->GetSystem<PhysicsSystem>().AddVelocityChangeToEntity((Entity)entity, f);
			break;
		case 3: // Acceleration
			Core::GetInstance()->GetSystem<PhysicsSystem>().AddAccelerationToEntity((Entity)entity, f);
			break;
		default:
			SLICE_LOG_ERROR("if u somehow made it come here i'll be dissapointed");
			break;
		}
	}

	static float RigidBody_GetGravityFactor(unsigned int entity)
	{	
		return Core::GetInstance()->GetSystem<PhysicsSystem>().GetGravityFactor((Entity)entity);
	}

	static void RigidBody_SetGravityFactor(unsigned int entity, float factor)
	{
		Core::GetInstance()->GetSystem<PhysicsSystem>().SetGravityFactor((Entity)entity, factor);
	}

	static bool RigidBody_IsGravityOff(unsigned int entity)
	{
		return Core::GetInstance()->GetSystem<PhysicsSystem>().IsGravityOff((Entity)entity);
	}

	static void RigidBody_OffGravity(unsigned int entity, bool condition)
	{
		Core::GetInstance()->GetSystem<PhysicsSystem>().OffGravity(Entity(entity), condition);
	}

#pragma endregion

#pragma region COLLIDERSHAPE FUNCTIONS
	
	static bool ColliderShape_IsEnabled(unsigned int entity)
	{
		GameObject go = FactoryInstance.GetGOByEntity((Entity)entity);
		if (!go.HasComponent<ColliderShape>())
		{
			SLICE_LOG_ERROR("Lol skill issue", entity);
			return false;
		}

		auto& collider = go.GetComponent<ColliderShape>();
		return collider.componentEnabled;
	}

	static void ColliderShape_SetEnabled(unsigned int entity, bool enabled)
	{
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
		GameObject go = FactoryInstance.GetGOByEntity((Entity)entity);

		if (go.HasComponent<ColliderShape>())
		{
			Entity entity = go.GetEntity();

			//using patch so that the event system can pick up the change
			reg.patch<SliceEngine::ColliderShape>(entity, [&](auto& collider)
				{
					collider.componentEnabled = enabled;
				});
		}
		else
		{
			SLICE_LOG_ERROR("Lol skill issue", entity);
		}
				
	}

#pragma endregion

#pragma region LAYERMASK FUNCTIONS

	static uint32_t LayerMask_GetMask(MonoString* string)
	{
		std::string name = MonoToString(string);

		return Core::GetInstance()->GetLayerManager()->GetMask(name);
	}

	static MonoString* LayerMask_LayerToName(uint32_t layer)
	{
		if(layer >= MAX_LAYERS)
		{
			SLICE_LOG_ERROR("Scripting: Layer {} is out of bounds.", layer);
			std::string errorLayer = "If your layer is this very long string u did something wrong or I did(Please contact Aloysius for assistance)";
			return mono_string_new(mono_domain_get(), errorLayer.c_str());
		}

		std::string layerName = Core::GetInstance()->GetLayerManager()->GetLayerName(layer);
		return mono_string_new(mono_domain_get(), layerName.c_str());
	}

	static uint32_t LayerMask_NameToLayer(MonoString* string)
	{
		std::string name = MonoToString(string);
		uint32_t layer = Core::GetInstance()->GetLayerManager()->GetLayer(name);

		if(layer >= INVALID_LAYER)
		{
			SLICE_LOG_ERROR("Scripting: Layer '{}' does not exist.", name);
			return INVALID_LAYER;
		}

		return layer;
	}

	

#pragma endregion

#pragma region RAYCASTING FUCNTIONS

	static bool Physics_Raycast(glm::vec3* origin, glm::vec3* direction, uint32_t*  bodyHitID, uint32_t* mask)
	{
		return Core::GetInstance()->GetSystem<PhysicsSystem>().PSystemRayCast(*origin, *direction, *bodyHitID, *mask);
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

	//Return a filepath
	static MonoString* Audio_GetSoundName(unsigned int entity)
	{
		//SLICE_LOG("Getting audio name from C++ for entity: {}", entity);

		auto &audio = FactoryInstance.GetGOByEntity((Entity)entity).GetComponent<AudioSource>();

		std::string test;

		for (auto pair : Core::GetInstance()->GetResourceManager()->mFileNameToGUID)
		{
			if (audio.soundGUID == pair.second)
			{
				test = pair.first;
				break;
			}
		}

		if (test == "")
		{
			return nullptr;
		}

		return mono_string_new(mono_domain_get(), test.c_str());

	}

	static void Audio_Play(unsigned int entity)
	{
		auto* audioComp = GetAudioComponent(entity);
		auto* transformComp = GetTransformComponent(entity);
		
		if (audioComp && !Core::GetInstance()->GetAudioManager()->IsChannelPlaying(audioComp->channel))
		{
			
			audioComp->channel = Core::GetInstance()->GetAudioManager()->PlaySound(*(audioComp), transformComp->position, glm::vec3(0.f));
		}
	}

	static void Audio_PlaySFX(MonoString* string, glm::vec3 position)
	{
		std::string key = MonoToString(string);
		if (position == glm::vec3(0.f))
		{
			Core::GetInstance()->GetProjectSettingsManager()->GetSettings<AudioSettings>()->PlaySFX(key);
		}
		else
		{
			Core::GetInstance()->GetProjectSettingsManager()->GetSettings<AudioSettings>()->PlaySFX(key, position);
		}
	}

	static void Audio_Stop(unsigned int entity)
	{
		if (auto *audioComp = GetAudioComponent(entity))
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
		if (auto *audioComp = GetAudioComponent(entity))
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
		if (auto *audioComp = GetAudioComponent(entity))
		{
			audioComp->isPaused = paused;
			// SoundSystem::UpdateChannelFromComponent will sync this
		}
	}

	static bool Audio_GetPaused(unsigned int entity)
	{
		if (auto *audioComp = GetAudioComponent(entity))
		{
			return audioComp->isPaused;
		}
		return false;
	}

	static void Audio_SetLoop(unsigned int entity, bool loop)
	{
		if (auto *audioComp = GetAudioComponent(entity))
		{
			audioComp->isLoop = loop;
		}
	}

	static bool Audio_GetLoop(unsigned int entity)
	{
		if (auto *audioComp = GetAudioComponent(entity))
		{
			return audioComp->isLoop;
		}
		return false;
	}

	static void Audio_SetVolume(unsigned int entity, float volume)
	{
		if (auto *audioComp = GetAudioComponent(entity))
		{
			audioComp->currentVolume = volume;
		}
	}

	static float Audio_GetVolume(unsigned int entity)
	{
		if (auto *audioComp = GetAudioComponent(entity))
		{
			return audioComp->currentVolume;
		}
		return 0.0f;
	}

	static void Audio_SetPitch(unsigned int entity, float pitch)
	{
		if (auto *audioComp = GetAudioComponent(entity))
		{
			audioComp->pitch = pitch;
		}
	}

	static float Audio_GetPitch(unsigned int entity)
	{
		if (auto *audioComp = GetAudioComponent(entity))
		{
			return audioComp->pitch;
		}
		return 1.0f;
	}

	static void Audio_SetSpatialBlend(unsigned int entity, float blend)
	{
		if (auto *audioComp = GetAudioComponent(entity))
		{
			audioComp->spatialBlend = blend;
		}
	}

	static float Audio_GetSpatialBlend(unsigned int entity)
	{
		if (auto *audioComp = GetAudioComponent(entity))
		{
			return audioComp->spatialBlend;
		}
		return 0.0f;
	}

	static void Audio_SetPan(unsigned int entity, float pan)
	{
		if (auto *audioComp = GetAudioComponent(entity)) audioComp->stereoPan = pan;
	}

	static float Audio_GetPan(unsigned int entity)
	{
		if (auto *audioComp = GetAudioComponent(entity)) return audioComp->stereoPan;
		return 0.0f;
	}

	static void Audio_SetMute(unsigned int entity, bool mute)
	{
		if (auto *audioComp = GetAudioComponent(entity)) audioComp->isMute = mute;
	}

	static bool Audio_GetMute(unsigned int entity)
	{
		if (auto *audioComp = GetAudioComponent(entity)) return audioComp->isMute;
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

	static bool HasScriptInstance(unsigned int entityID, MonoString *baseName)
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
	static bool Entity_HasComponent(unsigned int entityID, MonoReflectionType *componentType)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		MonoType *monoType = mono_reflection_type_get_type(componentType);

		if (mGameObjectHasComponentFuncs.count(monoType) <= 0)
		{
			// component not registered
			SLICE_LOG_ERROR("Component Not Registered");
			assert("Component not registered");
		}

		return mGameObjectHasComponentFuncs[monoType](GO);

	}

	static MonoArray *Entity_FindEntitiesWithTag(MonoString *tag)
	{
		std::string cStrName = MonoToString(tag);

		std::vector<Entity> entityIDs = FactoryInstance.GetEntitiesWithTag(cStrName);


		MonoDomain *domain = mono_domain_get();
		MonoArray *monoArray = mono_array_new(domain, mono_get_uint32_class(), entityIDs.size());

		for (size_t i = 0; i < entityIDs.size(); ++i)
		{
			mono_array_set(monoArray, uint32_t, i, static_cast<uint32_t>(entityIDs[i]));
		}

		return monoArray;
	}

	static unsigned int CreateNewGameObject(MonoString *prefabName)
	{
		std::string cStrName = MonoToString(prefabName);

		auto &prefabSys = Core::GetInstance()->GetSystem<PrefabSystem>();
		auto rm = Core::GetInstance()->GetResourceManager();
		auto it = rm->mFileNameToGUID.find(cStrName);
		if (it != rm->mFileNameToGUID.end())
		{
			GameObject newGO = prefabSys.CreatePrefab((GUID)it->second);
			if(cStrName == "EnemyTest")
			{
				SLICE_LOG("Creating Enemy with ID " + static_cast<unsigned int>(newGO.GetEntity()));
			//	std::cout << "Creating enemy with ID<" << static_cast<unsigned int>(newGO.GetEntity()) << ">\n";
			}
			return(unsigned int)newGO.GetEntity();
		}
		//mono_free(cStrName);
		SLICE_LOG_ERROR("Unable to create prefab from: " + cStrName);

		return entt::null;
	}

	static unsigned int CloneGO(MonoString *GoName)
	{
		std::string cStrName = MonoToString(GoName);
		auto GO = FactoryInstance.GetGOByName(cStrName);

		auto newGO = FactoryInstance.CloneGO(GO);
		return (unsigned int)newGO.GetEntity();
	}

	static uint32_t Entity_FindEntityWithName(MonoString *name)
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

	static MonoString *Entity_GetTag(unsigned int entityID)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (go.HasComponent<SliceEntity>())
		{
			return mono_string_new(mono_domain_get(), go.GetComponent<SliceEntity>().mTag.c_str());
		}
	}

	static void Entity_SetTag(unsigned int entityID, MonoString *tag)
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

	static uint32_t Entity_FindEntityWithID(unsigned int entityID)
	{
		auto go = FactoryInstance.GetGOByEntity((Entity)entityID);

		if (go.IsValid())
		{
			return static_cast<uint32_t>(go.GetEntity());
		}
		else
		{
			return 0;
		}
	}

	static bool Entity_IsActive(unsigned int entity)
	{
		if (RegistryInstance.any_of<InactiveEntity>(entt::entity(entity)))
		{
			return false;
		}

		return true;
	}

	static void Entity_SetActive(unsigned int entity, bool active)
	{
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
		entt::entity e = entt::entity(entity);
		if (active)
		{
			if (reg.any_of<InactiveEntity>(e))
			{
				reg.remove<InactiveEntity>(e);
			}
		}
		else
		{
			if (!reg.any_of<InactiveEntity>(e))
			{
				reg.emplace<InactiveEntity>(e);
			}
		}
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

	static void SetBool(unsigned int entityID, MonoString *string, bool val)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			std::string cStrName = MonoToString(string);

			GO.GetComponent<Animator>().stateMachine.SetBool(cStrName, val);
		}

	}

	static void SetInt(unsigned int entityID, MonoString *string, int val)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			std::string cStrName = MonoToString(string);
			GO.GetComponent<Animator>().stateMachine.SetInt(cStrName, val);

		}
	}

	static void SetFloat(unsigned int entityID, MonoString *string, float val)
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

		return 0.0f;
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

	static bool SafeToChange(unsigned int entityID, MonoString* string)
	{
		auto GO = FactoryInstance.GetGOByEntity((Entity)entityID);
		if (GO.HasComponent<Animator>())
		{
			std::string cStrName = MonoToString(string);

			return GO.GetComponent<Animator>().stateMachine.SafeToChange(cStrName);
		}
		return false;
	}

#pragma endregion
	
#pragma region SCENE FUNCTIONS
	static void QuitGame()
	{
		EventManager::GetInstance()->Publish<OnGameStopEvent>();
	}

#pragma endregion

#pragma region CAMERA FUNCTIONS

	static void Camera_SetMainCamera(unsigned int entityID)
	{
		auto* rm = Core::GetInstance()->GetRenderManager();
		rm->SetMainGameCamera((Entity)entityID);
	}

#pragma endregion

#pragma region NAVIGATION FUNCTIONS


	// Helper to get the agent component
	static NavAgent *GetNavAgent(uint32_t entityID)
	{
		auto *core = SliceEngine::Core::GetInstance();

		entt::registry &registry = core->GetRegistry();

		entt::entity e = (entt::entity)entityID;
		if (!registry.valid(e))
		{
			return nullptr;
		}

		return registry.try_get<NavAgent>(e);
	}

	static void NavAgent_ComponentState(uint32_t entityID, bool componentState)
	{
		NavAgent* agent = GetNavAgent(entityID);

		if (agent)
		{
			agent->componentEnabled = componentState;
		}
	}

	static void NavAgent_SetDestination(uint32_t entityID, glm::vec3 *target)
	{
		NavAgent *agent = GetNavAgent(entityID);
		if (agent)
		{
			agent->target = *target;
			agent->hasNewTarget = true; 
		}
	}

	static void NavAgent_Stop(uint32_t entityID)
	{
		NavAgent *agent = GetNavAgent(entityID);
		if (agent)
		{
			agent->currentPath.clear();
			agent->currentPathIndex = 0;
			agent->hasNewTarget = false;
		}
	}

	static float NavAgent_GetSpeed(uint32_t entityID)
	{
		NavAgent *agent = GetNavAgent(entityID);
		return agent ? agent->speed : 0.0f;
	}

	static void NavAgent_SetSpeed(uint32_t entityID, float speed)
	{
		NavAgent *agent = GetNavAgent(entityID);
		if (agent) agent->speed = speed;
	}

	static bool NavAgent_HasPath(uint32_t entityID)
	{
		NavAgent *agent = GetNavAgent(entityID);
		// Returns true if path is not empty
		return agent && !agent->currentPath.empty();

	}
	static bool NavAgent_GetComponentEnabled(uint32_t entityID)
	{
		NavAgent *agent = GetNavAgent(entityID);
		return agent->componentEnabled;
	}
	static void NavAgent_SetComponentEnabled(uint32_t entityID, bool isEnabled)
	{
		NavAgent *agent = GetNavAgent(entityID);
		if (agent) agent->componentEnabled = isEnabled;
	}
#pragma endregion

#pragma region SpriteRenderer FUNCTIONS
	static void SpriteRenderer_SetEnabled(uint32_t entityID, bool enabled)
	{
		GameObject GO = FactoryInstance.GetGOByEntity((Entity)entityID);

		if (GO.HasComponent<SpriteRenderer>())
		{
			auto& spriteRenderer = GO.GetComponent<SpriteRenderer>();
			spriteRenderer.componentEnabled = enabled;
		}
	}

#pragma endregion

#pragma region FontRenderer FUNCTIONS
	static void FonteRenderer_SetEnabled(uint32_t entityID, bool enabled)
	{
		GameObject GO = FactoryInstance.GetGOByEntity((Entity)entityID);

		if (GO.HasComponent<FontRenderer>())
		{
			auto& fontRenderer = GO.GetComponent<FontRenderer>();
			fontRenderer.componentEnabled = enabled;
		}
	}

#pragma endregion

#pragma region UI FUNCTIONS

	static float Slider_GetValue(uint32_t entityID)
	{
		auto* core = SliceEngine::Core::GetInstance();

		entt::registry& registry = core->GetRegistry();

		entt::entity e = (entt::entity)entityID;
		if (!registry.valid(e) || !registry.any_of<Slider>(e))
		{
			return 0.f;
		}

		auto const& slider = registry.get<Slider>(e);
		return slider.GetValue();
	}

	static void Slider_SetValue(uint32_t entityID, float value)
	{
		auto* core = SliceEngine::Core::GetInstance();

		entt::registry& registry = core->GetRegistry();

		entt::entity e = (entt::entity)entityID;
		if (!registry.valid(e) || !registry.any_of<Slider>(e))
		{
			return;
		}

		auto& slider = registry.get<Slider>(e);
		slider.SetValue(value, e);
	}

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
		mGameObjectHasComponentFuncs[monoType] = [](GameObject go) { return go.HasComponent<T>(); };
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
		RegisterComponent<ColliderShape>();
		RegisterComponent<RigidBody>();
		RegisterComponent<NavAgent>();
		RegisterComponent<Slider>();
		RegisterComponent<AudioSource>();
		RegisterComponent<SpriteRenderer>();
		RegisterComponent<FontRenderer>();
		//RegisterComponent<Animation>();
		//RegisterComponent<StateMachine>();
		//RegisterComponent<Renderer>();
		//RegisterComponent<TextRenderer>();
	}

	/// <summary>
	/// Register functions to C# side by using the internal mono call
	/// </summary>
	void ScriptFunctions::RegisterFunctions()
	{
		ADD_INTERNAL_CALL(Debug_Console);
		ADD_INTERNAL_CALL(QuitGame);

		//Camera
		ADD_INTERNAL_CALL(Camera_SetMainCamera);

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
		ADD_INTERNAL_CALL(Entity_FindEntityWithID);
		ADD_INTERNAL_CALL(SpriteRenderer_SetEnabled);
		ADD_INTERNAL_CALL(Entity_IsActive);
		ADD_INTERNAL_CALL(Entity_SetActive);

		// Transforms
		ADD_INTERNAL_CALL(Transform_GetPosition);
		ADD_INTERNAL_CALL(Transform_GetWorldPosition);
		ADD_INTERNAL_CALL(Transform_SetPosition);
		ADD_INTERNAL_CALL(Transform_GetScale);
		ADD_INTERNAL_CALL(Transform_SetScale);
		ADD_INTERNAL_CALL(Transform_GetRotation);
		ADD_INTERNAL_CALL(Transform_SetRotation);
		ADD_INTERNAL_CALL(Transform_GetRotationQuat);
		ADD_INTERNAL_CALL(Transform_GetWorldRotationQuat);
		ADD_INTERNAL_CALL(Transform_SetRotationQuat);

		// Key input & action mapping functions, idrk whhat exact functions the designers want so i'll just put down whateva
		ADD_INTERNAL_CALL(Input_IsKeyPressed);
		ADD_INTERNAL_CALL(Input_IsKeyDown);
		ADD_INTERNAL_CALL(Input_IsKeyReleased);
		ADD_INTERNAL_CALL(Input_IsMousePressed);
		ADD_INTERNAL_CALL(Input_IsMouseDown);
		ADD_INTERNAL_CALL(Input_IsMouseReleased);
		ADD_INTERNAL_CALL(Input_GetCursorState);
		ADD_INTERNAL_CALL(Input_SetCursorState);
		ADD_INTERNAL_CALL(Input_GetMousePosition);
		ADD_INTERNAL_CALL(Input_GetMouseDelta);
		ADD_INTERNAL_CALL(AM_EnableMap);
		ADD_INTERNAL_CALL(AM_PerformedThisFrame);
		ADD_INTERNAL_CALL(AM_GetValue2D);
		ADD_INTERNAL_CALL(AM_GetValue1D);

		// Particle system
		ADD_INTERNAL_CALL(ParticleSystem_GetDuration);
		ADD_INTERNAL_CALL(ParticleSystem_SetDuration);
		ADD_INTERNAL_CALL(ParticleSystem_GetRepeating);
		ADD_INTERNAL_CALL(ParticleSystem_SetRepeating);
		ADD_INTERNAL_CALL(ParticleSystem_GetLocalSpace);
		ADD_INTERNAL_CALL(ParticleSystem_SetLocalSpace);

		ADD_INTERNAL_CALL(ParticleSystem_GetDestroyOnExpire);
		ADD_INTERNAL_CALL(ParticleSystem_SetDestroyOnExpire);

		ADD_INTERNAL_CALL(ParticleSystem_GetMaxParticles);
		ADD_INTERNAL_CALL(ParticleSystem_SetMaxParticles);

		ADD_INTERNAL_CALL(ParticleSystem_GetGForce);
		ADD_INTERNAL_CALL(ParticleSystem_SetGForce);

		ADD_INTERNAL_CALL(ParticleSystem_GetEmissionRate);
		ADD_INTERNAL_CALL(ParticleSystem_SetEmissionRate);

		ADD_INTERNAL_CALL(ParticleSystem_GetShapeType);
		ADD_INTERNAL_CALL(ParticleSystem_SetShapeType);

		ADD_INTERNAL_CALL(ParticleSystem_GetConeArc);
		ADD_INTERNAL_CALL(ParticleSystem_SetConeArc);
		ADD_INTERNAL_CALL(ParticleSystem_GetConeRadius);
		ADD_INTERNAL_CALL(ParticleSystem_SetConeRadius);
		ADD_INTERNAL_CALL(ParticleSystem_GetSphereRadius);
		ADD_INTERNAL_CALL(ParticleSystem_SetSphereRadius);

		ADD_INTERNAL_CALL(ParticleSystem_GetScaleValueType);
		ADD_INTERNAL_CALL(ParticleSystem_SetScaleValueType);
		ADD_INTERNAL_CALL(ParticleSystem_GetScale);
		ADD_INTERNAL_CALL(ParticleSystem_SetScale);
		ADD_INTERNAL_CALL(ParticleSystem_GetScaleMin);
		ADD_INTERNAL_CALL(ParticleSystem_SetScaleMin);
		ADD_INTERNAL_CALL(ParticleSystem_GetScaleMax);
		ADD_INTERNAL_CALL(ParticleSystem_SetScaleMax);

		ADD_INTERNAL_CALL(ParticleSystem_GetLifetimeValueType);
		ADD_INTERNAL_CALL(ParticleSystem_SetLifetimeValueType);
		ADD_INTERNAL_CALL(ParticleSystem_GetLifetime);
		ADD_INTERNAL_CALL(ParticleSystem_SetLifetime);
		ADD_INTERNAL_CALL(ParticleSystem_GetMinLifetime);
		ADD_INTERNAL_CALL(ParticleSystem_SetMinLifetime);
		ADD_INTERNAL_CALL(ParticleSystem_GetMaxLifetime);
		ADD_INTERNAL_CALL(ParticleSystem_SetMaxLifetime);

		ADD_INTERNAL_CALL(ParticleSystem_GetRotationValueType);
		ADD_INTERNAL_CALL(ParticleSystem_SetRotationValueType);
		ADD_INTERNAL_CALL(ParticleSystem_GetRotation);
		ADD_INTERNAL_CALL(ParticleSystem_SetRotation);
		ADD_INTERNAL_CALL(ParticleSystem_GetRotationMin);
		ADD_INTERNAL_CALL(ParticleSystem_SetRotationMin);
		ADD_INTERNAL_CALL(ParticleSystem_GetRotationMax);
		ADD_INTERNAL_CALL(ParticleSystem_SetRotationMax);
		ADD_INTERNAL_CALL(ParticleSystem_GetIsRotation3D);
		ADD_INTERNAL_CALL(ParticleSystem_SetIsRotation3D);
		ADD_INTERNAL_CALL(ParticleSystem_GetRotation3DHint);
		ADD_INTERNAL_CALL(ParticleSystem_SetRotation3DHint);
		ADD_INTERNAL_CALL(ParticleSystem_GetMinRotation3DHint);
		ADD_INTERNAL_CALL(ParticleSystem_SetMinRotation3DHint);
		ADD_INTERNAL_CALL(ParticleSystem_GetMaxRotation3DHint);
		ADD_INTERNAL_CALL(ParticleSystem_SetMaxRotation3DHint);

		ADD_INTERNAL_CALL(ParticleSystem_GetSpawnPosValueType);
		ADD_INTERNAL_CALL(ParticleSystem_SetSpawnPosValueType);
		ADD_INTERNAL_CALL(ParticleSystem_GetSpawnPos);
		ADD_INTERNAL_CALL(ParticleSystem_SetSpawnPos);
		ADD_INTERNAL_CALL(ParticleSystem_GetSpawnPosMin);
		ADD_INTERNAL_CALL(ParticleSystem_SetSpawnPosMin);
		ADD_INTERNAL_CALL(ParticleSystem_GetSpawnPosMax);
		ADD_INTERNAL_CALL(ParticleSystem_SetSpawnPosMax);

		ADD_INTERNAL_CALL(ParticleSystem_GetColourValueType);
		ADD_INTERNAL_CALL(ParticleSystem_SetColourValueType);
		ADD_INTERNAL_CALL(ParticleSystem_GetColour);
		ADD_INTERNAL_CALL(ParticleSystem_SetColour);
		ADD_INTERNAL_CALL(ParticleSystem_GetColourMin);
		ADD_INTERNAL_CALL(ParticleSystem_SetColourMin);
		ADD_INTERNAL_CALL(ParticleSystem_GetColourMax);
		ADD_INTERNAL_CALL(ParticleSystem_SetColourMax);

		ADD_INTERNAL_CALL(ParticleSystem_GetSpeedValueType);
		ADD_INTERNAL_CALL(ParticleSystem_SetSpeedValueType);
		ADD_INTERNAL_CALL(ParticleSystem_GetSpeed);
		ADD_INTERNAL_CALL(ParticleSystem_SetSpeed);
		ADD_INTERNAL_CALL(ParticleSystem_GetSpeedMin);
		ADD_INTERNAL_CALL(ParticleSystem_SetSpeedMin);
		ADD_INTERNAL_CALL(ParticleSystem_GetSpeedMax);
		ADD_INTERNAL_CALL(ParticleSystem_SetSpeedMax);

		ADD_INTERNAL_CALL(ParticleSystem_GetColourOverLifetime);
		ADD_INTERNAL_CALL(ParticleSystem_SetColourOverLifetime);

		ADD_INTERNAL_CALL(ParticleSystem_GetCollision);
		ADD_INTERNAL_CALL(ParticleSystem_SetCollision);

		ADD_INTERNAL_CALL(ParticleSystem_GetTextureID);
		ADD_INTERNAL_CALL(ParticleSystem_SetTextureID);

		ADD_INTERNAL_CALL(ParticleSystem_IsExpired);

		ADD_INTERNAL_CALL(ParticleSystem_GetSystemTimer);
		ADD_INTERNAL_CALL(ParticleSystem_SetSystemTimer);

		// Console logging
		ADD_INTERNAL_CALL(Log);
		ADD_INTERNAL_CALL(LogWarn);
		ADD_INTERNAL_CALL(LogError);

		//Physics
		ADD_INTERNAL_CALL(RigidBody_GetVelocity);
		ADD_INTERNAL_CALL(RigidBody_SetVelocity);
		ADD_INTERNAL_CALL(RigidBody_AddForce);
		ADD_INTERNAL_CALL(ColliderShape_IsEnabled);
		ADD_INTERNAL_CALL(ColliderShape_SetEnabled);
		ADD_INTERNAL_CALL(RigidBody_GetGravityFactor);
		ADD_INTERNAL_CALL(RigidBody_SetGravityFactor);
		ADD_INTERNAL_CALL(RigidBody_IsGravityOff);
		ADD_INTERNAL_CALL(RigidBody_OffGravity);
		ADD_INTERNAL_CALL(Physics_Raycast);

		//LayerMask
		ADD_INTERNAL_CALL(LayerMask_GetMask);
		ADD_INTERNAL_CALL(LayerMask_LayerToName);
		ADD_INTERNAL_CALL(LayerMask_NameToLayer);

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
		ADD_INTERNAL_CALL(SafeToChange);

		// Navigation
		ADD_INTERNAL_CALL(GetNavAgent);
		ADD_INTERNAL_CALL(NavAgent_ComponentState);
		ADD_INTERNAL_CALL(NavAgent_SetDestination);
		ADD_INTERNAL_CALL(NavAgent_Stop);
		ADD_INTERNAL_CALL(NavAgent_GetSpeed);
		ADD_INTERNAL_CALL(NavAgent_SetSpeed);
		ADD_INTERNAL_CALL(NavAgent_HasPath);
		ADD_INTERNAL_CALL(NavAgent_GetComponentEnabled);
		ADD_INTERNAL_CALL(NavAgent_SetComponentEnabled);

		//UI
		ADD_INTERNAL_CALL(Slider_GetValue);
		ADD_INTERNAL_CALL(Slider_SetValue);
	}

#pragma endregion

}
