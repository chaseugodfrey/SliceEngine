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

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Resource/Model.h"

// My Comments to (Ctrl + f): -TODO- MAYDO:

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

		LinkFrameBufferSettings(FB_TOTAL, F_CLEAR);
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	void RenderManager::CreateInstancingParams()
	{
		mInstanceVtx.resize(mMaxInstance);
		glCreateBuffers(1, &mIVBO);
		glNamedBufferStorage(mIVBO, mInstanceVtx.size() * sizeof(glm::mat4), mInstanceVtx.data(), GL_DYNAMIC_STORAGE_BIT);
		LinkTransformInstancing((GUID)DefaultResourceIDs::CUBE_DEFAULT);
		LinkTransformInstancing((GUID)DefaultResourceIDs::FRUSTRUM_DEFAULT);
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
		// float_16 rgba Final Image To Send to Camera Texture
		glTextureStorage2D(mColAttachment[GOUT_FINAL], 1, GL_RGBA16F, maxWidth, maxHeight);
		glTextureParameterf(mColAttachment[GOUT_FINAL], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[GOUT_FINAL], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
		Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Update(0.f);

		IDPick();

		SetShader(S_POINT_SHADOW);
		LinkFrameBufferSettings(FB_NIL, F_CLEAR);
		LoadSettings(GPUSetting::SHADOW);
		RenderPointShadowMaps();

		auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>();
		for (auto cam : cams)
		{
			CalculateVP(cam);
			SetShader(S_SHADOW);
			LinkFrameBufferSettings(FB_NIL, F_CLEAR);
			LoadSettings(GPUSetting::SHADOW);
			RenderDirectionalShadowMaps(cam);

			SetShader(S_DEFERRED);
			if(cam == mCurrentCamIDHover)
				LinkFrameBufferSettings(FB_DEFERRED, F_ID_POS_NOM_TEX);
			else
				LinkFrameBufferSettings(FB_DEFERRED, F_POS_NOM_TEX);
			LoadSettings(GPUSetting::DEFAULT);
			UpdateCamVP();
			BindCameraDepth(cam);
			ClearBuffer(BufferClearSetting::ALL);
			Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Render(mCurrShader.second, true);

			SetShader(S_LIGHTING);
			LinkFrameBufferSettings(FB_FINAL, F_TEX);
			UpdateCamVP();
			BindCameraDepth(cam);
			ClearBuffer(BufferClearSetting::COLOR_ONLY);
			LightingRender(cam);
			
			if (Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag)
			{
				LoadSettings(GPUSetting::DEBUG);
				RenderDebug(cam);
			}

			LoadSettings(GPUSetting::DEFAULT);
			GammaCorrectionRender(cam);
		}
		
		mObjPickedThisFrame = false;
		LinkFrameBufferSettings(FB_TOTAL, F_CLEAR);
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

			auto& frustrum = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::FRUSTRUM_DEFAULT).get();
			//auto& frustrum = Core::GetInstance()->GetResourceManager()->GetModel("FrustrumFake");
			auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>();
			for (auto& entity : cams)
			{
				if (entity == cam) continue;

				auto& transform = Core::GetInstance()->GetRegistry().get<Transform>(entity);
				auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(entity);

				transform.transform = glm::mat4x4(1.f);
				transform.transform = glm::translate(transform.transform, transform.position);
				glm::mat4 Rot = glm::mat4_cast(transform.rotation);

				//glm::mat4x4 Rot = glm::eulerAngleXYZ(glm::radians(transform.rotation.x), glm::radians(transform.rotation.y + 90.f), glm::radians(transform.rotation.z));
				transform.transform *= Rot;
				transform.transform = glm::scale(transform.transform, transform.scale);

				glm::vec3 dirFacing = Rot * glm::vec4(0.f, 0.f, 1.f, 1.f);

				float tanT = tanf(glm::radians(camera.pov) * 0.5f);

				float nn = camera.near;
				float nh = nn * tanT;
				float nw = nh / camera.height * camera.width;
				float ff = camera.far;
				float fh = ff * tanT;
				float fw = fh / camera.height * camera.width;

				frustrum.vtx[0] = glm::vec3(-nw, -nh, nn);
				frustrum.vtx[1] = glm::vec3(nw, -nh, nn);
				frustrum.vtx[2] = glm::vec3(nw, nh, nn);
				frustrum.vtx[3] = glm::vec3(-nw, nh, nn);
				frustrum.vtx[4] = glm::vec3(-nw, -nh, nn);

				frustrum.vtx[5] = glm::vec3(-fw, -fh, ff);// C
				frustrum.vtx[6] = glm::vec3(fw, -fh, ff);
				frustrum.vtx[7] = glm::vec3(fw, fh, ff);
				frustrum.vtx[8] = glm::vec3(-fw, fh, ff);
				frustrum.vtx[9] = glm::vec3(-fw, -fh, ff);

				frustrum.vtx[10] = glm::vec3(fw, -fh, ff);
				frustrum.vtx[11] = glm::vec3(nw, -nh, nn); // C
				frustrum.vtx[12] = glm::vec3(nw, nh, nn);
				frustrum.vtx[13] = glm::vec3(fw, fh, ff); // C
				frustrum.vtx[14] = glm::vec3(-fw, fh, ff);
				frustrum.vtx[15] = glm::vec3(-nw, nh, nn); // C

				// Frustrum Rendering
				glNamedBufferSubData(frustrum.meshes[0].vbo, 0, frustrum.vtx.size() * sizeof(glm::vec3), frustrum.vtx.data());

				glNamedBufferSubData(mIVBO, 0, sizeof(glm::mat4), &transform.transform[0][0]);

				glBindVertexArray(frustrum.meshes[0].vao);
				glDrawArraysInstanced(frustrum.meshes[0].drawMode, 0, frustrum.meshes[0].drawCnt, 1);
			}
		}
		
		// Draw Instance Debug Box
		if (Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & DEBUG_OBJ_TAG)
		{
			SetShader(S_INSTANCED);
			UpdateCamVP();
			BindCameraDepth(cam);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::CUBE_DEFAULT).get();
			auto& mdl = model.meshes[0];	//i call it mdl cuz im lazy to change the below
			glBindVertexArray(mdl.vao);

			auto view = Core::GetInstance()->GetRegistry().view<renderEntity>(); //renderEntity
			int num{}, offset{};
			for (auto entity : view)
			{
				auto& transform = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity);
				mInstanceVtx[num] = glm::scale(transform.transform, glm::vec3(1.01f, 1.01f, 1.01f));
				num++;
				if (num == mMaxInstance)
				{
					glNamedBufferSubData(mIVBO, 0, sizeof(glm::mat4) * num, mInstanceVtx.data() + offset);
					glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, num);
					offset += num;
				}
			}
			glNamedBufferSubData(mIVBO, 0, sizeof(glm::mat4) * num, mInstanceVtx.data() + offset);
			glDrawElementsInstanced(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr, num);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	void RenderManager::LightingRender(Entity cam)
	{
		glBindTextureUnit(0, mColAttachment[GPU_OUT::GOUT_DIF]);
		glBindTextureUnit(1, mColAttachment[GPU_OUT::GOUT_POS]);
		glBindTextureUnit(2, mColAttachment[GPU_OUT::GOUT_NOM]);

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
				LoadSettings(GPUSetting::ADDITION);

				uniformLoc = glGetUniformLocation(mCurrShader.second, "uLight.direction");
				glUniform3f(uniformLoc, -lightT.position.x, -lightT.position.y, -lightT.position.z);

				SetDirectionalLightMtx(camT.position, lightT.position);

				glBindTextureUnit(3, light.depthTex);

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
					LoadSettings(GPUSetting::ADDITION);
				else
					LoadSettings(GPUSetting::SPE_ADDITION);

				glm::mat4 M{ 1.f };
				M = glm::translate(M, lightT.position);
				M = glm::scale(M, glm::vec3(pointLightFar, pointLightFar, pointLightFar));
				uniformLoc = glGetUniformLocation(mCurrShader.second, "M");
				glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &M[0][0]);

				glBindTextureUnit(4, light.shadowCubeMap);

				auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT);
				auto& mesh = mdl.get()->meshes[0];
				glBindVertexArray(mesh.vao);
				//glDrawArrays(mdl.get()->drawMode, 0, mdl.get()->drawCnt);
				glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				break;
			}
			}
		}
	}
	void RenderManager::GammaCorrectionRender(Entity cam)
	{
		SetShader(ShaderOpt::S_FINAL);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Core::GetInstance()->GetRegistry().get<Camera>(cam).textureID, 0); // GL_COLOR_ATTACHMENT0 - First Out
		ClearBuffer(BufferClearSetting::ALL);
		glBindTextureUnit(0, mColAttachment[GPU_OUT::GOUT_FINAL]);

		auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT).get();
		auto& mdl = model.meshes[0];	//i call it mdl cuz im lazy to change the below
		glBindVertexArray(mdl.vao);
		//glDrawArrays(mdl.get()->drawMode, 0, mdl.get()->drawCnt);
		glDrawElements(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr);

		//auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT);
		//glBindVertexArray(mdl.get()->vao);
		//glDrawElements(mdl.get()->drawMode, mdl.get()->drawCnt, GL_UNSIGNED_INT, nullptr);
	}
