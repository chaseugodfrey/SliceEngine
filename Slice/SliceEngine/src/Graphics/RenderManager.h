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
#include "Core/Events.h"

#include "Resource/ResourceManager.h"
#include "Resource/Resource.h"
#include "RenderCmdManager.h"

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
		void RegenerateSkybox();
		void Update(float dt);
		// Camera related functions
		GameObject CreateCamera();
		GameObject CreatePrefabCam();
		void SetMainGameCamera(Entity cam);
		std::optional<Entity>& GetGameCamera();
		void CopyMainCamSettings(Camera& othCam);
		void GetCameraAxis(GameObject& cam, glm::vec3& forward, glm::vec3& right, glm::vec3& up);
		void GetCameraAxis(glm::mat3& camRot, glm::vec3& forward, glm::vec3& right, glm::vec3& up);
		glm::mat4 DirLightMatCalc(const glm::mat4& proj, const glm::mat4& view, const glm::vec3 lightDir);

		void SelectCamIDPick(Entity cam);
		unsigned int ObjectPick(int mouseX, int mouseY);
		unsigned int GetPickedID();

		float GetSessionExposure() const;
		float GetSessionGamma() const;
		void SetSessionExposure(float exposure);
		void SetSessionGamma(float gamma);

		// Rendering functions
		void CalculateVP(Entity cam);
		void UpdateCamVP();
		void ForceCamNormalVP(Entity cam);
		void BindCameraDepth(Entity cam);
		// Rendering calls
		void Render();
		void RenderDebug(Entity cam);
		void RenderPerspectiveShadowMaps();
		void RenderDirectionalShadowMaps(Entity cam);
		void RenderSkybox();
		void RenderSkyboxLighting(Entity cam);
		void RenderLighting(Entity cam);
		void RenderAvgLum(Entity cam);
		void RenderGroundCloud(Entity cam);
		void RenderFog(Entity cam);
		void RenderBloom(Entity cam, bool specifallyGodRay);
		void RenderVignette(Entity cam);
		void RenderImpact(Entity cam);
		void RenderGammaCorrection(Entity cam);
		void Draw(); // Basically just copies the main camera texture to draw onto screen framebuffer
		// Utility functions
		void ForceSetCustomShader(const std::string& sh, GLuint s);
		bool UniformExists(const char* str, GLint& ref);
		void GatherLights();
		void GatherNearbyLights();
		float CalcPointLightFar(const glm::vec3& scale, const float lightIntensity);
		const glm::mat4& GetViewMatrix() const;
		const glm::mat4& GetProjMatrix() const;
		//void LinkTransformInstancing(GUID guid);
		
		// Colors
		glm::vec4 mNavMeshDebugColor_Base{ 0.f, 0.f, 0.7f, 0.4f };
		glm::vec4 mNavMeshDebugColor_Bounds{ 0.f, 0.2f, 0.25f, 0.85f };
		// Light Textures (I expose them :p)
		GLuint mDirLightDepthMaps{};
		GLuint mShadowCubeMapArr{};
		GLuint SkyboxIrradianceMap{};
		int numLightsFound{};
		float mainDirLightFar{};
		#define mMaxPointLights 10
		const size_t mMaxLights{150};
		const int mNumCascadeShadow = 5; // num of textures, below is -1 from this to account for 0
		const float shadowCascadeLevels[4]{ 40.f, 15.f, 6.f, 2.4f };

	private:
		const float mBloomFilterMult = 0.001f;
		const float mBloomStrengthMult = 0.1f;
		const float mExposureMult = 0.01f;
		float mSessionExposure{ 10.f };
		float mSessionGamma{ 45.454545f };
		const int mMaxBloom =  5;
		const float mLightZDist = 50.f;
		const float mZBufferShadow = 400.f;
		const float mMinShadowSize = 20.f;
		//const float zeroFiller[4]{ 0.f,0.f,0.f,0.f };
		//const float oneFiller[4]{ 1.f,1.f,1.f,1.f };
		const float mPointLightFar = 20.f;
		const int mSkyboxIrrDim = 32;
		const int mSkyboxDim = 1024;

		const unsigned int DIRECTIONAL_SHADOW_DIMENSION = 512;
		const float DIRECTIONAL_MAX_DIST = 400.f;
		const unsigned int SHADOW_DIMENSION = 1024;
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
#pragma region Enums
		enum FBOType : unsigned char
		{
			FB_NIL = 0,		// 0 Outs
			FB_DEFERRED,	// 6 Outs
			FB_THREE,		// 3 Outs
			FB_FINAL,		// 1 Out
			FB_TOTAL		// NO BIND
		};
		enum ShaderOpt
		{
			S_BASIC						,
			S_SHADOW				,
			S_POINT_SHADOW	,
			S_SPOT_SHADOW,
			S_SKYBOX					,
			S_SKYBOX_Light		,
			S_LIGHTING				,
			S_PARTICLES				,
			S_INSTANCED			,
			S_CLOUDS,
			S_DEBUG_LINE			,
			S_DEBUG_OUTLINE	,
			S_DEBUG_OUT_BLUR,
			S_DEBUG_OUTLJOIN,
			S_FOG							,
			S_BLOOM_SPLIT		,
			S_DOWNSCALING		,
			S_UPSCALING			,
			S_BLOOM_JOIN		,
			S_VIGNETTE				,
			S_IMPACT				,
			S_SKY_IRRADIANCE	,
			S_SKY_GENERATE		,
			S_LUMINANCE,
			S_EXT_LUMINANCE,
			S_FINAL						,
			S_COPY						
		};

		std::unordered_map<ShaderOpt, std::string> ShaderPaths =
		{
			{ ShaderOpt::S_BASIC,           "Shaders/basic.shader" },
			{ ShaderOpt::S_SHADOW,          "Shaders/shadow.shader" },
			{ ShaderOpt::S_POINT_SHADOW,    "Shaders/pointShadow.shader" },
			{ ShaderOpt::S_SPOT_SHADOW,		"Shaders/spotShadow.shader" },
			{ ShaderOpt::S_SKYBOX,          "Shaders/skybox.shader" },
			{ ShaderOpt::S_SKYBOX_Light,    "Shaders/skyboxLight.shader" },
			{ ShaderOpt::S_LIGHTING,        "Shaders/lighting.shader" },
			{ ShaderOpt::S_PARTICLES,       "Shaders/particles.shader" },
			{ ShaderOpt::S_INSTANCED,       "Shaders/instanced.shader" },
			{ ShaderOpt::S_CLOUDS,			"Shaders/cloud.shader" },
			{ ShaderOpt::S_DEBUG_LINE,      "Shaders/debugLine.shader" },
			{ ShaderOpt::S_DEBUG_OUTLINE,   "Shaders/debugOutline.shader" },
			{ ShaderOpt::S_DEBUG_OUT_BLUR,  "Shaders/debugOutlineBlur.shader" },
			{ ShaderOpt::S_DEBUG_OUTLJOIN,  "Shaders/debugOutlineJoin.shader" },
			{ ShaderOpt::S_FOG,             "Shaders/fog.shader" },
			{ ShaderOpt::S_BLOOM_SPLIT,     "Shaders/bloomSplit.shader" },
			{ ShaderOpt::S_DOWNSCALING,     "Shaders/downSample.shader" },
			{ ShaderOpt::S_UPSCALING,       "Shaders/upSample.shader" },
			{ ShaderOpt::S_BLOOM_JOIN,      "Shaders/bloomJoin.shader" },
			{ ShaderOpt::S_VIGNETTE,        "Shaders/vignette.shader" },
			{ ShaderOpt::S_IMPACT,			"Shaders/ImpactFrame.shader" },
			{ ShaderOpt::S_SKY_IRRADIANCE,  "Shaders/skyboxIrr.shader" },
			{ ShaderOpt::S_SKY_GENERATE,    "Shaders/skyboxGeneration.shader" },
			{ ShaderOpt::S_LUMINANCE,		"Shaders/luminance.shader" },
			{ ShaderOpt::S_EXT_LUMINANCE,	"Shaders/Extractluminance.shader" },
			{ ShaderOpt::S_FINAL,           "Shaders/final.shader" },
			{ ShaderOpt::S_COPY,            "Shaders/basicCopy.shader" }
		};

		public:
		enum GPU_OUT : unsigned char
		{
			GOUT_DIF = 0,
			GOUT_POS,
			GOUT_NOM,
			GOUT_ID,
			GOUT_ROUGH_METAL,
			GOUT_EMISSION,
			GOUT_GODRAY,
			GOUT_DEBUG_OUTLINE,
			GOUT_DEBUG_OUTLINE_BLURED,
			GOUT_LUM_EXTRACT,
			GOUT_IMPACT,
			GOUT_FINAL,
			GOUT_POST,
			GOUT_TOTAL
		};
		private:
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
			GPS_TEST_TRANSLUCENT	= 0b1101'0111,
			GPS_PARTICLES			= 0b1100'0110,
			GPS_SKYBOX				= 0b0000'0001,
			GPS_SKYBOX_AMBIENT		= 0b0101'0011,
			GPS_SHADOW				= 0b1001'0100,
			GPS_SPE_ADDITION		= 0b0010'0011,
			GPS_ADDITION			= 0b0011'0011,
			GPS_DEBUG				= 0b1100'0110,
			GPS_DEBUG_OUTLINE_BLEND	= 0b0101'0011,
			GPS_BLOOM				= 0b0001'0001,
			GPS_BLOOM2				= 0b0011'0011
		};
		enum class BufferClearSetting : unsigned char
		{
			COLOR_ONLY,
			ALL
		};
		struct LightDat
		{
			glm::vec3 pos;
			float uFarPlane;
			glm::vec3 dir;
			int type;
			glm::vec4 col;
			int hasShadow;
			int shadowNum;
			int spotShadowNum;
			float pointAngle;
			glm::mat4 pointlightMtx;
		};
