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

#include "Resource/ResourceManager.h"
#include "Resource/Resource.h"

namespace SliceEngine
{
	class ResourceManager;
	class RenderManager
	{
	public:		
		RenderManager();
		~RenderManager();

		GameObject& CreateCamera();
		void CreateInstancingParams();

		void UpdateCamGPU(Entity& cam);
		void Render();

		void RenderDebug(Entity& cam);
		
		void CreateFramebuffer();
		GLuint GetTexture();

		Transform& GetMainCameraTransform();
		void GetMainCameraAxis(glm::vec3& forward, glm::vec3& right, glm::vec3& up);

		void LinkInstancing(const std::string& mdlName);

		void IDPick(const int& mouseX, const int& mouseY);

		GLuint mFBO;	// For drawing the scene onto a texture
		GLuint mIVBO;
		//GLuint pboIds[2];	// For Object Picking
		//GLuint pboIdx[2];
		unsigned int mIDHovered;

	private:
		std::optional<Entity> mainCam;

		Handle<SliceEngineTypes::Shader> mCurrShader;
		Handle<SliceEngineTypes::Shader> mInstanceShader;
		std::vector<glm::mat4> mInstanceVtx;


		//std::shared_ptr<WorldSpaceGraphicsSystem> mWorldSpaceGraphics;
		//std::shared_ptr<CameraSystem> mCameraSys;
	};
}

#endif