#pragma endregion

#pragma region Rendering Helpers
	// Only need to calculate for camera once
	void RenderManager::CalculateVP(Entity cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		auto& camTrans = Core::GetInstance()->GetRegistry().get<Transform>(cam);

		glm::vec3 target{ 1.f, 0.f, 0.f }, up{ 0.f, 1.f, 0.f };
		glm::mat3 rot = glm::mat3_cast(camTrans.rotation);

		//glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));
		glm::vec3 forward = camTrans.rotation * glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f);


		V = glm::lookAt(camTrans.position, camTrans.position + forward , upVec);

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
	void RenderManager::LinkFrameBufferSettings(FBOType fbo, FBOSet settings)
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
		switch (fbo)
		{
		case FB_DEFERRED:
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (settings & FBOSet::F_TEX) ? mColAttachment[GOUT_DIF] : 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, (settings & FBOSet::F_ID) ? mColAttachment[GOUT_ID] : 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, (settings & FBOSet::F_POS) ? mColAttachment[GOUT_POS] : 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, (settings & FBOSet::F_NOM) ? mColAttachment[GOUT_NOM] : 0, 0);
			break;
		case FB_FINAL:
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (settings & FBOSet::F_TEX) ? mColAttachment[GOUT_FINAL] : 0, 0);
			break;
		}
	}
	// Does all the glEnable and Disables etc. Maybe can be more streamlined to check if certain settings are already there?
	void RenderManager::LoadSettings(GPUSetting setting)
	{
		if (mCurrGPUSetting == setting)
			return;

		switch (mCurrGPUSetting)
		{
		case GPUSetting::DEFAULT:
		{
			break;
		}
		case GPUSetting::SHADOW:
		{
			glCullFace(GL_BACK);
			break;
		}
		case GPUSetting::DEBUG:
		{
			glEnable(GL_CULL_FACE);

			glDisable(GL_BLEND);
			break;
		}
		case GPUSetting::ADDITION:
			__fallthrough;
		case GPUSetting::SPE_ADDITION:
		{
			glEnable(GL_DEPTH_TEST);
			//glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
			break;
		}
		}
		mCurrGPUSetting = setting;
		switch (mCurrGPUSetting)
		{
		case GPUSetting::DEFAULT:
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			break;
		}
		case GPUSetting::SHADOW:
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			break;
		}
		case GPUSetting::DEBUG:
		{
			glDisable(GL_CULL_FACE);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
		case GPUSetting::ADDITION:
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			glDisable(GL_DEPTH_TEST);
			//glEnable(GL_DEPTH_TEST);
			//glDepthFunc(GL_LESS);
			//glDepthMask(GL_FALSE);

			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		}
		case GPUSetting::SPE_ADDITION:
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glDisable(GL_DEPTH_TEST);
			//glEnable(GL_DEPTH_TEST);
			//glDepthFunc(GL_GEQUAL);
			//glDepthMask(GL_FALSE);

			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		}
		}
	}
	// Changes Shader if not current
	void RenderManager::SetShader(ShaderOpt sh)
	{
		if (sh != mCurrShader.first)
		{
			mCurrShader.first = sh;
			mCurrShader.second = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)sh).get()->s;
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
	// Sets this up at the start to bind slots 12~15 with the instance transform :p
	void RenderManager::LinkTransformInstancing(GUID guid)
	{
		//std::string tempFilePath = "Assets/Models/" + mdlName + ".txt";
		auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>(guid).get();
		auto& mdl = model.meshes[0];	//i call it mdl cuz im lazy to change the below

		// auto& mdl = Core::GetInstance()->GetResourceManager()->GetModel(mdlName);

		// Link drawing models with instancing vbo
		for (int i{}; i < 4; ++i)
		{
			glBindVertexArray(mdl.vao);
			int idx = 12 + i; // 12 ~ 15
			glEnableVertexArrayAttrib(mdl.vao, idx);
			glVertexArrayVertexBuffer(mdl.vao, idx, mIVBO, sizeof(glm::vec4) * i, sizeof(glm::mat4));
			glVertexArrayAttribIFormat(mdl.vao, idx, 4, GL_FLOAT, 0);
			glVertexArrayAttribBinding(mdl.vao, idx, idx);

			glVertexAttribDivisor(idx, 1);
		}
		glBindVertexArray(0);
	}
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