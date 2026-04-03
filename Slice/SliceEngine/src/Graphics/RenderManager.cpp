/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			RenderManager.cpp
 author:		Won Yu Xuan Rainne
 email:			won.m@digipen.edu
 brief:			Handles the Rendering pipeline, and related things

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "RenderManager.h"
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define PI05F 1.57079632679f
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/quaternion.hpp"
#include "TransformHelper.h"

#include "Core/Core.h"

#include "CameraSystem.h"
#include "LightingSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Systems/ParticleSystemManager.h"
#include "Core/EventManager.h"

#include "Resource/Shader.h"
#include "Resource/Model.h"

extern void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

// My Comments to (Ctrl + f): -TODO- MAYDO:

namespace SliceEngine
{
	struct PrefabCameraEntity {};


#pragma region Generate GPU Objects
	RenderManager::RenderManager()
	{
		auto* eventManager = EventManager::GetInstance();
		eventManager->Subscribe<DebugDrawRayEvent, &RenderManager::AddDebugRaysToDraw>(this);

		CreateFramebuffers();
	}
	RenderManager::~RenderManager()
	{
		glDeleteFramebuffers(FB_TOTAL, mFBO);
		if(mShadowUBO != 0)
			glDeleteBuffers(1, &mShadowUBO);
		if(mLightUBO != 0)
			glDeleteBuffers(1, &mLightUBO);


		if (mDirLightDepthMaps != 0)
			glDeleteTextures(1, &mDirLightDepthMaps);
		if (mShadowCubeMapArr != 0)
			glDeleteTextures(1, &mShadowCubeMapArr);

		glDeleteTextures(GOUT_TOTAL, mColAttachment);
		for (auto i : mBloomMips)
			glDeleteTextures(1, &i.tex);

		if (SkyboxMap != 0)
			glDeleteTextures(1, &SkyboxMap);

		if (SkyboxIrradianceMap != 0)
			glDeleteTextures(1, &SkyboxIrradianceMap);

		glDeleteBuffers(2, pboIds);
	}
	void RenderManager::CreateFramebuffers()
	{
		glGenFramebuffers(FB_TOTAL, mFBO);

		{
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO[FB_DEFERRED]);

			unsigned int drawBuffers[] = {
				GL_COLOR_ATTACHMENT0
				,GL_COLOR_ATTACHMENT1
				,GL_COLOR_ATTACHMENT2
				,GL_COLOR_ATTACHMENT3
				,GL_COLOR_ATTACHMENT4
				,GL_COLOR_ATTACHMENT5
			};
			glDrawBuffers(sizeof(drawBuffers) / sizeof(unsigned int), drawBuffers);
		}

		{
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO[FB_THREE]);

			unsigned int drawBuffers[] = {
				GL_COLOR_ATTACHMENT0
				,GL_COLOR_ATTACHMENT1
				,GL_COLOR_ATTACHMENT2
			};
			glDrawBuffers(sizeof(drawBuffers) / sizeof(unsigned int), drawBuffers);
		}

		{
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO[FB_FINAL]);
			unsigned int drawBuffers[] = {
				GL_COLOR_ATTACHMENT0
			};
			glDrawBuffers(sizeof(drawBuffers) / sizeof(unsigned int), drawBuffers);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, mFBO[FB_NIL]);
		glDrawBuffer(GL_NONE);

		//glGenRenderbuffers(1, &mRBO);
		//glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Core::GetInstance()->GetSystem<CameraSystem>().maxWidth, Core::GetInstance()->GetSystem<CameraSystem>().maxHeight);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);

		// Note: Framebuffer is always going to be incomplete this way due to not attaching a texture to it
		//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		//{
		//	SLICE_LOG_WARNING("Framebuffer not complete");
		//}

		glCreateBuffers(2, pboIds);
		glNamedBufferData(pboIds[0], 4, NULL, GL_STREAM_READ); // 4 bytes (Float)
		glNamedBufferData(pboIds[1], 4, NULL, GL_STREAM_READ);
		pboIdx[0] = 0;
		pboIdx[1] = 1;

		LinkFrameBufferSettings(FB_TOTAL, 0);
		CheckGLError();
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	void RenderManager::CreateInstancingParams()
	{
		glCreateBuffers(1, &mShadowUBO);
		glNamedBufferStorage(mShadowUBO, mNumCascadeShadow * sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, mShadowUBO);

		glCreateBuffers(1, &mLightUBO);
		glNamedBufferStorage(mLightUBO, (mMaxLights + 1) * sizeof(LightDat), nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, mLightUBO);

		CheckGLError();
	}
	void RenderManager::CreateDeferredTextures()
	{
		glCreateTextures(GL_TEXTURE_2D, GOUT_TOTAL, mColAttachment);

		const int maxHeight{ Core::GetInstance()->GetSystem<CameraSystem>().maxHeight }, maxWidth{ Core::GetInstance()->GetSystem<CameraSystem>().maxWidth };
		// uint_32 Entity_ID
		glTextureStorage2D(mColAttachment[GOUT_ID], 1, GL_R32UI, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_ID], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_ID], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// float_16 xyz Position
		glTextureStorage2D(mColAttachment[GOUT_POS], 1, GL_RGB16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_POS], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_POS], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// float_16 xyz Normal
		glTextureStorage2D(mColAttachment[GOUT_NOM], 1, GL_RGB16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_NOM], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_NOM], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// float_16 rgba Diffuse Color
		glTextureStorage2D(mColAttachment[GOUT_DIF], 1, GL_RGBA16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_DIF], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_DIF], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// float_16 rgb-Roughness + Metalic + isIgnoreLighting
		glTextureStorage2D(mColAttachment[GOUT_ROUGH_METAL], 1, GL_RGB16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_ROUGH_METAL], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_ROUGH_METAL], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// float_16 rgb-Emission
		glTextureStorage2D(mColAttachment[GOUT_EMISSION], 1, GL_RGB16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_EMISSION], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_EMISSION], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// float_16 rgb-GodRay
		glTextureStorage2D(mColAttachment[GOUT_GODRAY], 1, GL_RGB16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_GODRAY], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_GODRAY], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// unint_8 object to outline
		glTextureStorage2D(mColAttachment[GOUT_DEBUG_OUTLINE], 1, GL_R8UI, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_DEBUG_OUTLINE], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_DEBUG_OUTLINE], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// unint_8 the surrounding of the object
		glTextureStorage2D(mColAttachment[GOUT_DEBUG_OUTLINE_BLURED], 1, GL_R8UI, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_DEBUG_OUTLINE_BLURED], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_DEBUG_OUTLINE_BLURED], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// float_32 rgba Final Image To Send to Camera Texture
		glTextureStorage2D(mColAttachment[GOUT_LUM_EXTRACT], 9, GL_R32F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_LUM_EXTRACT], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameterf(mColAttachment[GOUT_LUM_EXTRACT], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_LUM_EXTRACT], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameterf(mColAttachment[GOUT_LUM_EXTRACT], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// float_16 rgb Impact Image
		glTextureStorage2D(mColAttachment[GOUT_IMPACT], 1, GL_RGB16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_IMPACT], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_IMPACT], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_IMPACT], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameterf(mColAttachment[GOUT_IMPACT], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// float_32 rgba Final Image To Send to Camera Texture
		glTextureStorage2D(mColAttachment[GOUT_FINAL], 1, GL_RGBA32F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_FINAL], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_FINAL], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_FINAL], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameterf(mColAttachment[GOUT_FINAL], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// float_32 rgba Post Processing for toggling Image To Send to Camera Texture
		glTextureStorage2D(mColAttachment[GOUT_POST], 1, GL_RGBA32F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_POST], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_POST], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_POST], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameterf(mColAttachment[GOUT_POST], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		mBloomMips.reserve(mMaxBloom + 1);
		glm::ivec2 intMip{ maxWidth, maxHeight };
		glm::vec2 mipDim{static_cast<glm::vec2>(intMip)};
		for (int i{}; i < mMaxBloom + 1; ++i)
		{
			BloomMip bm;
			bm.intSize = intMip;
			bm.size = mipDim;
			glCreateTextures(GL_TEXTURE_2D, 1, &bm.tex);
			glTextureStorage2D(bm.tex, 1, GL_R11F_G11F_B10F, intMip.x, intMip.y);
			glTextureParameterf(bm.tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameterf(bm.tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameterf(bm.tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameterf(bm.tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			mBloomMips.emplace_back(std::move(bm));
			intMip /= 2;
			mipDim /= 2.f;
		}

		// Read the texture for loading into skybox
		//GLuint faceTexID = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>((GUID)18349208178533231704).get()->texture_id;
		//GLuint faceTexID = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>((GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT).get()->texture_id;
		//GLint srcInternalFmt, width, height;
		//glBindTexture(GL_TEXTURE_2D, faceTexID);
		//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &srcInternalFmt);
		//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		
		// ----- Generates Skybox Texture -----
		glGenTextures(1, &SkyboxMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxMap);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB16F, mSkyboxDim, mSkyboxDim);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// Generates Skybox Irradiance Texture
		glGenTextures(1, &SkyboxIrradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxIrradianceMap);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB16F, mSkyboxIrrDim, mSkyboxIrrDim);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		RegenerateSkybox();

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &mDirLightDepthMaps);
		glTextureStorage3D(mDirLightDepthMaps, 1, GL_DEPTH_COMPONENT32F, DIRECTIONAL_SHADOW_DIMENSION, DIRECTIONAL_SHADOW_DIMENSION, mNumCascadeShadow);
		glTextureParameteri(mDirLightDepthMaps, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(mDirLightDepthMaps, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(mDirLightDepthMaps, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(mDirLightDepthMaps, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.f,1.f,1.f,1.f };
		glTextureParameterfv(mDirLightDepthMaps, GL_TEXTURE_BORDER_COLOR, borderColor);

		//glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &light.shadowCubeMap);
		//glTexStorage2D(light.shadowCubeMap, 1, GL_DEPTH_COMPONENT24, SHADOW_DIMENSION, SHADOW_DIMENSION); 
		glGenTextures(1, &mShadowCubeMapArr);
		glBindTexture(  GL_TEXTURE_CUBE_MAP_ARRAY, mShadowCubeMapArr);

		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_DEPTH_COMPONENT24, SHADOW_DIMENSION, SHADOW_DIMENSION, 6 * mMaxPointLights);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

		CheckGLError();
	}
	void RenderManager::RegenerateSkybox()
	{
		auto& mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::CUBE_DEFAULT).get()->meshes[0];
		glm::mat4 proj = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
		
		// ----- Generate Skybox -----
		SetShader(ShaderPaths[S_SKY_GENERATE]);
		LinkFrameBufferSettings(FB_FINAL, 1, SkyboxMap);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
		LoadSettings(GPS_NONE); // Means just draw irregardlesss
		glViewport(0, 0, mSkyboxDim, mSkyboxDim);
		ClearBuffer(BufferClearSetting::COLOR_ONLY);
		glBindVertexArray(mdl.vao);
		GLint uniformLoc;
		// Skybox Settings
		uniformLoc = glGetUniformLocation(mCurrShader.second, "zenithColor");
		glUniform4f(uniformLoc, skyboxData.zenithColor.r, skyboxData.zenithColor.g, skyboxData.zenithColor.b, 1.f);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "horizonColor");
		glUniform4f(uniformLoc, skyboxData.horizonColor.r, skyboxData.horizonColor.g, skyboxData.horizonColor.b, 1.f);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "groundColor");
		glUniform3f(uniformLoc, skyboxData.groundColor.r, skyboxData.groundColor.g, skyboxData.groundColor.b);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "sunPos");
		auto nomSunPos = glm::normalize(skyboxData.sunPos);
		glUniform3f(uniformLoc, nomSunPos.x, nomSunPos.y, nomSunPos.z);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "sunCol");
		glUniform3f(uniformLoc, skyboxData.sunCol.r, skyboxData.sunCol.g, skyboxData.sunCol.b);

		std::stringstream ss{};
		for (size_t i{}; i < 6; ++i)
		{
			glm::mat4 shadowMat{ proj * glm::lookAt(glm::vec3(0.f), mShadowCamDir[i].target, mShadowCamDir[i].up) };
			ss.str("");
			ss << "uShadowMat[" << std::to_string(i) << "]";
			uniformLoc = glGetUniformLocation(mCurrShader.second, ss.str().c_str());
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &shadowMat[0][0]);
		}
		glDrawElements(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr);
		CheckGLError();

		// --TODO-- CRYY, idk why the irridance map became no color if not regenerated ;w;

		// ----- Use generated Map to generate irradiance map -----
		SetShader(ShaderPaths[S_SKY_IRRADIANCE]);
		LinkFrameBufferSettings(FB_FINAL, 1, SkyboxIrradianceMap);
		glViewport(0, 0, mSkyboxIrrDim, mSkyboxIrrDim);
		ClearBuffer(BufferClearSetting::COLOR_ONLY);
		glBindTextureUnit(0, SkyboxMap);
		glBindVertexArray(mdl.vao);

		for (size_t i{}; i < 6; ++i)
		{
			glm::mat4 shadowMat{ proj * glm::lookAt(glm::vec3(0.f), mShadowCamDir[i].target, mShadowCamDir[i].up) };
			ss.str("");
			ss << "uShadowMat[" << std::to_string(i) << "]";
			uniformLoc = glGetUniformLocation(mCurrShader.second, ss.str().c_str());
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &shadowMat[0][0]);
		}
		glDrawElements(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr);

		CheckGLError();
	}
