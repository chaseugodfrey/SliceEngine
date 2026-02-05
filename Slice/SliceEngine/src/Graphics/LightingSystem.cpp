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

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &light.depthMaps);
		glTextureStorage3D(light.depthMaps, 1, GL_DEPTH_COMPONENT32F, SHADOW_DIMENSION, SHADOW_DIMENSION, 7);// mNumCascadeShadow
		glTextureParameteri(light.depthMaps, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(light.depthMaps, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(light.depthMaps, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(light.depthMaps, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.f,1.f,1.f,1.f };
		glTextureParameterfv(light.depthMaps, GL_TEXTURE_BORDER_COLOR, borderColor);
	
		//glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &light.shadowCubeMap);
		//glTexStorage2D(light.shadowCubeMap, 1, GL_DEPTH_COMPONENT24, SHADOW_DIMENSION, SHADOW_DIMENSION); 
		glGenTextures(1, &light.shadowCubeMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light.shadowCubeMap);
		for (u_int i{}; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, SHADOW_DIMENSION, SHADOW_DIMENSION, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	void LightingSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto& light = reg.get<Light>(entity);

		if(light.depthMaps != 0)
			glDeleteTextures(1, &light.depthMaps);
		if (light.shadowCubeMap != 0)
			glDeleteTextures(1, &light.shadowCubeMap);
	}
	void LightingSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{

	}

}