#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <memory>
#include "WorldSpaceGraphicsSystem.h"
#include "CameraSystem.h"
#include "../ECS/ECSTypes.h"

namespace SliceEngine
{
	class RenderManager
	{
	public:		
		RenderManager();
		~RenderManager();

		void InitAndLink(GLFWwindow* window);

		void UpdateCamGPU(GLFWwindow* window, ResourceManager* rcManager);
		void Render(GLFWwindow* window, ResourceManager* rcManager);
		
		void CreateFramebuffer(int width, int height);
		GLuint GetTexture();

		Transform& GetMainCameraTransform();

		void IDPick(const int& mouseX, const int& mouseY);

		GLuint mFBO;	// For drawing the scene onto a texture
		//GLuint pboIds[2];	// For Object Picking
		//GLuint pboIdx[2];
		unsigned int mIDHovered;

	private:

		entt::entity mainCam;

		//std::shared_ptr<WorldSpaceGraphicsSystem> mWorldSpaceGraphics;
		//std::shared_ptr<CameraSystem> mCameraSys;
	};
}

#endif