/*
*	Functions here assumes CreateCamera is called once, and therefore mainCam has value
*/

#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <memory>
#include "WorldSpaceGraphicsSystem.h"
#include "CameraSystem.h"
#include "../ECS/ECSTypes.h"
#include "../ECS/GameObject.h"

namespace SliceEngine
{
	class RenderManager
	{
	public:		
		RenderManager();
		~RenderManager();

		GameObject& CreateCamera();

		void UpdateCamGPU(ResourceManager* rcManager, Entity& cam);
		void Render(ResourceManager* rcManager);
		
		void CreateFramebuffer();
		GLuint GetTexture();

		Transform& GetMainCameraTransform();
		void GetMainCameraAxis(glm::vec3& forward, glm::vec3& right, glm::vec3& up);

		void IDPick(const int& mouseX, const int& mouseY);

		GLuint mFBO;	// For drawing the scene onto a texture
		//GLuint pboIds[2];	// For Object Picking
		//GLuint pboIdx[2];
		unsigned int mIDHovered;

	private:
		std::optional<Entity> mainCam;

		//std::shared_ptr<WorldSpaceGraphicsSystem> mWorldSpaceGraphics;
		//std::shared_ptr<CameraSystem> mCameraSys;
	};
}

#endif