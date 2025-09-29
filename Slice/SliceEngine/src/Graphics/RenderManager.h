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
		// Default constructor and destructor
		RenderManager();
		~RenderManager();
		// One-time setup functions
		void CreateInstancingParams();
		void CreateFramebuffer();
		// Camera related functions
		GameObject& CreateCamera();
		GameObject GetGameCamera();
		void GetCameraAxis(GameObject& cam, glm::vec3& forward, glm::vec3& right, glm::vec3& up);

		void LinkInstancing(const std::string& mdlName);

		void IDPick(const int& mouseX, const int& mouseY);
		// Rendering functions
		void CalculateVP(Entity& cam);
		void UpdateCamGPU(Entity& cam);
		// Rendering calls
		void Render();
		void RenderDebug(Entity& cam);
		// Utility functions
		bool UniformExists(const char* str, GLint& ref);
		void LinkTransformInstancing(const std::string& mdlName);
		void LinkDebugLineInstancing(const std::string& mdlName);

		GLuint mFBO;	// For drawing the scene onto a texture
		GLuint mIVBO;
		GLuint mDebugLineVBO;
		//GLuint pboIds[2];	// For Object Picking
		//GLuint pboIdx[2];
		unsigned int mIDHovered;

	private:
		const int mMaxInstance = 100;

		std::optional<Entity> mainCam;

		Handle<SliceEngineTypes::Shader> mCurrShader;
		Handle<SliceEngineTypes::Shader> mInstanceShader;
		Handle<SliceEngineTypes::Shader> mDebugLineShader;
		std::vector<glm::mat4> mInstanceVtx;
		glm::mat4 V, P;

		//std::shared_ptr<WorldSpaceGraphicsSystem> mWorldSpaceGraphics;
		//std::shared_ptr<CameraSystem> mCameraSys;
	};
}

#endif