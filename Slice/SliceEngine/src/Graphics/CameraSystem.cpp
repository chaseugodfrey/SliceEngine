#include <pch.h>
#include "ResourceManager.h"
#include "CameraSystem.h"

namespace SliceEngine
{
	void CameraSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& cam = reg.get<Camera>(entity);

		// Setup Basic Camera Components
		cam.near = 0.5f;
		cam.far = 200.f;
		cam.pov = 60.f;

		// Create Textures
		glCreateTextures(GL_TEXTURE_2D, 1, &cam.textureID);
		glTextureStorage2D(cam.textureID, 1, GL_RGBA16F, GAMESCREENWIDTH, GAMESCREENHEIGHT);
		glTextureParameterf(cam.textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameterf(cam.textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glCreateTextures(GL_TEXTURE_2D, 1, &mScene.picker_id);
		//glTextureStorage2D(mScene.picker_id, 1, GL_R32UI, width, height);
		//glTextureParameterf(mScene.picker_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTextureParameterf(mScene.picker_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mScene.picker_id, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &cam.depthTex);
		glTextureStorage2D(cam.depthTex, 1, GL_DEPTH_COMPONENT32F, GAMESCREENWIDTH, GAMESCREENHEIGHT);
	}
	void CameraSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto& cam = reg.get<Camera>(entity);

		glDeleteTextures(1, &cam.textureID);
		//glDeleteTextures(1, &mScene.picker_id);
		glDeleteTextures(1, &cam.depthTex);
	}
	void CameraSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{

	}
}