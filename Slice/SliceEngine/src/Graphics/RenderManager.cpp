#include <pch.h>
#include "RenderManager.h"
#define PI05F 1.57079632679f
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

#include "Core/Core.h"

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

		//glDeleteBuffers(2, pboIds);
	}
	GameObject& RenderManager::CreateCamera()
	{
		GameObject newCam = Core::GetInstance()->mFactory.CreateEO();
		
		auto& transform = newCam.GetComponent<Transform>();
		transform.position = glm::vec3(-2.f, 0.f, 0.f);
		
		newCam.AddComponent<Camera>();

		// MAYDO: has issue when deleting the cam game object, causing the mainCam to become Empty
		if (!mainCam.has_value())
			mainCam = newCam.GetEntity();
		return newCam;
	}

	void RenderManager::Render()
	{
		Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Update(0.f);

		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
		//IDPick(mousePosX, mousePosY);

		auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>();
		for (auto cam : cams)
		{
			mCurrShader = Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().UseShader();
			UpdateCamGPU(cam);

			Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Render(cam);
			RenderDebug(cam);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//std::swap(pboIdx[0], pboIdx[1]);
	}

	void RenderManager::RenderDebug(Entity& cam)
	{
		auto& frustrum = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>("Assets/Models/FrustrumFake.txt").get();
		//auto& frustrum = Core::GetInstance()->GetResourceManager()->GetModel("FrustrumFake");
		
		auto cams = Core::GetInstance()->GetRegistry().view<cameraEntity>();
		for (auto entity : cams)
		{
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

			GLint uniformLoc = glGetUniformLocation(/*mCurrShader.s*/ mCurrShader.get()->s, "M");
			glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &transform.transform[0][0]);

			glBindVertexArray(frustrum.vao);
			glDrawArrays(frustrum.drawMode, 0, frustrum.drawCnt);
		}
		
		mCurrShader = mInstanceShader;
		//glUseProgram(mCurrShader.s);
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

	void RenderManager::UpdateCamGPU(Entity& cam)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(cam);
		auto& camTrans = Core::GetInstance()->GetRegistry().get<Transform>(cam);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, camera.textureID, 0); // GL_COLOR_ATTACHMENT0 - First Out
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, camera.depthTex, 0);

		glm::vec3 target{ 1.f, 0.f, 0.f }, up{ 0.f, 1.f, 0.f };
		glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));

		glm::mat4 V = glm::lookAt(camTrans.position, camTrans.position + rot * target, rot * up);

		glm::mat4 P = glm::perspective(glm::radians(camera.pov), static_cast<float>(camera.width) / static_cast<float>(camera.height), camera.near, camera.far);

		//scuffed hack
		auto const& shader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>("Assets/Shaders/basic.txt");

		GLint uniformLoc;

		uniformLoc = glGetUniformLocation(/*rcManager->GetShader().s*/shader.get()->s, "V");
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &V[0][0]);
		uniformLoc = glGetUniformLocation(/*rcManager->GetShader().s*/shader.get()->s, "P");

		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &P[0][0]);

		glViewport(0, 0, camera.width, camera.height);
	}

	void RenderManager::CreateFramebuffer()
	{
		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

		//glGenRenderbuffers(1, &mScenes[i].RBO);
		//glBindRenderbuffer(GL_RENDERBUFFER, mScenes[i].RBO);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mScenes[i].RBO);

		unsigned int drawBuffers[] = {
			GL_COLOR_ATTACHMENT0
			//,GL_COLOR_ATTACHMENT1
		};
		glDrawBuffers(sizeof(drawBuffers) / sizeof(unsigned int), drawBuffers); // TODO: Check if this part links the frame buffer or texture

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
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	void RenderManager::CreateInstancingParams()
	{
		mInstanceShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>("Assets/Shaders/instanced.txt");
		//mInstanceShader = Core::GetInstance()->GetResourceManager()->GetShader("instanced");
		mInstanceVtx.resize(100);
		glCreateBuffers(1, &mIVBO);
		glNamedBufferStorage(mIVBO, mInstanceVtx.size() * sizeof(glm::mat4), mInstanceVtx.data(), GL_DYNAMIC_STORAGE_BIT);

		LinkInstancing("CubeWireframe");
	}
	void RenderManager::LinkInstancing(const std::string& mdlName)
	{
		std::string tempFilePath = "Assets/Models/" + mdlName + ".txt";
		auto& mdl = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>(tempFilePath).get();

//		auto& mdl = Core::GetInstance()->GetResourceManager()->GetModel(mdlName);

		// Link drawing models with instancing vbo
		for (int i{}; i < 4; ++i)
		{
			glBindVertexArray(mdl.vao);
			int idx = 12 + i; // 13 ~ 16
			glEnableVertexArrayAttrib(mdl.vao, idx);
			glVertexArrayVertexBuffer(mdl.vao, idx, mIVBO, sizeof(glm::vec4) * i, sizeof(glm::mat4));
			glVertexArrayAttribIFormat(mdl.vao, idx, 4, GL_FLOAT, 0);
			glVertexArrayAttribBinding(mdl.vao, idx, idx);

			glVertexAttribDivisor(idx, 1);
		}
		glBindVertexArray(0);
	}
	GLuint RenderManager::GetTexture()
	{
		return Core::GetInstance()->GetRegistry().get<Camera>(mainCam.value()).textureID;
	}
	Transform& RenderManager::GetMainCameraTransform()
	{
		return Core::GetInstance()->GetRegistry().get<Transform>(mainCam.value());
	}
	void RenderManager::GetMainCameraAxis(glm::vec3& forward, glm::vec3& right, glm::vec3& up)
	{
		glm::vec3 f{ 1.f, 0.f, 0.f }, u{ 0.f, 1.f, 0.f }, r{ 0.f,0.f,1.f };
		auto& camTrans = Core::GetInstance()->GetRegistry().get<Transform>(mainCam.value());
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
}