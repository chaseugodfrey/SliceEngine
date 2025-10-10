#include <pch.h>
#include "LightingSystem.h"

#include "../Core/Core.h"
#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "CameraSystem.h"
#include "WorldSpaceGraphicsSystem.h"

namespace SliceEngine
{
	void LightingSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& light = reg.get<Light>(entity);

		// Setup Basic Camera Components
		light.color = glm::vec3(1.0f, 1.0f, 1.0f);
		light.intensity = 0.5f;

		glCreateTextures(GL_TEXTURE_2D, 1, &light.depthTex);
		glTextureStorage2D(light.depthTex, 1, GL_DEPTH_COMPONENT32F, SHADOW_DIMENSION, SHADOW_DIMENSION);
		glTexParameteri(light.depthTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(light.depthTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(light.depthTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(light.depthTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	void LightingSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto& light = reg.get<Light>(entity);

		glDeleteTextures(1, &light.depthTex);
	}
	void LightingSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{

	}

}