#pragma endregion

#pragma region Camera
	GameObject RenderManager::CreateCamera()
	{
		GameObject newCam = Core::GetInstance()->mFactory.CreateEO();
		
		//auto& transform = newCam.GetComponent<Transform>();
		newCam.AddComponent<Camera>();
		//newCam.GetComponent<Camera>().renderTag = DEBUG_OBJ_TAG | DEBUG_GRID_TAG;

		return newCam;
	}
	GameObject RenderManager::CreatePrefabCam()
	{
		GameObject newCam = Core::GetInstance()->mFactory.CreateEO();
		
		//auto& transform = newCam.GetComponent<Transform>();
		newCam.AddComponent<Camera>();
		newCam.AddComponent<PrefabCameraEntity>();
		newCam.GetComponent<Camera>().debugRenderToggles = DEBUG_ALL_DEBUG;

		return newCam;
	}
	// MAYDO: has issue when deleting the cam game object, causing the mainCam to become Empty
	void RenderManager::SetMainGameCamera(Entity cam)
	{
		auto& camSys = Core::GetInstance()->GetSystem<CameraSystem>();

		std::optional<Entity> camEntity = camSys.GetCamera(cam);

		if (camEntity.has_value())
		{
			if (camSys.mainCam.has_value())
				Core::GetInstance()->GetRegistry().get<Camera>(camSys.mainCam.value()).isMainCamera = false;

			camSys.mainCam.emplace(cam);

			// Sync exposure
			auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
			camera.gamma = mSessionGamma;
			camera.isMainCamera = true;
		}
		else
			SLICE_LOG_ERROR("Setting to a non camera entity");
	}

	float RenderManager::GetSessionExposure() const { return mSessionExposure; }
	float RenderManager::GetSessionGamma() const { return mSessionGamma / 10.f; }

	void RenderManager::SetSessionExposure(float exposure)
	{
		mSessionExposure = exposure;
		auto& mainCam = GetGameCamera();
		if (mainCam.has_value())
		{
			auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(mainCam.value());
			camera.exposure = exposure;
		}
	}
	void RenderManager::SetSessionGamma(float gamma)
	{
		float modGamma = gamma * 10.f;
		mSessionGamma = modGamma;
		auto& mainCam = GetGameCamera();
		if (mainCam.has_value())
		{
			auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(mainCam.value());
			camera.gamma = modGamma;
		}
	}

	std::optional<Entity>& RenderManager::GetGameCamera()
	{
		return Core::GetInstance()->GetSystem<CameraSystem>().mainCam;
	}
	void RenderManager::CopyMainCamSettings(Camera& othCam)
	{
		auto currCam = GetGameCamera();
		if (currCam.has_value())
		{
			auto& actlCam = Core::GetInstance()->GetRegistry().get<Camera>(currCam.value());
			
			othCam.pov = actlCam.pov;
			othCam.near = actlCam.near;
			othCam.far = actlCam.far;
			othCam.luminanceLearningRate = actlCam.luminanceLearningRate;
			othCam.fogColor = actlCam.fogColor;
			othCam.fogIntensity = actlCam.fogIntensity;
			othCam.bloomFilterRadius = actlCam.bloomFilterRadius;
			othCam.bloomStrength = actlCam.bloomStrength;
			othCam.bloomLimit = actlCam.bloomLimit;
			othCam.exposure = actlCam.exposure;
			othCam.gamma = actlCam.gamma;
			othCam.whiteBalance = actlCam.whiteBalance;
			othCam.minLuminance = actlCam.minLuminance;
			othCam.maxLuminance = actlCam.maxLuminance;
			othCam.godRayFilterRadius = actlCam.godRayFilterRadius;
			othCam.godRayStrength = actlCam.godRayStrength;
			othCam.vignetteCenter = actlCam.vignetteCenter;
			othCam.vignetteIntensity = actlCam.vignetteIntensity;
			othCam.vignetteSmoothness = actlCam.vignetteSmoothness;
			othCam.impactPos = actlCam.impactPos;
			othCam.impactColor = actlCam.impactColor;
			othCam.impactColor2 = actlCam.impactColor2;
			othCam.impactSmooth = actlCam.impactSmooth;
			othCam.impactEpilepsy = actlCam.impactEpilepsy;
			othCam.impactAngle = actlCam.impactAngle;
			othCam.impactNoise1 = actlCam.impactNoise1;
			othCam.impactNoise2 = actlCam.impactNoise2;
			othCam.impactBlend = actlCam.impactBlend;
			othCam.cloudsHeight = actlCam.cloudsHeight;
			othCam.cloudsAmplitude = actlCam.cloudsAmplitude;
			othCam.cloudsIntensity = actlCam.cloudsIntensity;
			othCam.cloudsSmoothness = actlCam.cloudsSmoothness;
			othCam.cloudsCutoff = actlCam.cloudsCutoff;
			othCam.cloudsColor = actlCam.cloudsColor;
			othCam.cloudsSecondCloudOffset = actlCam.cloudsSecondCloudOffset;
			othCam.cloudsSecondCloudAmplitude = actlCam.cloudsSecondCloudAmplitude;
			othCam.cloudsSecondCloudIntensity = actlCam.cloudsSecondCloudIntensity;
			othCam.cloudsSecondCloudSmoothness = actlCam.cloudsSecondCloudSmoothness;
			othCam.cloudsSecondColor = actlCam.cloudsSecondColor;
			othCam.postRenderToggles = actlCam.postRenderToggles;
		}
	}
	void RenderManager::GetCameraAxis(GameObject& cam, glm::vec3& forward, glm::vec3& right, glm::vec3& up)
	{
		auto& camTrans = cam.GetComponent<Transform>();
		glm::mat3 rot = glm::mat3_cast(camTrans.rotation);
		//glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));
		GetCameraAxis(rot, forward, right, up);
	}
	void RenderManager::GetCameraAxis(glm::mat3& rot, glm::vec3& forward, glm::vec3& right, glm::vec3& up)
	{
		glm::vec3 f{ 1.f, 0.f, 0.f }, u{ 0.f, 1.f, 0.f }, r{ 0.f,0.f,1.f };

		forward = rot * f;
		right = rot * r;
		up = rot * u;
	}
	
	glm::mat4 RenderManager::DirLightMatCalc(const glm::mat4& proj, const glm::mat4& view, const glm::vec3 lightDir)
	{
		const auto inv = glm::inverse(proj * view);
		std::vector<glm::vec4> coners;
		coners.reserve(8);
		for (unsigned int x = 0; x < 2; ++x)
			for (unsigned int y = 0; y < 2; ++y)
				for (unsigned int z = 0; z < 2; ++z)
				{
					const glm::vec4 pt = inv * glm::vec4(2.f * static_cast<float>(x) - 1.f, 2.f * static_cast<float>(y) - 1.f, 2.f * static_cast<float>(z) - 1.f, 1.f);
					coners.emplace_back(pt / pt.w);
				}
		glm::vec3 center{};
		for (auto& v : coners)
			center += glm::vec3(v);
		center /= 8.f;

		glm::vec3 up{ 0.f, 1.f, 0.f };
		if (fabsf(glm::dot(up, lightDir)) > 0.999999f)
			up = glm::vec3(0.f, 0.f, 1.f);

		const glm::mat4 lightView = glm::lookAt(center, center + lightDir, up);

		float minX{ std::numeric_limits<float>::max() }, minY{ std::numeric_limits<float>::max() }, minZ{ std::numeric_limits<float>::max() };
		float maxX{ std::numeric_limits<float>::lowest() }, maxY{ std::numeric_limits<float>::lowest() }, maxZ{ std::numeric_limits<float>::lowest() };
		for (const auto& v : coners)
		{
			const auto trf = lightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		//float worldUnitsPerTexel = (maxX - minX) / Core::GetInstance()->GetSystem<LightingSystem>().SHADOW_DIMENSION;
		//minX = floor(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
		//maxX = floor(maxX / worldUnitsPerTexel) * worldUnitsPerTexel;
		//
		//float worldUnitsPerTexelY = (maxY - minY) / Core::GetInstance()->GetSystem<LightingSystem>().SHADOW_DIMENSION;
		//minY = floor(minY / worldUnitsPerTexelY) * worldUnitsPerTexelY;
		//maxY = floor(maxY / worldUnitsPerTexelY) * worldUnitsPerTexelY;

		//if (minZ < 0)
		//	minZ *= mLightZDist;
		//else
		//	minZ /= mLightZDist;
		//if (maxZ < 0)
		//	maxZ /= mLightZDist;
		//else
		//	maxZ *= mLightZDist;

		minZ -= mZBufferShadow * 1.5f;
		maxZ += mZBufferShadow * 0.5f;

		return glm::ortho(minX, maxX, minY, maxY, minZ, maxZ) * lightView;
	}
#pragma endregion

#pragma region Render
	void RenderManager::Update(float dt)
	{
		renderQueue.Update(dt);
		mTime += dt;
		if (mTime > 36000.f)
			mTime -= 36000.f;
	}

	void RenderManager::Render()
	{
		ForceResetDefaultSettings();
		renderQueue.GatherDrawCalls();
		
		if (skyboxData.isDirty)
		{
			RegenerateSkybox();
			skyboxData.isDirty = false;
		}

		IDPick();
		GatherLights();

		auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>(entt::exclude<InactiveEntity>);
		for (auto cam : cams)
		{
			auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
			if (!camera.componentEnabled) continue;
			bool isPrefabCam = Core::GetInstance()->GetRegistry().any_of<PrefabCameraEntity>(cam);

			mCurrFinalColAttachment = GOUT_FINAL;

			renderQueue.SortTranslucent(cam);
			CalculateVP(cam);
			GatherNearbyLights();

			// --------------------- Draw Calls From Here on ---------------------

			// Just clear for godray only (needs to be done after each cam :shrug
			LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[GOUT_GODRAY]);
			ClearBuffer(BufferClearSetting::COLOR_ONLY);

			// Reload Shadows
			RenderPerspectiveShadowMaps();
			RenderDirectionalShadowMaps(cam);

			SetShader(ShaderPaths[S_SKYBOX]);
			LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[GOUT_DIF]);
			LoadSettings(GPS_SKYBOX);
			UpdateCamVP();
			BindCameraDepth(cam);
			ClearBuffer(BufferClearSetting::COLOR_ONLY);
			RenderSkybox();

			//SetShader(ShaderPaths[S_DEFERRED]);
			if(cam == mCurrentCamIDHover)
				LinkFrameBufferSettings(FB_DEFERRED, 6, 0, mColAttachment[GOUT_ID], mColAttachment[GOUT_POS], mColAttachment[GOUT_NOM], mColAttachment[GOUT_ROUGH_METAL], mColAttachment[GOUT_EMISSION]);
			else
				LinkFrameBufferSettings(FB_DEFERRED, 6, 0, 0, mColAttachment[GOUT_POS], mColAttachment[GOUT_NOM], mColAttachment[GOUT_ROUGH_METAL], mColAttachment[GOUT_EMISSION]);
			LoadSettings(GPS_DEFAULT);
			//UpdateCamVP();
			BindCameraDepth(cam);
			ClearBuffer(BufferClearSetting::ALL);// Only one to do this, cuz dw reset
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColAttachment[GOUT_DIF], 0);
			renderQueue.UseDrawCalls(mCurrShader.second, isPrefabCam ? RenderCmdManager::DrawType::DRAW_PREFAB_OPAQUE : RenderCmdManager::DrawType::DRAW_OPAQUE, cameraPos);
			CheckGLError();
			//----------------------------------------------------------------
			SetShader(ShaderPaths[S_SKYBOX_Light]);
			LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
			LoadSettings(GPS_SKYBOX_AMBIENT);
			RenderSkyboxLighting(cam);

			SetShader(ShaderPaths[S_LIGHTING]);
			LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
			UpdateCamVP();
			BindCameraDepth(cam);
			RenderLighting(cam);
			//----------------------------------------------------------------
			LoadSettings(GPS_DEFAULT); // Does the ID part first
			BindCameraDepth(cam);
			if (cam == mCurrentCamIDHover) // Don't Draw into ID when drawing debugging onwards
			{
				//LinkFrameBufferSettings(FB_DEFERRED, 6, 0, mColAttachment[GOUT_ID], 0, 0, 0, 0);
				LinkFrameBufferSettings(FB_THREE, 3, 0, mColAttachment[GOUT_ID], 0);
				glDepthMask(GL_FALSE);
				renderQueue.UseDrawCalls(0, isPrefabCam ? RenderCmdManager::DrawType::DRAW_PREFAB_TRANSLUCENT_ID_ONLY : RenderCmdManager::DrawType::DRAW_TRANSLUCENT_ID_ONLY, cameraPos);
				CheckGLError();
			}
			LoadSettings(GPS_TEST_TRANSLUCENT); // Does the ID part first
			// When Drawing the Transparent part also writes into depth buffer
			//LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
			LinkFrameBufferSettings(FB_THREE, 3, mColAttachment[mCurrFinalColAttachment], 0, mColAttachment[GOUT_EMISSION]);
			//UpdateCamVP();
			glDepthMask(GL_TRUE);
			renderQueue.UseDrawCalls(0, isPrefabCam ? RenderCmdManager::DrawType::DRAW_PREFAB_TRANSLUCENT : RenderCmdManager::DrawType::DRAW_TRANSLUCENT, cameraPos);
			CheckGLError();

			// Special case for Post Processings
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).postRenderToggles & RENDER_GROUND_CLOUD)
				RenderGroundCloud(cam);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).postRenderToggles & RENDER_FOG)
				RenderFog(cam);

			RenderAvgLum(cam);
			//----------------------------------------------------------------
			// Post Processings - idk
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).postRenderToggles & RENDER_BLOOM)
				RenderBloom(cam, false);
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).postRenderToggles & RENDER_GODRAY)
				RenderBloom(cam, true);

			// Debug / QOL Stuffs
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).debugRenderToggles & DEBUG_ALL_DEBUG)
			{
				LoadSettings(GPS_DEBUG);
				LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
				RenderDebug(cam);
			}

			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).postRenderToggles & RENDER_VIGNETTE)
				RenderVignette(cam);
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).postRenderToggles & RENDER_IMPACT)
				RenderImpact(cam);



			RenderGammaCorrection(cam);
		}

		mDebugDrawRays.clear();
		
		mObjPickedThisFrame = false;
		LinkFrameBufferSettings(FB_TOTAL, 0);
		std::swap(pboIdx[0], pboIdx[1]);
	}
	void RenderManager::RenderDebug(Entity cam)
	{
		bool prefabCam = Core::GetInstance()->GetRegistry().any_of<PrefabCameraEntity>(cam);

		// Draw other cameras' frustrum
		if(!prefabCam && Core::GetInstance()->GetRegistry().get<Camera>(cam).debugRenderToggles & DEBUG_FRUSTRUM_TAG)
		{
			SetShader(ShaderPaths[S_INSTANCED]);
			UpdateCamVP();
			BindCameraDepth(cam);

			auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::FRUSTRUM_DEFAULT).get();
			auto& mdl = model.meshes[0];
			glBindVertexArray(mdl.vao);

			//auto& frustrum = Core::GetInstance()->GetResourceManager()->GetModel("FrustrumFake");
			auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>(entt::exclude<InactiveEntity>);
			int count{};
			for (auto& entity : cams)
			{
				if (entity == cam) continue;
				if (Core::GetInstance()->GetRegistry().any_of<PrefabCameraEntity>(entity)) continue;

				glm::mat4 mdlMtx = Core::GetInstance()->GetRegistry().get<Transform>(entity).transform;
				auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(entity);

				ShiftTransformMtx(mdlMtx, -cameraPos);

				renderQueue.mBasicIMtx[count].mdlMtx = mdlMtx *
					glm::rotate(glm::mat4(1.0f), -PI05F, glm::vec3(0.f, 1.f, 0.f)) *
					glm::inverse(glm::perspective(glm::radians(camera.pov), static_cast<float>(camera.width) / static_cast<float>(camera.height), camera.near, camera.far)) *
					glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
				if (++count > renderQueue.mMaxInstance)
					break;
			}
			glNamedBufferSubData(renderQueue.mIVBO, 0, sizeof(RenderCmdManager::BasicIDat) * count, renderQueue.mBasicIMtx.data());
			glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, count);
			CheckGLError();
		}
		
		// Draw Instance Debug Box (Physics)
		if (Core::GetInstance()->GetRegistry().get<Camera>(cam).debugRenderToggles & DEBUG_OBJ_TAG)
		{
			SetShader(ShaderPaths[S_INSTANCED]);
			UpdateCamVP();
			BindCameraDepth(cam);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


			std::unordered_map<GUID, std::vector<Entity>> debugShapes{};

			auto view = Core::GetInstance()->GetRegistry().view<PhysicEntity>(entt::exclude<InactiveEntity>); //renderEntity
			for (auto entity : view)
			{
				auto& shape = Core::GetInstance()->mFactory.mRegistry.get<ColliderShape>(entity);
				
				if (std::holds_alternative<ColliderShape::BoxData>(shape.shapeData))
				{
					debugShapes[(GUID)DefaultResourceIDs::CUBE_DEFAULT].push_back(entity);
				}
				if (std::holds_alternative<ColliderShape::SphereData>(shape.shapeData))
				{
					debugShapes[(GUID)DefaultResourceIDs::SPHERE_DEFAULT].push_back(entity);
				}
				if (std::holds_alternative<ColliderShape::CapsuleData>(shape.shapeData))
				{
					debugShapes[(GUID)DefaultResourceIDs::CAPSULE_DEFAULT].push_back(entity);
				}
				if (std::holds_alternative<ColliderShape::CylinderData>(shape.shapeData))
				{
					debugShapes[(GUID)DefaultResourceIDs::CYLINDER_DEFAULT].push_back(entity);
				}
				if (std::holds_alternative<ColliderShape::MeshData>(shape.shapeData) && Core::GetInstance()->mFactory.mRegistry.any_of<Renderer>(entity))
				{
					GUID guid = Core::GetInstance()->mFactory.mRegistry.get<Renderer>(entity).modelHandle.getGUID();
					debugShapes[guid].push_back(entity);
				}
			}


			for (auto& [modelID, entities] : debugShapes)
			{
				auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>(modelID).get();
				auto& mdl = model.meshes[0];	//i call it mdl cuz im lazy to change the below
				glBindVertexArray(mdl.vao);

				int num{};
				for (auto entity : entities)
				{
					auto& transform = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity);
					auto& shape = Core::GetInstance()->mFactory.mRegistry.get<ColliderShape>(entity);

					if (!shape.componentEnabled) // if not enabled should not draw
						continue;
					if (prefabCam != Core::GetInstance()->mFactory.mRegistry.any_of<PrefabEditingEntity>(entity))
						continue;

					if (std::holds_alternative<ColliderShape::BoxData>(shape.shapeData))
					{
						auto& boxData = std::get<ColliderShape::BoxData>(shape.shapeData);
						renderQueue.mBasicIMtx[num].mdlMtx = glm::scale(glm::translate(transform.transform, glm::vec3(shape.offSet.GetX(),shape.offSet.GetY(),shape.offSet.GetZ())), glm::vec3(boxData.scale.GetX() * 2.f, boxData.scale.GetY() * 2.f, boxData.scale.GetZ() * 2.f));
					}
					if (std::holds_alternative<ColliderShape::SphereData>(shape.shapeData))
					{
						auto& sphereData = std::get<ColliderShape::SphereData>(shape.shapeData);
						renderQueue.mBasicIMtx[num].mdlMtx = glm::scale(glm::translate(transform.transform, glm::vec3(shape.offSet.GetX(), shape.offSet.GetY(), shape.offSet.GetZ())), glm::vec3(sphereData.radius * 2.f));
					}
					if (std::holds_alternative<ColliderShape::CapsuleData>(shape.shapeData))
					{
						auto& capsuleData = std::get<ColliderShape::CapsuleData>(shape.shapeData);
						renderQueue.mBasicIMtx[num].mdlMtx = glm::scale(glm::translate(transform.transform, glm::vec3(shape.offSet.GetX(), shape.offSet.GetY(), shape.offSet.GetZ())), glm::vec3(capsuleData.radius * 2.f, capsuleData.height * 2.f, capsuleData.radius * 2.f));
					}
					if (std::holds_alternative<ColliderShape::CylinderData>(shape.shapeData))
					{
						auto& cylinderData = std::get<ColliderShape::CylinderData>(shape.shapeData);
						renderQueue.mBasicIMtx[num].mdlMtx = glm::scale(glm::translate(transform.transform, glm::vec3(shape.offSet.GetX(), shape.offSet.GetY(), shape.offSet.GetZ())), glm::vec3(cylinderData.radius * 2.f, cylinderData.height * 2.f, cylinderData.radius * 2.f));
					}
					if (std::holds_alternative<ColliderShape::MeshData>(shape.shapeData)) // already passed the has Renderer Check
					{
						renderQueue.mBasicIMtx[num].mdlMtx = transform.transform;
					}
					ShiftTransformMtx(renderQueue.mBasicIMtx[num].mdlMtx, -cameraPos);

					num++;
					if (num == renderQueue.mMaxInstance)
					{
						glNamedBufferSubData(renderQueue.mIVBO, 0, sizeof(RenderCmdManager::BasicIDat) * num, renderQueue.mBasicIMtx.data());
						glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, num);
						num = 0;
					}
				}
				if (num != 0)
				{
					glNamedBufferSubData(renderQueue.mIVBO, 0, sizeof(RenderCmdManager::BasicIDat)* num, renderQueue.mBasicIMtx.data());
					glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, num);
				}
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			CheckGLError();
		}


		// Draw Debug Grid Lines
		if(Core::GetInstance()->GetRegistry().get<Camera>(cam).debugRenderToggles & DEBUG_GRID_TAG)
		{
			SetShader(ShaderPaths[S_DEBUG_LINE]);
			ForceCamNormalVP(cam);
			BindCameraDepth(cam);
			GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uCamPos");
			SetUniformVec3(uniformLoc, cameraPos);

			glDrawArrays(GL_TRIANGLES, 0, 6);
			CheckGLError();
		}
		// Draw Debug Outline
		if (Core::GetInstance()->GetRegistry().get<Camera>(cam).debugRenderToggles & DEBUG_OUTLINE_SELECTED_TAG)
		{
			SetShader(ShaderPaths[S_DEBUG_OUTLINE]);
			LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[GOUT_DEBUG_OUTLINE]);
			ClearBuffer(BufferClearSetting::COLOR_ONLY);
			{
				glm::mat4 PV = P * V;
				GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uPV");
				glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &PV[0][0]);
			}
			BindCameraDepth(cam); // for the viewPort call
			LoadSettings(GPS_BLOOM);

			auto view = Core::GetInstance()->GetRegistry().view<SelectedEntity>(); // renderEntity
			for (auto entity : view)
			{
				auto entityGO = Core::GetInstance()->mFactory.GetGOByEntity(entity);
				if (!entityGO.HasComponent<Renderer>() || !entityGO.HasComponent<Transform>())
					continue;
				if (prefabCam != Core::GetInstance()->mFactory.mRegistry.any_of<PrefabEditingEntity>(entity))
					continue;
				renderQueue.SingleDraw(mCurrShader.second, entity, RenderCmdManager::DrawType::DRAW_MODELS, cameraPos);
			}
			// "Blur" Passes
			SetShader(ShaderPaths[S_DEBUG_OUT_BLUR]);
			LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[GOUT_DEBUG_OUTLINE_BLURED]); // Out
			ClearBuffer(BufferClearSetting::COLOR_ONLY);
			LoadSettings(GPS_BLOOM2); // Add
			glBindTextureUnit(0, mColAttachment[GOUT_DEBUG_OUTLINE]); // In
			//LoadSettings(); // Same Settings
			glDrawArrays(GL_TRIANGLES, 0, 6);

			// Final Draw Back on texture
			SetShader(ShaderPaths[S_DEBUG_OUTLJOIN]);
			LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
			BindCameraDepth(cam); // for the viewPort call
			LoadSettings(GPS_DEBUG_OUTLINE_BLEND); // Blend?
			glBindTextureUnit(0, mColAttachment[GOUT_DEBUG_OUTLINE_BLURED]); // In
			GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uCol");
			SetUniformVec3(uniformLoc, glm::vec3(1.f));

			glDrawArrays(GL_TRIANGLES, 0, 6);
			CheckGLError();
		}
		// Draw Debug Rays idfk (lines ig)
		if (Core::GetInstance()->GetRegistry().get<Camera>(cam).debugRenderToggles & DEBUG_DRAW_RAY_TAG)
		{
			SetShader(ShaderPaths[S_INSTANCED]);
			ForceCamNormalVP(cam);
			BindCameraDepth(cam);

			auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::CUBE_DEFAULT).get();
			auto& mdl = model.meshes[0];
			glBindVertexArray(mdl.vao);

			int count{};
			for (auto& i : mDebugDrawRays)
			{
				renderQueue.mBasicIMtx[count].mdlMtx = i;
				if (++count > renderQueue.mMaxInstance)
					break;
			}
			glNamedBufferSubData(renderQueue.mIVBO, 0, sizeof(RenderCmdManager::BasicIDat)* count, renderQueue.mBasicIMtx.data());
			glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, count);
			CheckGLError();
		}
	}
	void RenderManager::RenderPerspectiveShadowMaps()
	{
		SetShader(ShaderPaths[S_POINT_SHADOW]);
		LinkFrameBufferSettings(FB_NIL, 0);
		LoadSettings(GPS_SHADOW);
		glViewport(0, 0, SHADOW_DIMENSION, SHADOW_DIMENSION);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mShadowCubeMapArr, 0);
		//ClearBuffer(BufferClearSetting::ALL);
		float depthClearVal = 1.0f;
		CheckGLError();

		glm::vec3 eye{};
		for (auto it : dirtyShadows)
		{
			LightDat& light = allLightData[it];
			if (light.type != Light::Light_Point)
				continue;

			glClearTexSubImage(mShadowCubeMapArr, 0, 0, 0, light.shadowNum * 6,
				SHADOW_DIMENSION, SHADOW_DIMENSION, 6, GL_DEPTH_COMPONENT, GL_FLOAT, &depthClearVal);

			GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uFarPlane");
			glUniform1f(uniformLoc, light.uFarPlane);
			uniformLoc = glGetUniformLocation(mCurrShader.second, "uLightIdx");
			glUniform1i(uniformLoc, light.shadowNum);

			glm::mat4 lightP = glm::perspective(PI05F, 1.f, 0.01f, light.uFarPlane);
			std::stringstream ss{};
			for (size_t i{}; i < 6; ++i)
			{
				glm::mat4 shadowMat{ lightP * glm::lookAt(eye, mShadowCamDir[i].target, mShadowCamDir[i].up) };
				ss.str("");
				ss << "uShadowMat[" << std::to_string(i) << "]";
				uniformLoc = glGetUniformLocation(mCurrShader.second, ss.str().c_str());
				glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &shadowMat[0][0]);
			}

			renderQueue.UseDrawCalls(mCurrShader.second, RenderCmdManager::DrawType::DRAW_MODELS, light.pos);
		}
		CheckGLError();

		SetShader(ShaderPaths[S_SPOT_SHADOW]);
		for (auto it : dirtyShadows)
		{
			LightDat& light = allLightData[it];
			if (light.type != Light::Light_Spot)
				continue;

			int shadowNum = light.shadowNum * 6 + light.spotShadowNum;

			glClearTexSubImage(mShadowCubeMapArr, 0, 0, 0, shadowNum,
				SHADOW_DIMENSION, SHADOW_DIMENSION, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthClearVal);

			GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uFarPlane");
			glUniform1f(uniformLoc, light.uFarPlane);
			uniformLoc = glGetUniformLocation(mCurrShader.second, "uLightIdx");
			glUniform1i(uniformLoc, shadowNum);

			uniformLoc = glGetUniformLocation(mCurrShader.second, "uVP");
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &light.pointlightMtx[0][0]);

			renderQueue.UseDrawCalls(mCurrShader.second, RenderCmdManager::DrawType::DRAW_MODELS, light.pos);
		}

		dirtyShadows.clear();
		CheckGLError();
	}
	void RenderManager::RenderDirectionalShadowMaps(Entity cam)
	{
		if (!mDirLightFound)
			return;
		SetShader(ShaderPaths[S_SHADOW]);
		LinkFrameBufferSettings(FB_NIL, 0);
		LoadSettings(GPS_SHADOW);
		glViewport(0, 0, DIRECTIONAL_SHADOW_DIMENSION, DIRECTIONAL_SHADOW_DIMENSION);

		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		const float ar = static_cast<float>(camera.width) / static_cast<float>(camera.height);

		std::vector<glm::mat4> lightSpaceMtx;
		for (int i = 0; i < mNumCascadeShadow; ++i)
		{
			float near{}, far{};
			if (i == 0)
			{
				near = camera.near;
				far = camera.far / shadowCascadeLevels[0]; // 200.f / 50.f = 4.f
			}
			else if (i == mNumCascadeShadow - 1)
			{
				near = camera.far / shadowCascadeLevels[i - 1];
				far = camera.far;
			}
			else
			{
				near = camera.far / shadowCascadeLevels[i - 1];
				far = camera.far / shadowCascadeLevels[i];
			}
			const auto camProj = glm::perspective(glm::radians(camera.pov), ar, near, far);
			lightSpaceMtx.push_back(DirLightMatCalc(camProj, V, dirLightDat.dir));
		}
		glNamedBufferSubData(mShadowUBO, 0, lightSpaceMtx.size() * sizeof(glm::mat4), lightSpaceMtx.data());

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDirLightDepthMaps, 0);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderQueue.UseDrawCalls(mCurrShader.second, RenderCmdManager::DrawType::DRAW_MODELS, cameraPos);

		CheckGLError();
	}
	void RenderManager::RenderSkybox()
	{
		//glBindTextureUnit(0, SkyboxMap);
		glBindTextureUnit(0, SkyboxMap);

		auto& mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::CUBE_DEFAULT).get()->meshes[0];
		glBindVertexArray(mdl.vao);
		glDrawElements(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr);
		CheckGLError();
	}
	void RenderManager::RenderSkyboxLighting(Entity cam)
	{
		glBindTextureUnit(0, mColAttachment[GOUT_DIF]);
		glBindTextureUnit(1, mColAttachment[GOUT_NOM]);
		glBindTextureUnit(2, SkyboxIrradianceMap);
		glBindTextureUnit(3, mColAttachment[GOUT_EMISSION]);
		glBindTextureUnit(4, mColAttachment[GOUT_ROUGH_METAL]);

		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "skyboxLightingPower");
		glUniform1f(uniformLoc, skyboxData.lightingPower / 100.f);
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "willBloom");
		glUniform1i(uniformLoc, static_cast<GLint>(camera.postRenderToggles & RENDER_BLOOM));

		glDrawArrays(GL_TRIANGLES, 0, 6);
		CheckGLError();
	}
	void RenderManager::RenderLighting(Entity cam)
	{
		glBindTextureUnit(0, mColAttachment[GOUT_DIF]);
		glBindTextureUnit(1, mColAttachment[GOUT_POS]);
		glBindTextureUnit(2, mColAttachment[GOUT_NOM]);
		glBindTextureUnit(3, mColAttachment[GOUT_ROUGH_METAL]);
		glBindTextureUnit(4, mDirLightDepthMaps);
		glBindTextureUnit(5, mShadowCubeMapArr);
		glBindTextureUnit(6, mColAttachment[GOUT_EMISSION]);
		LoadSettings(GPS_ADDITION);

		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		GLint uniformLoc;

		glNamedBufferSubData(mLightUBO, 0, sizeof(LightDat), &dirLightDat);
		glNamedBufferSubData(mLightUBO, sizeof(LightDat), std::min(allLightData.size(), mMaxLights) * sizeof(LightDat), allLightData.data());
		CheckGLError();

		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCamPos");
		glUniform3f(uniformLoc, cameraPos.x, cameraPos.y, cameraPos.z);

		auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT);
		auto& mesh = mdl.get()->meshes[0];
		mainDirLightFar = 0.f;

		if (mDirLightFound)
		{
			mainDirLightFar = camera.far;
			uniformLoc = glGetUniformLocation(mCurrShader.second, "lightIdx");
			glUniform1i(uniformLoc, 200);
			uniformLoc = glGetUniformLocation(mCurrShader.second, "cascadeCnt");
			glUniform1i(uniformLoc, mNumCascadeShadow);
			std::stringstream ss{};
			for (int i = 0; i < mNumCascadeShadow; ++i)
			{
				ss.str("");
				ss << "cascadePlaneDist[" << std::to_string(i) << "]";
				uniformLoc = glGetUniformLocation(mCurrShader.second, ss.str().c_str());
				if (i == mNumCascadeShadow - 1)
					glUniform1f(uniformLoc, camera.far);
				else
					glUniform1f(uniformLoc, camera.far / shadowCascadeLevels[i]);
			}
			glBindVertexArray(mesh.vao);
			glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);

		}

		for (size_t i{}; i < allLightData.size(); ++i)
		{
			auto& light = allLightData.at(i);

			uniformLoc = glGetUniformLocation(mCurrShader.second, "lightIdx");
			glUniform1i(uniformLoc, i);

			glBindVertexArray(mesh.vao);
			glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
		}
		
		CheckGLError();
	}
	void RenderManager::RenderAvgLum(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);

		// Extract Luminiance before downscaling
		LoadSettings(GPS_DEFAULT);
		SetShader(ShaderPaths[S_EXT_LUMINANCE]);
		LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[GOUT_LUM_EXTRACT]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
		ClearBuffer(BufferClearSetting::COLOR_ONLY);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		CheckGLError();

		// Luminance Calc
		glGenerateTextureMipmap(mColAttachment[GOUT_LUM_EXTRACT]);
		SetShader(ShaderPaths[S_LUMINANCE]);
		LinkFrameBufferSettings(FB_FINAL, 1, camera.lum[static_cast<int>(camera.lumSelected)]);
		ClearBuffer(BufferClearSetting::COLOR_ONLY);
		glBindTextureUnit(0, mColAttachment[GOUT_LUM_EXTRACT]);
		glBindTextureUnit(1, camera.lum[static_cast<int>(!camera.lumSelected)]);

		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uLearningRate");
		if(!camera.camLoaded)
		{
			glUniform1f(uniformLoc, 1000);
			camera.camLoaded = true;
		}
		else
			glUniform1f(uniformLoc, camera.luminanceLearningRate);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uMaxLum");
		glUniform1f(uniformLoc, camera.maxLuminance);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uMinLum");
		glUniform1f(uniformLoc, camera.minLuminance);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		CheckGLError();

	}
	void RenderManager::RenderGroundCloud(Entity cam)
	{
		SetShader(ShaderPaths[S_CLOUDS]);
		LoadSettings(GPS_TEST_TRANSLUCENT);
		ForceCamNormalVP(cam);
		BindCameraDepth(cam);
		glDepthMask(GL_FALSE);
		LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);

		auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::PLANE_DEFAULT).get();
		auto& mdl = model.meshes[0];
		glBindVertexArray(mdl.vao);

		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);

		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uTime");
		glUniform1f(uniformLoc, mTime);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCamPos");
		SetUniformVec3(uniformLoc, cameraPos);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "numLights");
		glUniform1i(uniformLoc, numLightsFound);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "hasDirectionalLight");
		glUniform1i(uniformLoc, mDirLightFound);

		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsAmplitude");
		glUniform1f(uniformLoc, camera.cloudsAmplitude);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsIntensity");
		glUniform1f(uniformLoc, camera.cloudsIntensity);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsSmoothness");
		glUniform1f(uniformLoc, camera.cloudsSmoothness);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsCutoff");
		glUniform1f(uniformLoc, camera.cloudsCutoff);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudOffset");
		glUniform3f(uniformLoc, 0.f, camera.cloudsHeight, 0.f);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsColor");
		glUniform4f(uniformLoc, camera.cloudsColor.r, camera.cloudsColor.g, camera.cloudsColor.b, camera.cloudsColor.a);

		uniformLoc = glGetUniformLocation(mCurrShader.second, "numLights");
		glUniform1i(uniformLoc, numLightsFound);

		uniformLoc = glGetUniformLocation(mCurrShader.second, "cascadeCnt");
		glUniform1i(uniformLoc, mNumCascadeShadow);
		std::stringstream ss{};
		for (int i = 0; i < mNumCascadeShadow; ++i)
		{
			ss.str("");
			ss << "cascadePlaneDist[" << std::to_string(i) << "]";
			uniformLoc = glGetUniformLocation(mCurrShader.second, ss.str().c_str());
			if (i == mNumCascadeShadow - 1)
				glUniform1f(uniformLoc, mainDirLightFar);
			else
				glUniform1f(uniformLoc, mainDirLightFar / Core::GetInstance()->GetRenderManager()->shadowCascadeLevels[i]);
		}

		glDrawElements(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr);

		// 2nd cloud
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudOffset");
		glUniform3f(uniformLoc, camera.cloudsSecondCloudOffset.x, camera.cloudsHeight + camera.cloudsSecondCloudOffset.y, camera.cloudsSecondCloudOffset.z);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsCutoff");
		glUniform1f(uniformLoc, -1.f);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsAmplitude");
		glUniform1f(uniformLoc, camera.cloudsSecondCloudAmplitude);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsIntensity");
		glUniform1f(uniformLoc, camera.cloudsSecondCloudIntensity);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsSmoothness");
		glUniform1f(uniformLoc, camera.cloudsSecondCloudSmoothness);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCloudsColor");
		glUniform4f(uniformLoc, camera.cloudsSecondColor.r, camera.cloudsSecondColor.g, camera.cloudsSecondColor.b, camera.cloudsSecondColor.a);

		glDrawElements(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr);

		glDepthMask(GL_TRUE);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
		CheckGLError();
	}
	void RenderManager::RenderFog(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		auto& camT = Core::GetInstance()->GetRegistry().get<Transform>(cam);

		SetShader(ShaderPaths[S_FOG]);
		LoadSettings(GPS_DEFAULT);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);
		glBindTextureUnit(1, mColAttachment[GOUT_POS]);
		glBindTextureUnit(2, mColAttachment[GOUT_NOM]);
		ToggleFinalTexture();
		LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
		ClearBuffer(BufferClearSetting::ALL);

		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uFogColor");
		SetUniformVec3(uniformLoc, camera.fogColor);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uFogIntensity");
		glUniform1f(uniformLoc, camera.fogIntensity);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		CheckGLError();
	}
	void RenderManager::RenderBloom(Entity cam, bool specifallyGodRay)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);

		// Extract the Bright
		SetShader(ShaderPaths[S_BLOOM_SPLIT]);
		LoadSettings(GPS_BLOOM);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);
		if(specifallyGodRay)
			glBindTextureUnit(1, mColAttachment[GOUT_GODRAY]);
		else
			glBindTextureUnit(1, mColAttachment[GOUT_EMISSION]);
		LinkFrameBufferSettings(FB_FINAL, 1, mBloomMips[0].tex);
		ClearBuffer(BufferClearSetting::COLOR_ONLY);

		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uLimit");
		glUniform1f(uniformLoc, camera.bloomLimit);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Downscaling
		SetShader(ShaderPaths[S_DOWNSCALING]);

		glBindTextureUnit(0, mBloomMips[0].tex);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uTexelSize");
		glUniform2f(uniformLoc, 1.f/mBloomMips[0].size.x, 1.f/mBloomMips[0].size.y);
		for (int i{1}; i < mMaxBloom; ++i)
		{
			LinkFrameBufferSettings(FB_FINAL, 1, mBloomMips[i].tex);
			ClearBuffer(BufferClearSetting::COLOR_ONLY);
			glViewport(0, 0, mBloomMips[i].intSize.x, mBloomMips[i].intSize.y);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glUniform2f(uniformLoc, 1.f/mBloomMips[i].size.x, 1.f/mBloomMips[i].size.y);
			glBindTextureUnit(0, mBloomMips[i].tex);
		}

		SetShader(ShaderPaths[S_UPSCALING]);
		LoadSettings(GPS_BLOOM);

		uniformLoc = glGetUniformLocation(mCurrShader.second, "uFilterRadius");
		if (specifallyGodRay)
			glUniform1f(uniformLoc, camera.godRayFilterRadius * mBloomFilterMult);
		else
			glUniform1f(uniformLoc, camera.bloomFilterRadius * mBloomFilterMult);

		for (int i{ mMaxBloom - 1 }; i > 0; --i)
		{
			glBindTextureUnit(0, mBloomMips[i].tex);
			glViewport(0, 0, mBloomMips[i-1].intSize.x, mBloomMips[i-1].intSize.y);
			LinkFrameBufferSettings(FB_FINAL, 1, mBloomMips[i-1].tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		SetShader(ShaderPaths[S_BLOOM_JOIN]);
		LoadSettings(GPS_DEFAULT);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);
		glBindTextureUnit(1, mBloomMips[0].tex);
		ToggleFinalTexture();
		LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
		ClearBuffer(BufferClearSetting::ALL);

		uniformLoc = glGetUniformLocation(mCurrShader.second, "uBloomStrength");
		if (specifallyGodRay)
			glUniform1f(uniformLoc, camera.godRayStrength * mBloomStrengthMult);
		else
			glUniform1f(uniformLoc, camera.bloomStrength * mBloomStrengthMult);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		CheckGLError();
	}
	void RenderManager::RenderVignette(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);

		SetShader(ShaderPaths[S_VIGNETTE]);
		LoadSettings(GPS_DEFAULT);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);
		ToggleFinalTexture();
		LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
		ClearBuffer(BufferClearSetting::ALL);

		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uVignetteCenter");
		glUniform2f(uniformLoc, camera.vignetteCenter.x, camera.vignetteCenter.y);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uVignetteIntensity");
		glUniform1f(uniformLoc, camera.vignetteIntensity);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uVignetteSmoothness");
		glUniform1f(uniformLoc, camera.vignetteSmoothness);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		CheckGLError();
	}
	void RenderManager::RenderImpact(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		auto& cameraT = Core::GetInstance()->GetRegistry().get<Transform>(cam);

		SetShader(ShaderPaths[S_IMPACT]);
		LoadSettings(GPS_DEFAULT);
		glBindTextureUnit(0, mColAttachment[GOUT_POS]);
		glBindTextureUnit(1, mColAttachment[GOUT_NOM]);
		LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[GOUT_IMPACT]);
		ClearBuffer(BufferClearSetting::ALL);

		glm::mat4 PV = P * V;
		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uVP");
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &PV[0][0]);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "time");
		glUniform1f(uniformLoc, mTime);

		uniformLoc = glGetUniformLocation(mCurrShader.second, "impactPos");
		glm::vec3 relImpactPos = camera.impactPos - cameraT.GetWorldPosition();
		glUniform3f(uniformLoc, relImpactPos.x, relImpactPos.y, relImpactPos.z);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "impactColor");
		glUniform3f(uniformLoc, camera.impactColor.x, camera.impactColor.y, camera.impactColor.z);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "impactColor2");
		glUniform3f(uniformLoc, camera.impactColor2.x, camera.impactColor2.y, camera.impactColor2.z);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "epilepsy");
		glUniform1f(uniformLoc, camera.impactEpilepsy);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "isSmooth");
		glUniform1i(uniformLoc, camera.impactSmooth);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "impactAngle");
		glUniform1f(uniformLoc, glm::radians(camera.impactEpilepsy));
		uniformLoc = glGetUniformLocation(mCurrShader.second, "noiseScale");
		glUniform1f(uniformLoc, camera.impactNoise1);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "secondNoiseScale");
		glUniform1f(uniformLoc, camera.impactNoise2);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		CheckGLError();
	}
	void RenderManager::RenderGammaCorrection(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);

		LoadSettings(GPS_DEFAULT);
		SetShader(ShaderPaths[S_FINAL]);
		LinkFrameBufferSettings(FB_FINAL, 1, Core::GetInstance()->GetRegistry().get<Camera>(cam).textureID);
		ClearBuffer(BufferClearSetting::ALL);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);
		glBindTextureUnit(1, camera.lum[static_cast<int>(camera.lumSelected)]);
		glBindTextureUnit(2, mColAttachment[GOUT_IMPACT]);

		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uExposure");
		glUniform1f(uniformLoc, camera.exposure * mExposureMult);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uGamma");
		glUniform1f(uniformLoc, camera.gamma / 100.f);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "White");
		glUniform1f(uniformLoc, camera.whiteBalance);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "impactBlend");
		if(Core::GetInstance()->GetRegistry().get<Camera>(cam).postRenderToggles & RENDER_IMPACT)
			glUniform1f(uniformLoc, camera.impactBlend);
		else
			glUniform1f(uniformLoc, 0.f);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		camera.lumSelected = !camera.lumSelected;
		CheckGLError();
	}
	void RenderManager::Draw()
	{
		LinkFrameBufferSettings(FB_TOTAL, 0);
		LoadSettings(GPS_DEFAULT);
		ClearBuffer(BufferClearSetting::ALL);
		if (GetGameCamera().has_value())
		{
			SetShader(ShaderPaths[S_COPY]);
			glBindTextureUnit(0, Core::GetInstance()->GetRegistry().get<Camera>(GetGameCamera().value()).textureID);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		CheckGLError();
	}
#pragma endregion

#pragma region Rendering Helpers
	// Only need to calculate for camera once
	void RenderManager::CalculateVP(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		auto& camTrans = Core::GetInstance()->GetRegistry().get<Transform>(cam);

		auto& worldTr = camTrans.transform;

		glm::vec3 camPosition{ 0.f, 0.f, 0.f }, target{1.f, 0.f, 0.f}, up{0.f, 1.f, 0.f};
		glm::mat3 rot = glm::mat3(worldTr);

		//glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));
		glm::vec3 forward = glm::normalize(rot * target);

		cameraPos = worldTr[3];

		V = glm::lookAt(camPosition, forward, up);
		camera.V = V;

		P = glm::perspective(glm::radians(camera.pov), static_cast<float>(camera.width) / static_cast<float>(camera.height), camera.near, camera.far);
		camera.P = P;
	}
	// Updates V P uniforms
	void RenderManager::UpdateCamVP()
	{
		GLint uniformLoc;
		if (UniformExists("V", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &V[0][0]);
		if (UniformExists("P", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &P[0][0]);
		CheckGLError();
	}
	void RenderManager::ForceCamNormalVP(Entity cam)
	{
		auto& camTrans = Core::GetInstance()->GetRegistry().get<Transform>(cam);
		glm::vec3 camPosition{ cameraPos }, target{1.f, 0.f, 0.f}, up{0.f, 1.f, 0.f};
		glm::mat3 rot = glm::mat3(camTrans.transform);
		glm::vec3 forward = glm::normalize(rot * target);

		glm::mat4 tempV = glm::lookAt(camPosition, camPosition + forward, up);
		GLint uniformLoc;
		if (UniformExists("V", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &tempV[0][0]);
		if (UniformExists("P", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &P[0][0]);
		CheckGLError();
	}
	// Binds Depth Texture from camera & ViewPort size
	void RenderManager::BindCameraDepth(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, camera.depthTex, 0);

		glViewport(0, 0, camera.width, camera.height);
		CheckGLError();
	}
	bool RenderManager::UniformExists(const char* str, GLint& ref)
	{
		ref = glGetUniformLocation(mCurrShader.second, str);
		if (ref >= 0)
			return true;

		//std::stringstream ss;
		//ss << "Uniform variable: " << str << " doesn't exist!!!\n";
		//SLICE_LOG_WARNING(ss.str());
		return false;
	}
	void RenderManager::GatherLights()
	{
		numLightsFound = 0;
		mDirLightFound = false;
		allLightData.clear();
		sortedLights.clear();
		activeShadowSet.clear();
		dirtyShadows.clear();

		auto view = Core::GetInstance()->GetRegistry().view<lightingEntity>(entt::exclude<InactiveEntity>);
		for (auto entity : view)
		{
			auto& light = Core::GetInstance()->GetRegistry().get<Light>(entity);
			if (!light.componentEnabled) continue;

			++numLightsFound;

			auto& transform = Core::GetInstance()->GetRegistry().get<Transform>(entity);

			if (light.type == Light::Light_Directional)
			{
				if (!mDirLightFound)
				{
					dirLightDat.hasShadow = light.castsShadow;
					dirLightDat.dir = -transform.GetWorldPosition();
					dirLightDat.type = static_cast<int>(light.type);
					dirLightDat.col = glm::vec4(light.color, light.intensity);
					mDirLightFound = true;
				}
				continue;
			}

			LightDat tempDat;
			tempDat.pos = transform.GetWorldPosition();
			tempDat.uFarPlane = CalcPointLightFar(transform.GetWorldScale(), light.intensity);
			tempDat.type = static_cast<int>(light.type);
			tempDat.col = glm::vec4(light.color, light.intensity);
			tempDat.hasShadow = 0;
			tempDat.pointAngle = glm::radians(light.angle);
			if (light.type == Light::Light_Spot)
			{
				glm::vec3 eye{};
				glm::mat3 rot = glm::mat3(transform.transform);
				tempDat.dir = rot * glm::vec3(0.f, -1.f, 0.f);

				glm::mat4 lightP = glm::perspective(tempDat.pointAngle, 1.f, 0.01f, tempDat.uFarPlane);
				glm::vec3 lightUp{ 0.f, 1.f, 0.f };
				if (abs(glm::dot(lightUp, tempDat.dir)) > 0.99999f)
					lightUp = glm::vec3(0.f, 0.f, 1.f);

				tempDat.pointlightMtx = lightP * glm::lookAt(eye, tempDat.dir, lightUp);
			}

			if(light.castsShadow)
				sortedLights.emplace_back(allLightData.size());
			allLightData.emplace_back(tempDat);
		}
	}
	void RenderManager::GatherNearbyLights()
	{
		// Sort - Small to big
		glm::vec3 camP = cameraPos;
		std::sort(sortedLights.begin(), sortedLights.end(), [&camP, this](const auto& a, const auto& b) {
			return glm::distance2(allLightData[a].pos, camP) < glm::distance2(allLightData[b].pos, camP);
		});
		// Gather first n Lights (Spot lights considered 1/6)
		std::unordered_set<size_t> newShadows{}; // --MAYDO-- Group the spot lights tgt lol ;w; 
		size_t numShadowCnt{}, emptySpotShadowNum{}, totalSpotsRequired{};

		for (auto& it : sortedLights) // it -> allLightData[it] accessor
		{
			if (allLightData[it].type == Light::Light_Spot)
			{
				if (emptySpotShadowNum == 0)
				{
					if (numShadowCnt >= mMaxPointLights)
						break;

					emptySpotShadowNum = 6;
					++numShadowCnt;
				}
				newShadows.insert(it);
				--emptySpotShadowNum;
				++totalSpotsRequired;
			}
			else if (allLightData[it].type == Light::Light_Point)
			{
				if (numShadowCnt >= mMaxPointLights)
					continue;
				// write
				newShadows.insert(it);
				++numShadowCnt;
			}
		}
		if (activeShadowSet.empty())
		{
			int numPointShadow{}, numSpotShadow{6}, lastSpotShadow{};
			for (auto& it : newShadows)
			{
				allLightData[it].hasShadow = 1;
				dirtyShadows.push_back(it);
				if (allLightData[it].type == Light::Light_Point)
				{
					allLightData[it].shadowNum = numPointShadow++;
				}
				else if (allLightData[it].type == Light::Light_Spot)
				{
					if (numSpotShadow == 6)
					{
						lastSpotShadow = numPointShadow++;
						numSpotShadow = 0;
					}
					allLightData[it].shadowNum = lastSpotShadow;
					allLightData[it].spotShadowNum = numSpotShadow++;
				}
			}
		}
		else
		{
			std::vector<std::pair<size_t, size_t>> openSpotSlots;// Slot num, sub slot num
			std::vector<size_t> openPointSlots;

			std::unordered_map<size_t, std::vector<size_t>> slotsTakenForSpotLights{};// slot Num, alive's ID

			// If currently active shadows not found in this iteration
			for (auto& it : activeShadowSet)
			{
				if (newShadows.find(it) == newShadows.end())
				{
					allLightData[it].hasShadow = 0;
					if (allLightData[it].type == Light::Light_Point)
						openPointSlots.push_back(allLightData[it].shadowNum);
					else if (allLightData[it].type == Light::Light_Spot)
					{
						openSpotSlots.push_back({ allLightData[it].shadowNum, allLightData[it].spotShadowNum });
						if (!slotsTakenForSpotLights.contains(allLightData[it].shadowNum))
							slotsTakenForSpotLights[allLightData[it].shadowNum];
					}
				}
				else if (allLightData[it].type == Light::Light_Spot)
				{
					slotsTakenForSpotLights[allLightData[it].shadowNum].push_back(it);
				}
			}

			// cleanup - Set the correct openSlots, add spot lights into newShadows if necessary

			// I need free more slots (Dun need care about less, cuz I can just use the WHOLE openSpotSlots cuz i need more anyways hahahs)
			while (totalSpotsRequired / 6 > slotsTakenForSpotLights.size())
			{
				auto newSpotLightSlot = openPointSlots.front();
				openPointSlots.erase(openPointSlots.begin());
				slotsTakenForSpotLights[newSpotLightSlot];
				for (size_t i{}; i < 6; ++i)
					openSpotSlots.push_back({ newSpotLightSlot, i });
			}
			// I need consolidate spot lights to take less cube space
			while (totalSpotsRequired / 6 < slotsTakenForSpotLights.size())
			{
				size_t leastFilledSlot{}, numInLeastFilledSlot{6};
				for (auto& i : slotsTakenForSpotLights)
				{
					if (i.second.size() < numInLeastFilledSlot)
					{
						leastFilledSlot = i.first;
						numInLeastFilledSlot = i.second.size();
					}
				}
				std::remove_if(openSpotSlots.begin(), openSpotSlots.end(), [&leastFilledSlot](const auto& a) { return a.first == leastFilledSlot; });
				for (auto& it : slotsTakenForSpotLights[leastFilledSlot])
					newShadows.insert(it);

				slotsTakenForSpotLights.erase(leastFilledSlot);
			}

			// If new shadows prev did not exist
			for (auto& it : newShadows)
				if (activeShadowSet.find(it) == activeShadowSet.end())
				{
					allLightData[it].hasShadow = 1;
					dirtyShadows.push_back(it);
					if (allLightData[it].type == Light::Light_Point)
					{
						if (!openPointSlots.empty())
						{
							allLightData[it].shadowNum = openPointSlots.front();
							openPointSlots.erase(openPointSlots.begin());
						}
						else
							SLICE_LOG_WARNING("Wrong Calc for open point shadow slots");
					}
					else if (allLightData[it].type == Light::Light_Spot)
					{
						if (!openSpotSlots.empty())
						{
							allLightData[it].shadowNum = openSpotSlots.front().first;
							allLightData[it].spotShadowNum = openSpotSlots.front().second;
							openSpotSlots.erase(openSpotSlots.begin());
						}
						else
							SLICE_LOG_WARNING("Wrong Calc for open spot shadow slots");
					}
				}
		}
		std::swap(activeShadowSet, newShadows);
	}
	float RenderManager::CalcPointLightFar(const glm::vec3& scale, const float lightIntensity)
	{
		float maxS = fmaxf(scale.x, fmaxf(scale.y, scale.z));
		return (log10f(lightIntensity) + maxS) * mPointLightFar;
	}

	const glm::mat4& RenderManager::GetViewMatrix() const { return V; }
	const glm::mat4& RenderManager::GetProjMatrix() const { return P; }
#pragma endregion

#pragma region Linking
	// Binds Framebuffer and Links any internal Output textures
	void RenderManager::LinkFrameBufferSettings(FBOType fbo, int numColAttachments, ...)
	{
		if (fbo == FBOType::FB_TOTAL)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			mCurrFBO = fbo;
			return;
		}
		else if (fbo != mCurrFBO)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO[fbo]);
			mCurrFBO = fbo;
		}

		va_list args;
		va_start(args, numColAttachments);
		for (int i{}; i < numColAttachments; ++i)
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, va_arg(args, GLuint), 0);
		va_end(args);
	}
	// Does all the glEnable and Disables etc.
	void RenderManager::LoadSettings(GPUSetting setting)
	{
		GPUSetting changeInSettings{ static_cast<GPUSetting>(mCurrGPUSetting ^ setting) };
		if (changeInSettings)
		{
			// glEnables
			if (changeInSettings & GPS_ENABLE_CULL_FACE)
			{
				if (setting & GPS_ENABLE_CULL_FACE)
					glEnable(GL_CULL_FACE);
				else
					glDisable(GL_CULL_FACE);
			}
			if (changeInSettings & GPS_ENABLE_BLEND)
			{
				if (setting & GPS_ENABLE_BLEND)
					glEnable(GL_BLEND);
				else
					glDisable(GL_BLEND);
			}
			if (changeInSettings & GPS_ENABLE_DEPTH)
			{
				if (setting & GPS_ENABLE_DEPTH)
					glEnable(GL_DEPTH_TEST);
				else
					glDisable(GL_DEPTH_TEST);
			}
			// Other Settings
			if (changeInSettings & GPS_CULL_BACK_NOT_FRONT)
			{
				if (setting & GPS_CULL_BACK_NOT_FRONT)
					glCullFace(GL_BACK);
				else
					glCullFace(GL_FRONT);
			}
			if (changeInSettings & GPS_BLEND_SRC_ONEMINUS & setting)
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if (changeInSettings & GPS_BLEND_ONE_ONE & setting)
				glBlendFunc(GL_ONE, GL_ONE);
			if (changeInSettings & GPS_DEPTH_LESS & setting)
				glDepthFunc(GL_LESS);

			mCurrGPUSetting = setting;
		}
	}
	void RenderManager::QuickSetSettings(GPUSetting setting, bool toggleOn)
	{
		if (toggleOn)
		{
			if (setting & mCurrGPUSetting)
				return;
			switch (setting)
			{
			case GPS_ENABLE_CULL_FACE:
				glEnable(GL_CULL_FACE);
				break;
			case GPS_ENABLE_BLEND:
				glEnable(GL_BLEND);
				break;
			case GPS_ENABLE_DEPTH:
				glEnable(GL_DEPTH_TEST);
				break;
			case GPS_CULL_BACK_NOT_FRONT:
				glCullFace(GL_BACK);
				break;
			case GPS_BLEND_ONE_ONE:
				glBlendFunc(GL_ONE, GL_ONE);
				if(mCurrGPUSetting & GPS_BLEND_SRC_ONEMINUS)
					mCurrGPUSetting = static_cast<GPUSetting>(mCurrGPUSetting ^ GPS_BLEND_SRC_ONEMINUS);
				break;
			case GPS_BLEND_SRC_ONEMINUS:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				if(mCurrGPUSetting & GPS_BLEND_ONE_ONE)
					mCurrGPUSetting = static_cast<GPUSetting>(mCurrGPUSetting ^ GPS_BLEND_ONE_ONE);
				break;
			case GPS_DEPTH_LESS:
				glDepthFunc(GL_LESS);
				break;
			}
			mCurrGPUSetting = static_cast<GPUSetting>(mCurrGPUSetting | setting);
		}
		else
		{
			switch (setting)
			{
			case GPS_ENABLE_CULL_FACE:
				glDisable(GL_CULL_FACE);
				break;
			case GPS_ENABLE_BLEND:
				glDisable(GL_BLEND);
				break;
			case GPS_ENABLE_DEPTH:
				glDisable(GL_DEPTH_TEST);
				break;
			case GPS_CULL_BACK_NOT_FRONT:
				glCullFace(GL_FRONT);
				break;
			}
			if (mCurrGPUSetting & setting)
				mCurrGPUSetting = static_cast<GPUSetting>(mCurrGPUSetting ^ setting);
		}
	}
	void RenderManager::ForceResetDefaultSettings()
	{
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glDepthFunc(GL_LESS);
		mCurrGPUSetting = GPS_DEFAULT;
	}
	void RenderManager::AddDebugRaysToDraw(const DebugDrawRayEvent& e)
	{
		const float thickness = 0.05f;

		glm::vec3 direction = glm::normalize(e.Dir);

		// 2. Calculate Rotation
		// We want to rotate the default UP vector (0,1,0) to our target direction
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

		// Handle the edge case where direction is exactly opposite of UP
		glm::quat rotation;
		float dot = glm::dot(up, direction);
		if (dot < -0.9999f) {
			rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(1, 0, 0));
		}
		else {
			rotation = glm::rotation(up, direction);
		}
		glm::mat4 model{ 1.f };
		model = glm::translate(model, e.Origin + (direction * (e.magnitude * 0.5f)));

		// Rotation: Orient towards Dir
		model = model * glm::toMat4(rotation);

		// Scale: X and Z are thickness, Y is the length (magnitude)
		model = glm::scale(model, glm::vec3(thickness, e.magnitude, thickness));
		mDebugDrawRays.push_back(model);
	}
	void RenderManager::ForceSetCustomShader(const std::string& sh, GLuint s)
	{
		mCurrShader.first = sh;
		mCurrShader.second = s;
	}
	// Changes Shader if not current
	void RenderManager::SetShader(std::string sh)
	{
		if (sh != mCurrShader.first)
		{
			shaderHandle = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>(sh);
			mCurrShader.first = sh;
			mCurrShader.second = shaderHandle.get()->s;
			glUseProgram(mCurrShader.second);
		}
	}
	// Honestly, Just clears all currently
	void RenderManager::ClearBuffer(BufferClearSetting setting)
	{
		switch (setting)
		{
		case BufferClearSetting::COLOR_ONLY:
		{
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);
			break;
		}
		case BufferClearSetting::ALL:
		{
			//glClearBufferfv(GL_COLOR, 0, zeroFiller);
			//glClearBufferfv(GL_COLOR, 1, zeroFiller);
			//glClearBufferfv(GL_COLOR, 2, zeroFiller);
			//glClearBufferfv(GL_COLOR, 3, zeroFiller);
			//glClearBufferfv(GL_DEPTH, 0, oneFiller);
			//glClearColor(0.75294f, 1.f, 0.93333f, 1.f);
			//__fallthrough;
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		}
		}
	}
	void RenderManager::ToggleFinalTexture()
	{
		if (mCurrFinalColAttachment == GOUT_FINAL)
			mCurrFinalColAttachment = GOUT_POST;
		else
			mCurrFinalColAttachment = GOUT_FINAL;
	}
	void RenderManager::SetUniformVec3(GLint uniformLoc, const glm::vec3& vec)
	{
		glUniform3f(uniformLoc, vec.x, vec.y, vec.z);
	}
	// Sets this up at the start to bind slots 12~15 with the instance transform :p
	//void RenderManager::LinkTransformInstancing(GUID guid)
	//{
	//	//std::string tempFilePath = "Assets/Models/" + mdlName + ".txt";
	//	auto modelHandle = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>(guid);
	//	auto model = modelHandle.get();
	//	auto& mdl = model->meshes[0];	//i call it mdl cuz im lazy to change the below
	//
	//	// auto& mdl = Core::GetInstance()->GetResourceManager()->GetModel(mdlName);
	//
	//	// Link drawing models with instancing vbo
	//	for (int i{}; i < 4; ++i)
	//	{
	//		glBindVertexArray(mdl.vao);
	//		int idx = 12 + i; // 12 ~ 15
	//		glEnableVertexArrayAttrib(mdl.vao, idx);
	//		glVertexArrayVertexBuffer(mdl.vao, idx, mIVBO, sizeof(glm::vec4) * i, sizeof(glm::mat4));
	//		glVertexArrayAttribIFormat(mdl.vao, idx, 4, GL_FLOAT, 0);
	//		glVertexArrayAttribBinding(mdl.vao, idx, idx);
	//
	//		glVertexAttribDivisor(idx, 1);
	//	}
	//	glBindVertexArray(0);
	//}
