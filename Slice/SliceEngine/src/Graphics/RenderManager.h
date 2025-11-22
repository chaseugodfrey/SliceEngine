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
		void GatherDrawCalls();
		// Rendering calls
		void Render();
		void RenderDebug(Entity cam);
		void RenderPointShadowMaps();
		void RenderDirectionalShadowMaps(Entity cam);
		void RenderSkybox(Entity cam);
		void RenderLighting(Entity cam);
		void RenderAfterLighting(Entity cam);
		void RenderFog(Entity cam);
		void RenderBloom(Entity cam);
		void RenderVignette(Entity cam);
		void RenderGammaCorrection(Entity cam);
		// Utility functions
		bool UniformExists(const char* str, GLint& ref);
		//void LinkTransformInstancing(GUID guid);
		
		// Colors
		glm::vec4 mNavMeshDebugColor_Base{ 0.f, 0.f, 0.7f, 0.4f };
		glm::vec4 mNavMeshDebugColor_Bounds{ 0.f, 0.2f, 0.25f, 0.85f };

	private:
		const int mMaxInstance = 500;
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
		struct InstanceData
		{
			glm::mat4 mtx;
			glm::ivec4 mat;
		};
		struct RenderCmd
		{
			Handle<SliceEngineTypes::Model> mdl;
			Handle<SliceEngineTypes::Material> mat;
			glm::mat4 mtx;
		};
#pragma region Enums
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
			S_SKYBOX		= 10501127717050996268,
			S_LIGHTING		= 17353385404596894578,
			S_PARTICLES		= 15022037422749583333,
			S_INSTANCED		= 17697828682138082227,
			S_DEBUG_LINE	= 13567802095736790143,
			S_FOG			= 10740115564374233650,
			S_BLOOM_SPLIT	= 12702531725689492235,
			S_DOWNSCALING	= 9611694325200796232,
			S_UPSCALING		= 17037775471000192005,
			S_BLOOM_JOIN	= 11454882705531309873,
			S_VIGNETTE		= 15557538937295862472,
			S_FINAL			= 9302529766740298710
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
		enum GPUSetting : unsigned char
		{
			GPS_ENABLE_CULL_FACE	= 0b0000'0001,
			GPS_ENABLE_BLEND		= 0b0000'0010,
			GPS_ENABLE_DEPTH		= 0b0000'0100,

			GPS_CULL_BACK_NOT_FRONT	= 0b0001'0000, // Else Cull Front
			GPS_BLEND_ONE_ONE		= 0b0010'0000,
			GPS_BLEND_SRC_ONEMINUS	= 0b0100'0000,
			GPS_DEPTH_LESS			= 0b1000'0000,

			GPS_NONE				= 0x00,
			GPS_DEFAULT				= 0b1001'0101,
			GPS_PARTICLES			= 0b1100'0110,
			GPS_SHADOW				= 0b1000'0101,
			GPS_SPE_ADDITION		= 0b0010'0011,
			GPS_ADDITION			= 0b0011'0011,
			GPS_DEBUG				= 0b1100'0110,
			GPS_BLOOM				= 0b0001'0001,
			GPS_BLOOM2				= 0b0011'0011
		};
		enum class BufferClearSetting : unsigned char
		{
			COLOR_ONLY,
			ALL
		};
#pragma endregion
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
		std::vector<InstanceData> mInstanceVtx;

		GLuint SkyboxMap{};
		GLuint mColAttachment[GOUT_TOTAL]{};
		GPU_OUT mCurrFinalColAttachment{ GOUT_FINAL };
		std::vector<BloomMip> mBloomMips;
		GPUSetting mCurrGPUSetting{ GPS_NONE };
		glm::mat4 V, P;

		void SetDirectionalLightMtx(glm::vec3 camPos, glm::vec3 lightPos);
		void LinkFrameBufferSettings(FBOType fbo, int numColAttachments, ...);
		void LoadSettings(GPUSetting setting);
		void QuickSetSettings(GPUSetting setting, bool toggleOn);
		void SetShader(ShaderOpt sh);
		void ClearBuffer(BufferClearSetting setting);
		void ToggleFinalTexture();

		void IDPick();
	};
}

#endif