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

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Resource/Model.h"

// My Comments to (Ctrl + f): TODO: MAYDO:

namespace SliceEngine
{
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

		//glDeleteBuffers(2, pboIds);
	}
	void RenderManager::CreateFramebuffer()
	{
		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

		//glGenRenderbuffers(1, &mScenes[i].RBO);
		//glBindRenderbuffer(GL_RENDERBUFFER, mScenes[i].RBO);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mScenes[i].RBO);

		// Note: Framebuffer is always going to be incomplete this way due to not attaching a texture to it
		//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		//{
		//	SLICE_LOG_WARNING("Framebuffer not complete");
		//}

		//glGenBuffers(2, pboIds);
		//glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[0]);
		//glBufferData(GL_PIXEL_PACK_BUFFER, 4, 0, GL_STREAM_READ);
		//glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[1]);
		//glBufferData(GL_PIXEL_PACK_BUFFER, 4, 0, GL_STREAM_READ);
		//pboIdx[0] = 0;
		//pboIdx[1] = 1;


		//glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		LinkFrameBufferSettings(FBOSetting::UNBIND);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	void RenderManager::CreateInstancingParams()
	{
		mInstanceShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>("Assets/Shaders/instanced.txt");
		//mInstanceShader = Core::GetInstance()->GetResourceManager()->GetShader("instanced");
		mInstanceVtx.resize(mMaxInstance);
		glCreateBuffers(1, &mIVBO);
		glNamedBufferStorage(mIVBO, mInstanceVtx.size() * sizeof(glm::mat4), mInstanceVtx.data(), GL_DYNAMIC_STORAGE_BIT);
		LinkTransformInstancing("CubeWireframe");

		// Make Debug Line VBO
		mDebugLineShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>("Assets/Shaders/debugLine.txt");
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
		LinkDebugLineInstancing("Line");
	}
	void RenderManager::CreateDeferredTextures()
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &mColAttachment[0]);
		glTextureStorage2D(mColAttachment[0], 1, GL_RGB16F, Core::GetInstance()->GetSystem<CameraSystem>().maxWidth, Core::GetInstance()->GetSystem<CameraSystem>().maxHeight);
		glTextureParameterf(mColAttachment[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameterf(mColAttachment[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glCreateTextures(GL_TEXTURE_2D, 1, &mColAttachment[1]);
		glTextureStorage2D(mColAttachment[1], 1, GL_RG16F, Core::GetInstance()->GetSystem<CameraSystem>().maxWidth, Core::GetInstance()->GetSystem<CameraSystem>().maxHeight);
		glTextureParameterf(mColAttachment[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameterf(mColAttachment[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	
	GameObject& RenderManager::CreateCamera()
	{
		GameObject newCam = Core::GetInstance()->mFactory.CreateEO();
		
		auto& transform = newCam.GetComponent<Transform>();
		transform.position = glm::vec3(-2.f, 1.f, 0.f);
		transform.rotation = glm::vec3(0.f, 0.f, -10.f);
		newCam.AddComponent<Camera>();

		// MAYDO: has issue when deleting the cam game object, causing the mainCam to become Empty
		if (!mainCam.has_value())
			mainCam = newCam.GetEntity();
		return newCam;
	}
	Entity& RenderManager::GetMainCamera()
	{
		return mainCam.value();
	}
	void RenderManager::GetCameraAxis(const Entity& cam, glm::vec3& forward, glm::vec3& right, glm::vec3& up)
	{
		glm::vec3 f{ 1.f, 0.f, 0.f }, u{ 0.f, 1.f, 0.f }, r{ 0.f,0.f,1.f };
		auto& camTrans = Core::GetInstance()->GetRegistry().get<Transform>(cam);
		glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));
		forward = rot * f;
		right = rot * r;
		up = rot * u;
	}
	
	void RenderManager::IDPick(const int& mouseX, const int& mouseY)
	{
		//// if out of bounds
		//if (mouseX > 1920 || mouseX < 0 || mouseY > 1080 || mouseY < 0)
		//{
		//	mIDHovered = std::numeric_limits<unsigned int>().max();
		//	return;
		//}

		//GLint prevBinding{};
		//glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevBinding);

		//glBindFramebuffer(GL_FRAMEBUFFER, mScene.FBO);
		//glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIdx[0]]);

		//glNamedFramebufferReadBuffer(mScene.FBO, GL_COLOR_ATTACHMENT1);
		//glReadPixels(mouseX, mouseY, 1, 1,
		//	GL_RED_INTEGER, GL_UNSIGNED_INT, 0);

		//glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIdx[1]]);
		//GLuint* src = (GLuint*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		//if (src)
		//{
		//	mIDHovered = *src;
		//	glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		//}
		////if (RenderHelper::GetInstance()->mEditorWindowActive)
		////	RenderHelper::GetInstance()->mSelectedID = goID;
		//if (mIDHovered >= std::numeric_limits<unsigned int>().max())
		//	mIDHovered = 0;

		//glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		//glReadBuffer(GL_NONE);
		//glBindFramebuffer(GL_FRAMEBUFFER, prevBinding);
		//return;
	}

	void RenderManager::Render()
	{
		Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Update(0.f);

		LinkFrameBufferSettings(FBOSetting::BIND);
		//IDPick(mousePosX, mousePosY);

		auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>();
		for (auto cam : cams)
		{
			CalculateVP(cam);

			mCurrShader = Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().UseShader();
			UpdateCamGPU(cam);
			LinkFrameBufferSettings(FBOSetting::COLOR_POS_NOM);
			glClearBufferfv(GL_COLOR, 1, zeroFiller);
			glClearBufferfv(GL_COLOR, 2, zeroFiller);

			GLint uniformLoc;
			if (UniformExists("uPass", uniformLoc))
				glUniform1i(uniformLoc, 0);
			Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Render(cam);

			glUniform1i(uniformLoc, 1);
			DeferredRender();

			RenderDebug(cam);
		}

		LinkFrameBufferSettings(FBOSetting::UNBIND);
		//std::swap(pboIdx[0], pboIdx[1]);
	}
	void RenderManager::RenderDebug(Entity& cam)
	{
		glDisable(GL_DEPTH_TEST);

		// Draw other cameras' frustrum
		{
			auto& frustrum = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>("Assets/Models/FrustrumFake.txt").get();
			//auto& frustrum = Core::GetInstance()->GetResourceManager()->GetModel("FrustrumFake");
			auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>();
			for (auto& entity : cams)
			{
				if (entity == cam) continue;

				auto& transform = Core::GetInstance()->GetRegistry().get<Transform>(entity);
				auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(entity);

				transform.transform = glm::mat4x4(1.f);
				transform.transform = glm::translate(transform.transform, transform.position);
				glm::mat4x4 Rot = glm::eulerAngleXYZ(glm::radians(transform.rotation.x), glm::radians(transform.rotation.y + 90.f), glm::radians(transform.rotation.z));
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
				glNamedBufferSubData(frustrum.vbo, 0, frustrum.vtx.size() * sizeof(glm::vec3), frustrum.vtx.data());

				GLint uniformLoc;
				if (UniformExists("M", uniformLoc))
					glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &transform.transform[0][0]);

				glBindVertexArray(frustrum.vao);
				glDrawArrays(frustrum.drawMode, 0, frustrum.drawCnt);
			}
		}
		
		// Draw Instance Debug Box
		{
			mCurrShader = mInstanceShader;
			glUseProgram(mCurrShader.get()->s);
			UpdateCamGPU(cam);

			auto view = Core::GetInstance()->GetRegistry().view<renderEntity>(); //renderEntity
			int num{};
			for (auto entity : view)
			{
				auto& transform = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity);
				mInstanceVtx[num] = glm::scale(transform.transform, glm::vec3(1.01f, 1.01f, 1.01f));
				num++;
			}
			glNamedBufferSubData(mIVBO, 0, sizeof(glm::mat4) * num, mInstanceVtx.data());
			//auto& mdl = Core::GetInstance()->GetResourceManager()->GetModel("CubeWireframe");
			auto& mdl = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>("Assets/Models/CubeWireframe.txt").get();
			glBindVertexArray(mdl.vao);
			glDrawArraysInstanced(mdl.drawMode, 0, mdl.drawCnt, num);
		}

		// Draw Debug Line
		{
			mCurrShader = mDebugLineShader;
			glUseProgram(mCurrShader.get()->s);
			UpdateCamGPU(cam);
			auto& mdl = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>("Assets/Models/Line.txt").get();
			
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
		glm::mat4 idx{ 1.f };
		GLint uniformLoc;
		if (UniformExists("V", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &idx[0][0]);
		if (UniformExists("M", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &idx[0][0]);
		idx = glm::scale(idx, glm::vec3(2.f, 2.f, 2.f));
		if (UniformExists("P", uniformLoc))
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &idx[0][0]);

		if (UniformExists("uLight[0].position", uniformLoc))
		glUniform3f(uniformLoc, 5, 1.f, 5.f);
		if (UniformExists("uLight[0].La", uniformLoc))
		glUniform3f(uniformLoc, 0.2f, 0.2f, 0.2f);
		if (UniformExists("uLight[0].Ld", uniformLoc))
		glUniform3f(uniformLoc, 0.5f, 0.5f, 0.5f);
		if (UniformExists("uLight[0].Ls", uniformLoc))
		glUniform3f(uniformLoc, 1.f, 1.f, 1.f);

		if (UniformExists("uMat.Ka", uniformLoc))
		glUniform3f(uniformLoc, 1.f, 1.f, 1.f);
		if (UniformExists("uMat.Kd", uniformLoc))
		glUniform3f(uniformLoc, 1.f, 1.f, 1.f);
		if (UniformExists("uMat.Ks", uniformLoc))
		glUniform3f(uniformLoc, 1.f, 1.f, 1.f);
		if (UniformExists("uMat.shininess", uniformLoc))
		glUniform1f(uniformLoc, 1.f);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mColAttachment[0]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mColAttachment[1]);

		auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>("Assets/Models/Quad.txt");
		glBindVertexArray(mdl.get()->vao);
		glDrawArrays(mdl.get()->drawMode, 0, mdl.get()->drawCnt);
	}

	void RenderManager::CalculateVP(Entity& cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		auto& camTrans = Core::GetInstance()->GetRegistry().get<Transform>(cam);

		glm::vec3 target{ 1.f, 0.f, 0.f }, up{ 0.f, 1.f, 0.f };
		glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));

		V = glm::lookAt(camTrans.position, camTrans.position + rot * target, rot * up);
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
			unsigned int drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(sizeof(drawBuffers) / sizeof(unsigned int), drawBuffers); // TODO: Check if this part links the frame buffer or texture
			break;
		}
		case FBOSetting::COLOR_POS_NOM:
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, mColAttachment[0], 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, mColAttachment[1], 0);
			unsigned int drawBuffers[] = {
				GL_COLOR_ATTACHMENT0
				,GL_COLOR_ATTACHMENT1
				,GL_COLOR_ATTACHMENT2
			};
			glDrawBuffers(sizeof(drawBuffers) / sizeof(unsigned int), drawBuffers); // TODO: Check if this part links the frame buffer or texture
			break;
		}
		}
	}
	void RenderManager::LinkTransformInstancing(const std::string& mdlName)
	{
		std::string tempFilePath = "Assets/Models/" + mdlName + ".txt";
		auto& mdl = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>(tempFilePath).get();
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
	void RenderManager::LinkDebugLineInstancing(const std::string& mdlName)
	{
		std::string tempFilePath = "Assets/Models/" + mdlName + ".txt";
		auto& mdl = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>(tempFilePath).get();

		glBindVertexArray(mdl.vao);
		int idx = 15;
		glEnableVertexArrayAttrib(mdl.vao, idx);
		glVertexArrayVertexBuffer(mdl.vao, idx, mDebugLineVBO, 0, sizeof(glm::vec3));
		glVertexArrayAttribIFormat(mdl.vao, idx, 3, GL_FLOAT, 0);
		glVertexArrayAttribBinding(mdl.vao, idx, idx);

		glVertexAttribDivisor(idx, 1);
		glBindVertexArray(0);
	}
}