#pragma endregion

#pragma region IDPick
	void RenderManager::SelectCamIDPick(Entity cam)
	{
		mCurrentCamIDHover = cam;
	}
	unsigned int RenderManager::ObjectPick(int mouseX, int mouseY)
	{
		mObjPickX = mouseX;
		mObjPickY = mouseY;
		IDPick();
		return GetPickedID();
	}
	void RenderManager::IDPick()
	{
		// if out of bounds
		if (mObjPickX > static_cast<unsigned int>(Core::GetInstance()->GetSystem<CameraSystem>().maxWidth) || mObjPickX < 0 ||
			mObjPickY >  static_cast<unsigned int>(Core::GetInstance()->GetSystem<CameraSystem>().maxHeight) || mObjPickY < 0)
		{
			mIDHovered = std::numeric_limits<unsigned int>().max();
			return;
		}
		mObjPickedThisFrame = true;

		GLint prevBinding{};
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBinding);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO[FB_DEFERRED]);
		glActiveTexture(GL_TEXTURE1);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mColAttachment[GOUT_ID], 0);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIdx[0]]);
		
		glNamedFramebufferReadBuffer(mFBO[FB_DEFERRED], GL_COLOR_ATTACHMENT1);
		glReadPixels(mObjPickX, mObjPickY, 1, 1,
			GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
		
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIdx[1]]);
		GLuint* src = (GLuint*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		if (src)
		{
			mIDHovered = *src;
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		}
		if (mIDHovered == 0)
			mIDHovered = std::numeric_limits<unsigned int>().max();
		
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, prevBinding);
		CheckGLError();
	}
	unsigned int RenderManager::GetPickedID()
	{
		return mIDHovered;
	}
#pragma endregion
}