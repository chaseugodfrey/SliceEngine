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
#define PI05F 1.57079632679f
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

#include "Core/Core.h"

#include "WorldSpaceGraphicsSystem.h"
#include "CameraSystem.h"
#include "LightingSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Systems/ParticleSystemManager.h"
#include "Navigation/NavigationSystem.h"

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Resource/Model.h"

// My Comments to (Ctrl + f): -TODO- MAYDO:
// -TODO- Currently not using mat in the InstanceData struct (original intention is to keep track of which textures to use)
// -TODO- Make Gather Render Commands, and then draw using these commands instead lol

namespace SliceEngine
{
#pragma region Generate GPU Objects
	RenderManager::RenderManager()
	{
		CreateFramebuffers();
	}
	RenderManager::~RenderManager()
	{
		glDeleteFramebuffers(FB_TOTAL, mFBO);
		glDeleteBuffers(1, &mIVBO);

		glDeleteTextures(GOUT_TOTAL, mColAttachment);
		for (auto i : mBloomMips)
			glDeleteTextures(1, &i.tex);

		if (SkyboxMap != 0)
			glDeleteTextures(1, &SkyboxMap);

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
			};
			glDrawBuffers(sizeof(drawBuffers) / sizeof(unsigned int), drawBuffers); // -TODO- Check if this part links the frame buffer or texture
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
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	void RenderManager::CreateInstancingParams()
	{
		mInstanceVtx.resize(mMaxInstance);
		glCreateBuffers(1, &mIVBO);
		glNamedBufferStorage(mIVBO, mMaxInstance * sizeof(InstanceData), mInstanceVtx.data(), GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mIVBO);
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
		// float_16 rgb-Roughness + a-Metalic
		glTextureStorage2D(mColAttachment[GOUT_ROUGH_METAL], 1, GL_RGBA16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_ROUGH_METAL], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_ROUGH_METAL], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// float_16 rgba Final Image To Send to Camera Texture
		glTextureStorage2D(mColAttachment[GOUT_FINAL], 1, GL_RGBA16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_FINAL], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_FINAL], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// float_16 rgba Post Processing for toggling Image To Send to Camera Texture
		glTextureStorage2D(mColAttachment[GOUT_POST], 1, GL_RGBA16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_POST], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_POST], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

		GLuint faceTexID = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>((GUID)18349208178533231704).get()->texture_id;
		GLint srcInternalFmt, width, height;
		glBindTexture(GL_TEXTURE_2D, faceTexID);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &srcInternalFmt);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		glGenTextures(1, &SkyboxMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxMap);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, srcInternalFmt, width, height);
		for (u_int i{}; i < 6; ++i)
		{
			glCopyImageSubData(faceTexID, GL_TEXTURE_2D, 0, 0, 0, 0, SkyboxMap, GL_TEXTURE_CUBE_MAP, 0, 0, 0, i, width, height, 1);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
#pragma endregion

#pragma region Camera
	GameObject RenderManager::CreateCamera()
	{
		GameObject newCam = Core::GetInstance()->mFactory.CreateEO();
		
		auto& transform = newCam.GetComponent<Transform>();
		transform.position = glm::vec3(-2.f, 1.f, 0.f);
		transform.rotation = glm::quat(glm::radians(glm::vec3(0.f, 0.f, -10.f)));
		newCam.AddComponent<Camera>();
		//newCam.GetComponent<Camera>().renderTag = DEBUG_OBJ_TAG | DEBUG_GRID_TAG;

		return newCam;
	}
	// MAYDO: has issue when deleting the cam game object, causing the mainCam to become Empty
	void RenderManager::SetMainGameCamera(GameObject cam)
	{
		mainCam.emplace(cam);
	}
	std::optional<GameObject>& RenderManager::GetGameCamera()
	{
		return mainCam;
	}
	void RenderManager::GetCameraAxis(GameObject& cam, glm::vec3& forward, glm::vec3& right, glm::vec3& up)
	{
		glm::vec3 f{ 1.f, 0.f, 0.f }, u{ 0.f, 1.f, 0.f }, r{ 0.f,0.f,1.f };
		auto& camTrans = cam.GetComponent<Transform>();
		glm::mat3 rot = glm::mat3_cast(camTrans.rotation);
		//glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));
		forward = rot * f;
		right = rot * r;
		up = rot * u;
	}
#pragma endregion

#pragma region Render
	void RenderManager::Render()
	{
		//Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Update(0.f);
		//GatherDrawCalls();// Does nothing atm

		IDPick();

		SetShader(S_POINT_SHADOW);
		LinkFrameBufferSettings(FB_NIL, 0);
		LoadSettings(GPS_SHADOW);
		RenderPointShadowMaps();

		auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>();
		for (auto cam : cams)
		{
			mCurrFinalColAttachment = GOUT_FINAL;

			CalculateVP(cam);
			SetShader(S_SHADOW);
			LinkFrameBufferSettings(FB_NIL, 0);
			LoadSettings(GPS_SHADOW);
			RenderDirectionalShadowMaps(cam);

			SetShader(S_DEFERRED);
			if(cam == mCurrentCamIDHover)
				LinkFrameBufferSettings(FB_DEFERRED, 5, mColAttachment[GOUT_DIF], mColAttachment[GOUT_ID], mColAttachment[GOUT_POS], mColAttachment[GOUT_NOM], mColAttachment[GOUT_ROUGH_METAL]);
			else
				LinkFrameBufferSettings(FB_DEFERRED, 5, mColAttachment[GOUT_DIF], 0, mColAttachment[GOUT_POS], mColAttachment[GOUT_NOM], mColAttachment[GOUT_ROUGH_METAL]);
			LoadSettings(GPS_DEFAULT);
			UpdateCamVP();
			BindCameraDepth(cam);
			ClearBuffer(BufferClearSetting::ALL);
			Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Render(mCurrShader.second, true);

			SetShader(S_SKYBOX);
			LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
			LoadSettings(GPS_SHADOW);
			UpdateCamVP();
			BindCameraDepth(cam);
			ClearBuffer(BufferClearSetting::COLOR_ONLY);
			RenderSkybox(cam);

			SetShader(S_LIGHTING);
			LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
			UpdateCamVP();
			BindCameraDepth(cam);
			RenderLighting(cam);

			SetShader(S_PARTICLES);
			// Use Same FrameBufferSettings & Don't Clear Buffer
			UpdateCamVP();
			BindCameraDepth(cam);
			LoadSettings(GPS_PARTICLES);
			RenderAfterLighting(cam);
			
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & DEBUG_ALL_DEBUG)
			{
				LoadSettings(GPS_DEBUG);
				RenderDebug(cam);
			}
			// Post Processings
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & RENDER_FOG)
				RenderFog(cam);
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & RENDER_BLOOM)
				RenderBloom(cam);
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & RENDER_VIGNETTE)
				RenderVignette(cam);

			LoadSettings(GPS_DEFAULT);
			RenderGammaCorrection(cam);
		}
		
		mObjPickedThisFrame = false;
		LinkFrameBufferSettings(FB_TOTAL, 0);
		std::swap(pboIdx[0], pboIdx[1]);
	}
	void RenderManager::RenderDebug(Entity cam)
	{
		// Draw other cameras' frustrum
		if(Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & DEBUG_FRUSTRUM_TAG)
		{
			SetShader(S_INSTANCED);
			UpdateCamVP();
			BindCameraDepth(cam);

			auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::FRUSTRUM_DEFAULT).get();
			auto& mdl = model.meshes[0];
			glBindVertexArray(mdl.vao);

			//auto& frustrum = Core::GetInstance()->GetResourceManager()->GetModel("FrustrumFake");
			auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>();
			int count{};
			for (auto& entity : cams)
			{
				if (entity == cam) continue;

				auto& transform = Core::GetInstance()->GetRegistry().get<Transform>(entity);
				auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(entity);

				mInstanceVtx[count].mtx = transform.transform *
					glm::rotate(glm::mat4(1.0f), -PI05F, glm::vec3(0.f, 1.f, 0.f)) *
					glm::inverse(glm::perspective(glm::radians(camera.pov), static_cast<float>(camera.width) / static_cast<float>(camera.height), camera.near, camera.far)) *
					glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
				++count;
			}
			glNamedBufferSubData(mIVBO, 0, sizeof(InstanceData) * count, mInstanceVtx.data());
			glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, count);
		}
		
		// Draw Instance Debug Box
		if (Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & DEBUG_OBJ_TAG)
		{
			SetShader(S_INSTANCED);
			UpdateCamVP();
			BindCameraDepth(cam);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			auto view = Core::GetInstance()->GetRegistry().view<PhysicEntity>(); //renderEntity
			for (int i{}; i < 3; ++i)
			{
				GUID modelID;
				switch (i)
				{
				case 0:
					modelID = (GUID)DefaultResourceIDs::CUBE_DEFAULT;
					break;
				case 1:
					modelID = (GUID)DefaultResourceIDs::SPHERE_DEFAULT;
					break;
				case 2:
					modelID = (GUID)DefaultResourceIDs::CAPSULE_DEFAULT;
					break;
				}

				auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>(modelID).get();
				auto& mdl = model.meshes[0];	//i call it mdl cuz im lazy to change the below
				glBindVertexArray(mdl.vao);

				int num{}, offset{};
				for (auto entity : view)
				{
					auto& transform = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity);
					auto& shape = Core::GetInstance()->mFactory.mRegistry.get<ColliderShape>(entity);
					if (std::holds_alternative<ColliderShape::BoxData>(shape.shapeData))
					{
						if (i != 0)
							continue;
						auto& boxData = std::get<ColliderShape::BoxData>(shape.shapeData);
						mInstanceVtx[num].mtx = glm::scale(glm::translate(transform.transform, glm::vec3(shape.offSet.GetX(),shape.offSet.GetY(),shape.offSet.GetZ())), glm::vec3(boxData.scale.GetX() * 2.f, boxData.scale.GetY() * 2.f, boxData.scale.GetZ() * 2.f));
					}
					if (std::holds_alternative<ColliderShape::SphereData>(shape.shapeData))
					{
						if (i != 1)
							continue;
						auto& sphereData = std::get<ColliderShape::SphereData>(shape.shapeData);
						mInstanceVtx[num].mtx = glm::scale(glm::translate(transform.transform, glm::vec3(shape.offSet.GetX(), shape.offSet.GetY(), shape.offSet.GetZ())), glm::vec3(sphereData.radius * 2.f));
					}
					if (std::holds_alternative<ColliderShape::CapsuleData>(shape.shapeData))
					{
						if (i != 2)
							continue;
						auto& capsuleData = std::get<ColliderShape::CapsuleData>(shape.shapeData);
						mInstanceVtx[num].mtx = glm::scale(glm::translate(transform.transform, glm::vec3(shape.offSet.GetX(), shape.offSet.GetY(), shape.offSet.GetZ())), glm::vec3(capsuleData.radius * 2.f, capsuleData.height * 2.f, capsuleData.radius * 2.f));
					}

					num++;
					if (num == mMaxInstance)
					{
						glNamedBufferSubData(mIVBO, 0, sizeof(InstanceData) * num, mInstanceVtx.data() + offset);
						glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, num);
						offset += num;
						num = 0;
					}
				}
				if (num != 0)
				{
					glNamedBufferSubData(mIVBO, 0, sizeof(InstanceData) * num, mInstanceVtx.data() + offset);
					glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, num);
				}
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Draw Recast Navigation Data
		if (Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & DEBUG_NAVMESH_TAG)
		{
			SetShader(S_BASIC);
			UpdateCamVP();
			BindCameraDepth(cam);
			GLuint uniformLoc = glGetUniformLocation(mCurrShader.second, "uColor");
			auto& navDatOpt = Core::GetInstance()->GetSystem<NavigationSystem>().GetNavMeshDebugData();
			if (navDatOpt.has_value())
			{
				auto& navDat = navDatOpt.value();
				//glUniform4f(uniformLoc, mNavMeshDebugColor_Base.r, mNavMeshDebugColor_Base.g, mNavMeshDebugColor_Base.b, mNavMeshDebugColor_Base.a);
				glUniform4f(uniformLoc, 0.f, 0.f, 0.7f, 0.4f);
				glBindVertexArray(navDat.data[0].vao);
				glDrawArrays(GL_TRIANGLES, 0, navDat.data[0].drawCnt);
				glUniform4f(uniformLoc, 0.f, 0.2f, 0.25f, 0.85f);
				//glUniform4f(uniformLoc, mNavMeshDebugColor_Bounds.r, mNavMeshDebugColor_Bounds.g, mNavMeshDebugColor_Bounds.b, mNavMeshDebugColor_Bounds.a);
				glBindVertexArray(navDat.data[1].vao);
				glDrawArrays(GL_TRIANGLES, 0, navDat.data[1].drawCnt);
			}
		}

		// Draw Debug Line
		if(Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & DEBUG_GRID_TAG)
		{
			SetShader(S_DEBUG_LINE);
			UpdateCamVP();
			BindCameraDepth(cam);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	void RenderManager::RenderPointShadowMaps()
	{
		const auto shadowDim = Core::GetInstance()->GetSystem<LightingSystem>().SHADOW_DIMENSION;
		glViewport(0, 0, shadowDim, shadowDim);

		auto view = Core::GetInstance()->GetRegistry().view<lightingEntity>();
		for (auto entity : view)
		{
			auto& light = Core::GetInstance()->GetRegistry().get<Light>(entity);
			if (light.type != Light::LightType::Light_Point) continue;
			auto& transform = Core::GetInstance()->GetRegistry().get<Transform>(entity);

			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light.shadowCubeMap, 0);
			ClearBuffer(BufferClearSetting::ALL);

			GLuint uniformLoc = glGetUniformLocation(mCurrShader.second, "uLightPos");
			glUniform3f(uniformLoc, transform.position.x, transform.position.y, transform.position.z);
			uniformLoc = glGetUniformLocation(mCurrShader.second, "uFarPlane");
			glUniform1f(uniformLoc, pointLightFar);
			glm::mat4 lightP = glm::perspective(PI05F, 1.f, 0.01f, pointLightFar);
			std::stringstream ss{};
			for (size_t i{}; i < 6; ++i)
			{
				glm::mat4 shadowMat{ lightP * glm::lookAt(transform.position, transform.position + mShadowCamDir[i].target, mShadowCamDir[i].up) };
				ss.str("");
				ss << "uShadowMat[" << std::to_string(i) << "]";
				uniformLoc = glGetUniformLocation(mCurrShader.second, ss.str().c_str());
				glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &shadowMat[0][0]);
			}

			Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Render(mCurrShader.second, false);
		}
	}
	void RenderManager::RenderDirectionalShadowMaps(Entity cam)
	{
		const auto shadowDim = Core::GetInstance()->GetSystem<LightingSystem>().SHADOW_DIMENSION;
		glViewport(0, 0, shadowDim, shadowDim);

		auto& camT = Core::GetInstance()->GetRegistry().get<Transform>(cam);

		auto view = Core::GetInstance()->GetRegistry().view<lightingEntity>();
		for (auto entity : view)
		{
			auto& light = Core::GetInstance()->GetRegistry().get<Light>(entity);
			if (light.type != Light::LightType::Light_Directional) continue;
			auto& transform = Core::GetInstance()->GetRegistry().get<Transform>(entity);

			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light.depthTex, 0);
			glClear(GL_DEPTH_BUFFER_BIT);

			SetDirectionalLightMtx(camT.position, transform.position);

			Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Render(mCurrShader.second, false);
		}
	}
	void RenderManager::RenderSkybox(Entity cam)
	{
		glBindTextureUnit(0, SkyboxMap);

		GLint uniformLoc;
		if (UniformExists("M", uniformLoc))
		{
			auto& camT = Core::GetInstance()->GetRegistry().get<Transform>(cam);

			glm::mat4 M{ 1.f };
			M = glm::translate(M, glm::vec3(camT.transform[3]));
			M = glm::scale(M, glm::vec3(100.f));
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &M[0][0]);
		}

		auto& mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::CUBE_DEFAULT).get()->meshes[0];
		glBindVertexArray(mdl.vao);
		glDrawElements(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr);
	}
	void RenderManager::RenderLighting(Entity cam)
	{
		glBindTextureUnit(0, mColAttachment[GOUT_DIF]);
		glBindTextureUnit(1, mColAttachment[GOUT_POS]);
		glBindTextureUnit(2, mColAttachment[GOUT_NOM]);
		glBindTextureUnit(3, mColAttachment[GOUT_ROUGH_METAL]);

		auto& camT = Core::GetInstance()->GetRegistry().get<Transform>(cam);
		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uCamPos");
		glUniform3f(uniformLoc, camT.position.x, camT.position.y, camT.position.z);

		auto view = Core::GetInstance()->GetRegistry().view<lightingEntity>();
		for (auto entity : view)
		{
			auto& light = Core::GetInstance()->GetRegistry().get<Light>(entity);
			auto& lightT = Core::GetInstance()->GetRegistry().get<Transform>(entity);
			uniformLoc = glGetUniformLocation(mCurrShader.second, "uLight.position");
			glUniform3f(uniformLoc, lightT.position.x, lightT.position.y, lightT.position.z);
			uniformLoc = glGetUniformLocation(mCurrShader.second, "uLight.color");
			glUniform4f(uniformLoc, light.color.r, light.color.g, light.color.b, light.intensity);
			uniformLoc = glGetUniformLocation(mCurrShader.second, "uLight.type");
			glUniform1i(uniformLoc, static_cast<int>(light.type));
			uniformLoc = glGetUniformLocation(mCurrShader.second, "uLight.hasShadow");
			glUniform1f(uniformLoc, 1.f);

			switch(light.type)
			{
			case Light::LightType::Light_Directional:
			{
				LoadSettings(GPS_ADDITION);

				uniformLoc = glGetUniformLocation(mCurrShader.second, "uLight.direction");
				glUniform3f(uniformLoc, -lightT.position.x, -lightT.position.y, -lightT.position.z);

				SetDirectionalLightMtx(camT.position, lightT.position);

				glBindTextureUnit(4, light.depthTex);

				auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT);
				auto& mesh = mdl.get()->meshes[0];
				glBindVertexArray(mesh.vao);
				//glDrawArrays(mdl.get()->drawMode, 0, mdl.get()->drawCnt);
				glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				break;
			}
			case Light::LightType::Light_Point:
			{
				if(glm::distance(camT.position, lightT.position) > pointLightFar * 0.5f)
					LoadSettings(GPS_ADDITION);
				else
					LoadSettings(GPS_SPE_ADDITION);

				glm::mat4 M{ 1.f };
				M = glm::translate(M, lightT.position);
				M = glm::scale(M, glm::vec3(pointLightFar, pointLightFar, pointLightFar));
				uniformLoc = glGetUniformLocation(mCurrShader.second, "M");
				glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &M[0][0]);
				uniformLoc = glGetUniformLocation(mCurrShader.second, "uFarPlane");
				glUniform1f(uniformLoc, pointLightFar);

				glBindTextureUnit(5, light.shadowCubeMap);

				auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::SPHERE_DEFAULT);
				auto& mesh = mdl.get()->meshes[0];
				glBindVertexArray(mesh.vao);
				//glDrawArrays(mdl.get()->drawMode, 0, mdl.get()->drawCnt);
				glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				break;
			}
			}
		}
	}
	void RenderManager::RenderAfterLighting(Entity cam)
	{
		auto& mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT).get()->meshes[0];
		glBindVertexArray(mdl.vao);
		int cnt{};
		GLuint lastTexID{};
		for (auto& ptx : Core::GetInstance()->GetSystem<ParticleSystemManager>().particlesTransforms)
		{
			if (ptx.textureID == 0)
				ptx.textureID = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>((GUID)(DefaultResourceIDs::COLOR_DEADED_DEFAULT))->texture_id;
			if (ptx.textureID != lastTexID)
			{
				if (cnt)
				{
					glNamedBufferSubData(mIVBO, 0, sizeof(InstanceData) * cnt, mInstanceVtx.data());
					glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, cnt);
				}
				cnt = 0;
				lastTexID = ptx.textureID;
				glBindTextureUnit(0, lastTexID);
			}
			mInstanceVtx[cnt].mtx = ptx.transform;
			++cnt;
			if (cnt == mMaxInstance)
			{
				glNamedBufferSubData(mIVBO, 0, sizeof(InstanceData) * cnt, mInstanceVtx.data());
				glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, cnt);
				cnt = 0;
			}
		}
		if (cnt)
		{
			glNamedBufferSubData(mIVBO, 0, sizeof(InstanceData) * cnt, mInstanceVtx.data());
			glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, cnt);
		}
	}
	void RenderManager::RenderFog(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		auto& camT = Core::GetInstance()->GetRegistry().get<Transform>(cam);

		SetShader(S_FOG);
		LoadSettings(GPS_DEFAULT);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);
		glBindTextureUnit(1, mColAttachment[GOUT_POS]);
		glBindTextureUnit(2, mColAttachment[GOUT_NOM]);
		ToggleFinalTexture();
		LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
		ClearBuffer(BufferClearSetting::ALL);

		GLuint uniformLoc = glGetUniformLocation(mCurrShader.second, "uFogColor");
		glUniform3f(uniformLoc, camera.fogColor.r, camera.fogColor.g, camera.fogColor.b);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uFogIntensity");
		glUniform1f(uniformLoc, camera.fogIntensity);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uCamPos");
		glUniform3f(uniformLoc, camT.position.x, camT.position.y, camT.position.z);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	void RenderManager::RenderBloom(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);

		// Extract the Bright
		SetShader(S_BLOOM_SPLIT);
		LoadSettings(GPS_BLOOM);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);
		LinkFrameBufferSettings(FB_FINAL, 1, mBloomMips[0].tex);
		ClearBuffer(BufferClearSetting::COLOR_ONLY);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Downscaling
		SetShader(S_DOWNSCALING);

		glBindTextureUnit(0, mBloomMips[0].tex);
		GLint uniformLoc = glGetUniformLocation(mCurrShader.second, "uTexelSize");
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

		SetShader(S_UPSCALING);
		LoadSettings(GPS_BLOOM);

		uniformLoc = glGetUniformLocation(mCurrShader.second, "uFilterRadius");
		glUniform1f(uniformLoc, camera.bloomFilterRadius);

		for (int i{ mMaxBloom - 1 }; i > 0; --i)
		{
			glBindTextureUnit(0, mBloomMips[i].tex);
			glViewport(0, 0, mBloomMips[i-1].intSize.x, mBloomMips[i-1].intSize.y);
			LinkFrameBufferSettings(FBOType::FB_FINAL, 1, mBloomMips[i-1].tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		SetShader(S_BLOOM_JOIN);
		LoadSettings(GPS_DEFAULT);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);
		glBindTextureUnit(1, mBloomMips[0].tex);
		ToggleFinalTexture();
		LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
		ClearBuffer(BufferClearSetting::ALL);

		uniformLoc = glGetUniformLocation(mCurrShader.second, "uBloomStrength");
		glUniform1f(uniformLoc, camera.bloomStrength);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	void RenderManager::RenderVignette(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);

		SetShader(S_VIGNETTE);
		LoadSettings(GPS_DEFAULT);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);
		ToggleFinalTexture();
		LinkFrameBufferSettings(FB_FINAL, 1, mColAttachment[mCurrFinalColAttachment]);
		ClearBuffer(BufferClearSetting::ALL);

		GLuint uniformLoc = glGetUniformLocation(mCurrShader.second, "uVignetteCenter");
		glUniform2f(uniformLoc, camera.vignetteCenter.x, camera.vignetteCenter.y);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uVignetteIntensity");
		glUniform1f(uniformLoc, camera.vignetteIntensity);
		uniformLoc = glGetUniformLocation(mCurrShader.second, "uVignetteSmoothness");
		glUniform1f(uniformLoc, camera.vignetteSmoothness);

		glDrawArrays(GL_TRIANGLES, 0, 6);

	}
	void RenderManager::RenderGammaCorrection(Entity cam)
	{
		SetShader(ShaderOpt::S_FINAL);
		LinkFrameBufferSettings(FB_FINAL, 1, Core::GetInstance()->GetRegistry().get<Camera>(cam).textureID);
		ClearBuffer(BufferClearSetting::ALL);
		glBindTextureUnit(0, mColAttachment[mCurrFinalColAttachment]);

		auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT).get();
		auto& mdl = model.meshes[0];	//i call it mdl cuz im lazy to change the below
		glBindVertexArray(mdl.vao);
		//glDrawArrays(mdl.get()->drawMode, 0, mdl.get()->drawCnt);
		glDrawElements(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr);
	}
