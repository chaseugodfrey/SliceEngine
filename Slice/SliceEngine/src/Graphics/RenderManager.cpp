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
	constexpr static inline uint64_t basicShaderGUID = 18310719961107313904;
	constexpr static inline uint64_t instancedShaderGUID = 17697828682138082227;
	constexpr static inline uint64_t debugLineShaderGUID = 13567802095736790143;

#pragma region Generate GPU Objects
	RenderManager::RenderManager()
	{
		CreateFramebuffer();
	}
	RenderManager::~RenderManager()
	{
		glDeleteFramebuffers(1, &mFBO);
		glDeleteBuffers(1, &mIVBO);
		glDeleteBuffers(1, &mDebugLineVBO);

		glDeleteTextures(1, &mColAttachment[0]);
		glDeleteTextures(1, &mColAttachment[1]);
		glDeleteTextures(1, &mColAttachment[2]);
		glDeleteTextures(1, &mColAttachment[3]);

		glDeleteBuffers(2, pboIds);
	}
	void RenderManager::CreateFramebuffer()
	{
		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

		unsigned int drawBuffers[] = {
			GL_COLOR_ATTACHMENT0
			,GL_COLOR_ATTACHMENT1
			,GL_COLOR_ATTACHMENT2
			,GL_COLOR_ATTACHMENT3
			,GL_COLOR_ATTACHMENT4
		};
		glDrawBuffers(sizeof(drawBuffers) / sizeof(unsigned int), drawBuffers); // -TODO- Check if this part links the frame buffer or texture


		//glGenRenderbuffers(1, &mRBO);
		//glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Core::GetInstance()->GetSystem<CameraSystem>().maxWidth, Core::GetInstance()->GetSystem<CameraSystem>().maxHeight);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);

		// Note: Framebuffer is always going to be incomplete this way due to not attaching a texture to it
		//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		//{
		//	SLICE_LOG_WARNING("Framebuffer not complete");
		//}

		glGenBuffers(2, pboIds);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[0]);
		glBufferData(GL_PIXEL_PACK_BUFFER, 4, 0, GL_STREAM_READ);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[1]);
		glBufferData(GL_PIXEL_PACK_BUFFER, 4, 0, GL_STREAM_READ);
		pboIdx[0] = 0;
		pboIdx[1] = 1;


		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		LinkFrameBufferSettings(FBOSetting::UNBIND);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	void RenderManager::CreateInstancingParams()
	{
		mInstanceShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)instancedShaderGUID);
		//mInstanceShader = Core::GetInstance()->GetResourceManager()->GetShader("instanced");
		mInstanceVtx.resize(mMaxInstance);
		glCreateBuffers(1, &mIVBO);
		glNamedBufferStorage(mIVBO, mInstanceVtx.size() * sizeof(glm::mat4), mInstanceVtx.data(), GL_DYNAMIC_STORAGE_BIT);
		LinkTransformInstancing((GUID)DefaultResourceIDs::CUBE_DEFAULT);
		LinkTransformInstancing((GUID)DefaultResourceIDs::FRUSTRUM_DEFAULT);

		// Make Debug Line VBO
		mDebugLineShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)debugLineShaderGUID);
		std::vector<glm::vec3> mDebugLines;
		mDebugLines.reserve(mMaxInstance);
		// offset, scale, if rotate
		float lineScale = mMaxInstance / 2.f;
		mDebugLines.emplace_back(glm::vec3(0.f, lineScale, 0.f));
		mDebugLines.emplace_back(glm::vec3(0.f, lineScale, 1.f));
		int counter{2};
		for (int i{}; counter + 4 < mMaxInstance; ++i)
		{
			mDebugLines.emplace_back(glm::vec3(i, lineScale, 0.f));
			mDebugLines.emplace_back(glm::vec3(-i, lineScale, 0.f));
			mDebugLines.emplace_back(glm::vec3(i, lineScale, 1.f));
			mDebugLines.emplace_back(glm::vec3(-i, lineScale, 1.f));
			counter += 4;
		}
		glCreateBuffers(1, &mDebugLineVBO);
		glNamedBufferStorage(mDebugLineVBO, mDebugLines.size() * sizeof(glm::vec3), mDebugLines.data(), GL_MAP_WRITE_BIT);
		LinkDebugLineInstancing();
	}
	void RenderManager::CreateDeferredTextures()
	{
		// Entity ID
		glCreateTextures(GL_TEXTURE_2D, 1, &mColAttachment[0]);
		glTextureStorage2D(mColAttachment[0], 1, GL_R32UI, Core::GetInstance()->GetSystem<CameraSystem>().maxWidth, Core::GetInstance()->GetSystem<CameraSystem>().maxHeight);
		glTextureParameterf(mColAttachment[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// Pos
		glCreateTextures(GL_TEXTURE_2D, 1, &mColAttachment[1]);
		glTextureStorage2D(mColAttachment[1], 1, GL_RGB16F, Core::GetInstance()->GetSystem<CameraSystem>().maxWidth, Core::GetInstance()->GetSystem<CameraSystem>().maxHeight);
		glTextureParameterf(mColAttachment[1], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[1], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// Nom
		glCreateTextures(GL_TEXTURE_2D, 1, &mColAttachment[2]);
		glTextureStorage2D(mColAttachment[2], 1, GL_RGB16F, Core::GetInstance()->GetSystem<CameraSystem>().maxWidth, Core::GetInstance()->GetSystem<CameraSystem>().maxHeight);
		glTextureParameterf(mColAttachment[2], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[2], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// Nom
		glCreateTextures(GL_TEXTURE_2D, 1, &mColAttachment[3]);
		glTextureStorage2D(mColAttachment[3], 1, GL_RGB16F, Core::GetInstance()->GetSystem<CameraSystem>().maxWidth, Core::GetInstance()->GetSystem<CameraSystem>().maxHeight);
		glTextureParameterf(mColAttachment[3], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameterf(mColAttachment[3], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
		// MAYDO: has issue when deleting the cam game object, causing the mainCam to become Empty

		return newCam;
	}
	
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

		LinkFrameBufferSettings(FBOSetting::BIND);
		IDPick();

		auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>();
		for (auto cam : cams)
		{
			CalculateVP(cam);

			mCurrShader = Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().UseShader();
			if(cam == mCurrentCamIDHover)
				LinkFrameBufferSettings(FBOSetting::ID_POS_NOM_TEX);
			else
				LinkFrameBufferSettings(FBOSetting::POS_NOM_TEX);
			LoadSettings(GPUSetting::DEFAULT);
			UpdateCamGPU(cam);
			ClearBuffer(BufferClearSetting::ALL);

			GLint uniformLoc;
			if (UniformExists("uPass", uniformLoc))
				glUniform1i(uniformLoc, 0);
			Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Render(cam);

			DeferredRender();
			
			if(Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag)
				RenderDebug(cam);
		}

		mObjPickedThisFrame = false;
		LinkFrameBufferSettings(FBOSetting::UNBIND);
		std::swap(pboIdx[0], pboIdx[1]);
	}
	void RenderManager::RenderDebug(Entity& cam)
	{
		glEnable(GL_DEPTH_TEST);

		// Draw other cameras' frustrum
		if(Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & DEBUG_FRUSTRUM_TAG)
		{
			mCurrShader = mInstanceShader;
			glUseProgram(mCurrShader.get()->s);
			UpdateCamGPU(cam);

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
			mCurrShader = mInstanceShader;
			glUseProgram(mCurrShader.get()->s);
			UpdateCamGPU(cam);

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
					glDrawArraysInstanced(mdl.drawMode, 0, mdl.drawCnt, num);
					offset += num;
				}
			}
			glNamedBufferSubData(mIVBO, 0, sizeof(glm::mat4) * num, mInstanceVtx.data() + offset);
			glDrawArraysInstanced(GL_LINES, 0, mdl.drawCnt, num);
		}

		// Draw Debug Line
		if(Core::GetInstance()->GetRegistry().get<Camera>(cam).renderTag & DEBUG_GRID_TAG)
		{
			mCurrShader = mDebugLineShader;
			glUseProgram(mCurrShader.get()->s);
			UpdateCamGPU(cam);
			auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::LINE_DEFAULT).get();
			auto& mdl = model.meshes[0];	//i call it mdl cuz im lazy to change the below
			
			GLint uniformLoc;
			if(UniformExists("uPosOffset", uniformLoc))
				glUniform2f(uniformLoc, 0.0f, 0.0f);
			if(UniformExists("uScale", uniformLoc))
				glUniform1f(uniformLoc, 1.0f);

			glBindVertexArray(mdl.vao);
			glDrawArraysInstanced(mdl.drawMode, 0, mdl.drawCnt, ((mMaxInstance - 2) / 4) * 4 + 2);
		}
	}
	void RenderManager::DeferredRender()
	{
		glDisable(GL_DEPTH_TEST);

		GLint uniformLoc;
		if (UniformExists("uPass", uniformLoc))
			glUniform1i(uniformLoc, 1);

		Core::GetInstance()->GetSystem<LightingSystem>().SetLightingParams(mCurrShader->s);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);//glColorMaski(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, 0, 0);
		glBindTextureUnit(0, mColAttachment[3]);
		glBindTextureUnit(1, mColAttachment[1]);
		glBindTextureUnit(2, mColAttachment[2]);


		auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT).get();
		auto& mdl = model.meshes[0];	//i call it mdl cuz im lazy to change the below
		glBindVertexArray(mdl.vao);
		//glDrawArrays(mdl.get()->drawMode, 0, mdl.get()->drawCnt);
		glDrawElements(mdl.drawMode, mdl.drawCnt, GL_UNSIGNED_INT, nullptr);

		//auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)1001);
		//glBindVertexArray(mdl.get()->vao);
		//glDrawElements(GL_TRIANGLES, mdl.get()->drawCnt, GL_UNSIGNED_INT, 0);
	}
