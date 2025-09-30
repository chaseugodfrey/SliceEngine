/*
*	Functions here assumes CreateCamera is called once, and therefore mainCam has value
*/

#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <memory>
#include "../ECS/ECSTypes.h"
#include "../ECS/GameObject.h"

#include "Resource/ResourceManager.h"
#include "Resource/Resource.h"

namespace SliceEngine
{
	class RenderManager
	{
	public:
		// Default constructor and destructor
		RenderManager();
		~RenderManager();
		// One-time setup functions
		void CreateFramebuffer();
		void CreateInstancingParams();
		void CreateDeferredTextures();
		// Camera related functions
		GameObject& CreateCamera();
		Entity& GetMainCamera();
		void GetCameraAxis(const Entity& cam, glm::vec3& forward, glm::vec3& right, glm::vec3& up);

		void IDPick(const int& mouseX, const int& mouseY);
		// Rendering functions
		void CalculateVP(Entity& cam);
		void UpdateCamGPU(Entity& cam);
		// Rendering calls
		void Render();
		void RenderDebug(Entity& cam);
		void DeferredRender();
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
		const float zeroFiller[4]{ 0.f,0.f,0.f,0.f };
		const float oneFiller[4]{ 1.f,1.f,1.f,1.f };

		std::optional<Entity> mainCam;

		Handle<SliceEngineTypes::Shader> mCurrShader;
		Handle<SliceEngineTypes::Shader> mInstanceShader;
		Handle<SliceEngineTypes::Shader> mDebugLineShader;
		std::vector<glm::mat4> mInstanceVtx;
		GLuint mColAttachment[2];
		glm::mat4 V, P;

		enum class FBOSetting : unsigned char
		{
			UNBIND,
			BIND,
			COLOR_ONLY,
			COLOR_POS_NOM
		};
		void LinkFrameBufferSettings(FBOSetting setting);
	};
}

#endif