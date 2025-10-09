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
		void CalculateVP(Entity cam);
		void UpdateCamGPU(Entity cam);
		// Rendering calls
		void Render();
		void RenderDebug(Entity cam);
		void DeferredRender(Entity cam);
		// Utility functions
		bool UniformExists(const char* str, GLint& ref);
		void LinkTransformInstancing(GUID guid);
		void LinkDebugLineInstancing();

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
		enum GPU_OUT : unsigned char
		{
			G_DIF = 0,
			G_POS,
			G_NOM,
			G_ID,
			G_TOTAL
		};

		GLuint mColAttachment[GPU_OUT::G_TOTAL];
		glm::mat4 V, P;

		enum FBOSet : unsigned char
		{
			F_UNBIND			= 0b1000'0000,
			F_BIND				= 0b0100'0000,
			F_CLEAR				= 0x00,
			F_ID				= 0b0000'0001,
			F_POS				= 0b0000'0010,
			F_NOM				= 0b0000'0100,
			F_TEX				= 0b0000'1000,
			F_POS_NOM			= 0b0000'0110,
			F_POS_NOM_TEX		= 0b0000'1110,
			F_ID_POS_NOM		= 0b0000'0111,
			F_ID_POS_NOM_TEX	= 0b0000'1111
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
		void LinkFrameBufferSettings(FBOSet setting);
		void LoadSettings(GPUSetting setting);
		void ClearBuffer(BufferClearSetting setting);

		void IDPick();
	};
}

#endif