#pragma endregion

#pragma region Rendering Helpers
	void RenderManager::CalculateVP(Entity& cam)
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
	void RenderManager::UpdateCamGPU(Entity& cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, camera.textureID, 0); // GL_COLOR_ATTACHMENT0 - First Out
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, camera.depthTex, 0);

		//scuffed hack
		//auto const& shader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>("Assets/Shaders/basic.txt");

		GLint uniformLoc;
		if (UniformExists("V", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &V[0][0]);
		if (UniformExists("P", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &P[0][0]);

		glViewport(0, 0, camera.width, camera.height);
	}
	bool RenderManager::UniformExists(const char* str, GLint& ref)
	{
		ref = glGetUniformLocation(mCurrShader.get()->s, str);
		if (ref >= 0)
			return true;

		std::stringstream ss;
		ss << "Uniform variable: " << str << " doesn't exist!!!\n";
		SLICE_LOG_WARNING(ss.str());
		return false;
	}
#pragma endregion

#pragma region Linking
	void RenderManager::LinkFrameBufferSettings(FBOSetting settings)
	{
		switch (settings)
		{
		case FBOSetting::UNBIND:
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			break;
		}
		case FBOSetting::BIND:
		{
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
			break;
		}
		case FBOSetting::COLOR_ONLY:
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
			break;
		}
		case FBOSetting::ID:
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mColAttachment[0], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
			break;
		}
		case FBOSetting::POS_NOM:
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, mColAttachment[1], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, mColAttachment[2], 0);
			break;
		}
		case FBOSetting::POS_NOM_TEX:
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, mColAttachment[1], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, mColAttachment[2], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, mColAttachment[3], 0);
			break;
		}
		case FBOSetting::ID_POS_NOM:
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mColAttachment[0], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, mColAttachment[1], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, mColAttachment[2], 0);
			break;
		}
		case FBOSetting::ID_POS_NOM_TEX:
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mColAttachment[0], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, mColAttachment[1], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, mColAttachment[2], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, mColAttachment[3], 0);
			break;
		}
		}
	}
	void RenderManager::LoadSettings(GPUSetting setting)
	{
		switch (setting)
		{
		case GPUSetting::DEFAULT:
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
		}
	}
	void RenderManager::ClearBuffer(BufferClearSetting setting)
	{
		switch (setting)
		{
		case BufferClearSetting::ALL:
		{
			glClearBufferfv(GL_COLOR, 0, zeroFiller);
			glClearBufferfv(GL_COLOR, 1, zeroFiller);
			glClearBufferfv(GL_COLOR, 2, zeroFiller);
			glClearBufferfv(GL_COLOR, 3, zeroFiller);
			glClearBufferfv(GL_COLOR, 4, zeroFiller);
			glClearBufferfv(GL_DEPTH, 0, oneFiller);
			__fallthrough;
		}
		case BufferClearSetting::DEFAULT:
		{
			//glClearColor(0.75294f, 1.f, 0.93333f, 1.f);
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		}
		}
	}
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
	void RenderManager::LinkDebugLineInstancing()
	{
		//std::string tempFilePath = "Assets/Models/" + mdlName + ".txt";
		auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::LINE_DEFAULT).get();
		auto& mdl = model.meshes[0];	//i call it mdl cuz im lazy to change the below

		glBindVertexArray(mdl.vao);
		int idx = 15;
		glEnableVertexArrayAttrib(mdl.vao, idx);
		glVertexArrayVertexBuffer(mdl.vao, idx, mDebugLineVBO, 0, sizeof(glm::vec3));
		glVertexArrayAttribIFormat(mdl.vao, idx, 3, GL_FLOAT, 0);
		glVertexArrayAttribBinding(mdl.vao, idx, idx);

		glVertexAttribDivisor(idx, 1);
		glBindVertexArray(0);
	}
#pragma endregion

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
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
		glActiveTexture(GL_TEXTURE1);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mColAttachment[0], 0);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIdx[0]]);
		
		glNamedFramebufferReadBuffer(mFBO, GL_COLOR_ATTACHMENT1);
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
}