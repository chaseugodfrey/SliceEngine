/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			CameraSystem.cpp
 author:		Won Yu Xuan Rainne
 email:			won.m@digipen.edu
 brief:			Handles Creation of individual render targets for each camera

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "CameraSystem.h"

#include "../Core/Core.h"

namespace SliceEngine
{
	void CameraSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& cam = reg.get<Camera>(entity);

		if (reg.any_of<SliceEntity>(entity))
		{
			if (gameCameras.empty())
			{
				mainCam = entity;
				cam.isMainCamera = true;
			}

			gameCameras.push_back(entity);
		}

		//glfwGetWindowSize(Core::GetInstance()->GetWindow(), &cam.width, &cam.height);

		// Create Textures
		glCreateTextures(GL_TEXTURE_2D, 1, &cam.textureID);
		glTextureStorage2D(cam.textureID, 1, GL_RGBA16F, maxWidth, maxHeight);
		glTextureParameterf(cam.textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameterf(cam.textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glCreateTextures(GL_TEXTURE_2D, 1, &mScene.picker_id);
		//glTextureStorage2D(mScene.picker_id, 1, GL_R32UI, width, height);
		//glTextureParameterf(mScene.picker_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTextureParameterf(mScene.picker_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mScene.picker_id, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &cam.depthTex);
		glTextureStorage2D(cam.depthTex, 1, GL_DEPTH_COMPONENT32F, maxWidth, maxHeight);
	
		// float_32 rgba Keep Tracks of the Running luminance Amount
		glCreateTextures(GL_TEXTURE_2D, 2, cam.lum);
		glTextureStorage2D(cam.lum[0], 1, GL_RGBA32F, 1, 1);
		glTextureParameterf(cam.lum[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(cam.lum[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureStorage2D(cam.lum[1], 1, GL_RGBA32F, 1, 1);
		glTextureParameterf(cam.lum[1], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(cam.lum[1], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	}
	void CameraSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto& cam = reg.get<Camera>(entity);

		if (auto temp = reg.try_get<EngineEntity>(entity))
			return;

		for (auto it = gameCameras.begin(); it != gameCameras.end(); it++)
		{
			if (*it == entity)
			{
				gameCameras.erase(it);
				break;
			}
		}
		
		if (entity == mainCam)
		{
			// get the next camera in the vector
			if (!gameCameras.empty())
			{
				mainCam = *gameCameras.begin();
				reg.get<Camera>(*gameCameras.begin()).isMainCamera = true;
			}
			else
				// if game camera is empty, then set it to a null opt
				mainCam = std::nullopt;
		}

		glDeleteTextures(1, &cam.textureID);
		//glDeleteTextures(1, &mScene.picker_id);
		glDeleteTextures(1, &cam.depthTex);
		glDeleteTextures(2, cam.lum);
	}
	void CameraSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		
	}
	std::optional<Entity> CameraSystem::GetCamera(Entity entity)
	{
		for (auto camEntity : gameCameras)
		{
			if (camEntity == entity)
			{
				return entity;
			}
		}

		return std::nullopt;
	}
}