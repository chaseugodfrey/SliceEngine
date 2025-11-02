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
		void CreateFramebuffers();
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
		void UpdateCamVP();
		void BindCameraDepth(Entity cam);
		// Rendering calls
		void Render();
		void RenderDebug(Entity cam);
		void RenderPointShadowMaps();
		void RenderDirectionalShadowMaps(Entity cam);
		void RenderLighting(Entity cam);
		void RenderBloom();
		void RenderGammaCorrection(Entity cam);
		// Utility functions
		bool UniformExists(const char* str, GLint& ref);
		void LinkTransformInstancing(GUID guid);

	private:
		const int mMaxInstance = 100;
		const int mMaxBloom =  5;
		const float zeroFiller[4]{ 0.f,0.f,0.f,0.f };
		const float oneFiller[4]{ 1.f,1.f,1.f,1.f };
		const float pointLightFar = 20.f;
		struct ShadowCamDir
		{
			glm::vec3 target;
			glm::vec3 up;
		};
		const ShadowCamDir mShadowCamDir[6]{
			{glm::vec3(1.f,0.f,0.f), glm::vec3(0.f,-1.f,0.f) },
			{glm::vec3(-1.f,0.f,0.f), glm::vec3(0.f,-1.f,0.f)},
			{glm::vec3(0.f,1.f,0.f), glm::vec3(0.f,0.f,1.f)},
			{glm::vec3(0.f,-1.f,0.f), glm::vec3(0.f,0.f,-1.f)},
			{glm::vec3(0.f,0.f,1.f), glm::vec3(0.f,-1.f,0.f) },
			{glm::vec3(0.f,0.f,-1.f), glm::vec3(0.f,-1.f,0.f)}
		};
		struct BloomMip
		{
			glm::vec2 size;
			glm::ivec2 intSize;
			GLuint tex;
		};
		enum FBOType : unsigned char
		{
			FB_NIL = 0,		// 0 Outs
			FB_DEFERRED,	// 4 Outs
			FB_FINAL,		// 1 Out
			FB_TOTAL		// NO BIND
		};
		enum ShaderOpt : uint64_t
		{
			S_BASIC			= 18310719961107313904,
			S_SHADOW		= 15542823559299526962,
			S_POINT_SHADOW	= 16403285895328080424,
			S_DEFERRED		= 9461939409271178249,
			S_LIGHTING		= 17353385404596894578,
			S_FINAL			= 9302529766740298710,
			S_INSTANCED		= 17697828682138082227,
			S_DEBUG_LINE	= 13567802095736790143,
			S_BLOOM_SPLIT	= 12702531725689492235,
			S_DOWNSCALING	= 9611694325200796232,
			S_UPSCALING		= 17037775471000192005
		};

		enum GPU_OUT : unsigned char
		{
			GOUT_DIF = 0,
			GOUT_POS,
			GOUT_NOM,
			GOUT_ID,
			GOUT_ROUGH_METAL,
			GOUT_FINAL,
			GOUT_POST,
			GOUT_TOTAL
		};

		enum class GPUSetting : unsigned char
		{
			DEFAULT,
			SHADOW,
			SPE_ADDITION,
			ADDITION,
			DEBUG,
			TOTAL
		};
		enum class BufferClearSetting : unsigned char
		{
			COLOR_ONLY,
			ALL
		};

		FBOType mCurrFBO{ FB_TOTAL };
		GLuint mFBO[FB_TOTAL]{};	// For drawing the scene onto a texture
		GLuint mIVBO{};
		//GLuint mRBO;
		GLuint pboIds[2]{};	// For Object Picking
		GLuint pboIdx[2]{};
		unsigned int mObjPickX{}, mObjPickY{};
		bool mObjPickedThisFrame{ false };
		Entity mCurrentCamIDHover{};
		unsigned int mIDHovered{};

		std::optional<GameObject> mainCam;
		Handle<SliceEngineTypes::Shader> shaderHandle;
		std::pair<ShaderOpt, GLuint> mCurrShader;
		std::vector<glm::mat4> mInstanceVtx;

		GLuint mColAttachment[GOUT_TOTAL]{};
		std::vector<BloomMip> mBloomMips;
		GPUSetting mCurrGPUSetting{ GPUSetting::TOTAL };
		glm::mat4 V, P;

		void SetDirectionalLightMtx(glm::vec3 camPos, glm::vec3 lightPos);
		void LinkFrameBufferSettings(FBOType fbo, int numColAttachments, ...);
		void LoadSettings(GPUSetting setting);
		void SetShader(ShaderOpt sh);
		void ClearBuffer(BufferClearSetting setting);

		void IDPick();
	};
}

#endif