#pragma endregion
		FBOType mCurrFBO{ FB_TOTAL };
		GLuint mFBO[FB_TOTAL]{};	// For drawing the scene onto a texture
		GLuint mShadowUBO;
		GLuint mLightUBO;
		//GLuint mRBO;
		GLuint pboIds[2]{};	// For Object Picking
		GLuint pboIdx[2]{};
		unsigned int mObjPickX{}, mObjPickY{};
		bool mObjPickedThisFrame{ false };
		Entity mCurrentCamIDHover{};
		unsigned int mIDHovered{};
		float mTime{};

		bool mDirLightFound{ false };
		LightDat dirLightDat;
		std::vector<LightDat> allLightData{}; // for raw data
		std::vector<size_t> sortedLights; // for sorting
		std::unordered_set<size_t> activeShadowSet{};
		std::vector<size_t> dirtyShadows{};

		Handle<SliceEngineTypes::Shader> shaderHandle;
		std::pair<std::string, GLuint> mCurrShader;
		RenderCmdManager renderQueue;

		GLuint SkyboxMap{};

		public:
		GLuint mColAttachment[GOUT_TOTAL]{};
		private:
		GPU_OUT mCurrFinalColAttachment{ GOUT_FINAL };
		std::vector<BloomMip> mBloomMips;
		GPUSetting mCurrGPUSetting{ GPS_NONE };
		glm::mat4 V, P;// Camera's
		glm::vec3 cameraPos;

		std::vector<glm::mat4> mDebugDrawRays;

		void LinkFrameBufferSettings(FBOType fbo, int numColAttachments, ...);
		void LoadSettings(GPUSetting setting);
		void QuickSetSettings(GPUSetting setting, bool toggleOn);
		void ForceResetDefaultSettings();
		void SetShader(std::string sh);
		void ClearBuffer(BufferClearSetting setting);
		void ToggleFinalTexture();
		void SetUniformVec3(GLint uniformLoc, const glm::vec3& vec);

		void AddDebugRaysToDraw(const DebugDrawRayEvent&);

		void IDPick();

		public:
		struct SkyboxDat
		{
			bool isDirty = true;
			float lightingPower{ 100.f };
			glm::vec3 zenithColor{ 0.733f, 0.87f, 1.0f };
			glm::vec3 horizonColor{ 0.815f, 0.917f, 1.0f };
			glm::vec3 groundColor{ 1.f,1.f, 1.0f };
			glm::vec3 sunPos{ 0.0, 1.0, 0.0 };
			glm::vec3 sunCol{ 1.f, 1.f, 1.f };
		};
		SkyboxDat skyboxData;
	};
}

#endif