#pragma endregion

#pragma region Rendering Helpers
	// Only need to calculate for camera once
	void RenderManager::CalculateVP(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		auto& camTrans = Core::GetInstance()->GetRegistry().get<Transform>(cam);

		auto& worldTr = camTrans.transform;

		glm::vec3 camPosition{ worldTr[3] }, target{1.f, 0.f, 0.f}, up{0.f, 1.f, 0.f};
		glm::mat3 rot = glm::mat3(worldTr);

		//glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));
		glm::vec3 forward = glm::normalize(rot * target);

		V = glm::lookAt(camPosition, camPosition + forward, up);

		P = glm::perspective(glm::radians(camera.pov), static_cast<float>(camera.width) / static_cast<float>(camera.height), camera.near, camera.far);
	}
	// Updates V P uniforms
	void RenderManager::UpdateCamVP()
	{
		GLint uniformLoc;
		if (UniformExists("V", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &V[0][0]);
		if (UniformExists("P", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &P[0][0]);

	}
	// Binds Depth Texture from camera & ViewPort size
	void RenderManager::BindCameraDepth(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, camera.depthTex, 0);

		glViewport(0, 0, camera.width, camera.height);
	}
	void RenderManager::GatherDrawCalls()
	{

	}
	bool RenderManager::UniformExists(const char* str, GLint& ref)
	{
		ref = glGetUniformLocation(mCurrShader.second, str);
		if (ref >= 0)
			return true;

		std::stringstream ss;
		ss << "Uniform variable: " << str << " doesn't exist!!!\n";
		SLICE_LOG_WARNING(ss.str());
		return false;
	}
#pragma endregion

#pragma region Linking
	void RenderManager::SetDirectionalLightMtx(glm::vec3 camPos, glm::vec3 lightPos)
	{
		glm::vec3 dir = glm::normalize(-lightPos);

		float sDim = 40.f;
		camPos -= sDim * dir;

		glm::mat4 lightP = glm::ortho(-sDim, sDim, -sDim, sDim, 0.f, 4.f * sDim);
		glm::mat4 lightV = glm::lookAt(
			camPos,
			camPos + dir,
			glm::vec3(1.f, 0.f, 0.f));
		lightP = lightP * lightV;

		GLuint uniformLoc = glGetUniformLocation(mCurrShader.second, "uLightMtx");
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &lightP[0][0]);
	}
	// Binds Framebuffer and Links any internal Output textures
	void RenderManager::LinkFrameBufferSettings(FBOType fbo, int numColAttachments, ...)
	{
		if (fbo == FBOType::FB_TOTAL)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	// Changes Shader if not current
	void RenderManager::SetShader(ShaderOpt sh)
	{
		if (sh != mCurrShader.first)
		{
			shaderHandle = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)sh);
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
		return;
	}
	unsigned int RenderManager::GetPickedID()
	{
		return mIDHovered;
	}
#pragma endregion
}