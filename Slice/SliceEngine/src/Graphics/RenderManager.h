/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			RenderManager.cpp
 author:		Won Yu Xuan Rainne
 email:			won.m@digipen.edu
 brief:			Handles the Rendering pipeline, and related things

				Functions here assumes CreateCamera is called once, and therefore mainCam has value

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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
		GameObject CreateCamera();
		void SetMainGameCamera(GameObject cam);
		std::optional<GameObject>& GetGameCamera();
		void GetCameraAxis(GameObject& cam, glm::vec3& forward, glm::vec3& right, glm::vec3& up);

		void SelectCamIDPick(Entity cam);
		unsigned int ObjectPick(int mouseX, int mouseY);
		unsigned int GetPickedID();
		// Rendering functions
		void CalculateVP(Entity& cam);
		void UpdateCamGPU(Entity& cam);
		// Rendering calls
		void Render();
		void RenderDebug(Entity& cam);
		void DeferredRender();
		// Utility functions
		bool UniformExists(const char* str, GLint& ref);
		void LinkTransformInstancing(GUID guid);
		void LinkDebugLineInstancing(GUID guid);

	private:
		const int mMaxInstance = 100;
		const float zeroFiller[4]{ 0.f,0.f,0.f,0.f };
		const float oneFiller[4]{ 1.f,1.f,1.f,1.f };


		GLuint mFBO;	// For drawing the scene onto a texture
		GLuint mIVBO;
		GLuint mDebugLineVBO;
		//GLuint mRBO;
		GLuint pboIds[2];	// For Object Picking
		GLuint pboIdx[2];
		unsigned int mObjPickX{}, mObjPickY{};
		bool mObjPickedThisFrame{ false };
		Entity mCurrentCamIDHover;
		unsigned int mIDHovered;
		
		std::optional<GameObject> mainCam;

		Handle<SliceEngineTypes::Shader> mCurrShader;
		Handle<SliceEngineTypes::Shader> mInstanceShader;
		Handle<SliceEngineTypes::Shader> mDebugLineShader;
		std::vector<glm::mat4> mInstanceVtx;
		GLuint mColAttachment[4];
		glm::mat4 V, P;

		enum class FBOSetting : unsigned char
		{
			UNBIND,
			BIND,
			COLOR_ONLY,
			POS_NOM,
			POS_NOM_TEX,
			ID,
			ID_POS_NOM,
			ID_POS_NOM_TEX
		};
		enum class GPUSetting : unsigned char
		{
			DEFAULT
		};
		enum class BufferClearSetting : unsigned char
		{
			DEFAULT,
			ALL
		};
		void LinkFrameBufferSettings(FBOSetting setting);
		void LoadSettings(GPUSetting setting);
		void ClearBuffer(BufferClearSetting setting);

		void IDPick();
	};
}

#endif