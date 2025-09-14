#include <pch.h>
#include "RenderManager.h"
#define PI05F 1.57079632679f
#include <glm.hpp>
#include <common.hpp>
#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/euler_angles.hpp"

#include "../Core/Core.h"

namespace SliceEngine
{
	RenderManager::RenderManager(){	}
	RenderManager::~RenderManager()
	{
		glDeleteFramebuffers(1, &mFBO);
		//glDeleteBuffers(2, pboIds);
	}
	void RenderManager::InitAndLink(GLFWwindow* window)
	{

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		CreateFramebuffer(width, height);

		mainCam = Core::GetInstance()->GetRegistry().create();
		Core::GetInstance()->GetRegistry().emplace<Transform>(mainCam, glm::vec3(-2.f,0.f,0.f), glm::vec3(0.f, 0.f, 0.f));
		Core::GetInstance()->GetRegistry().emplace<Camera>(mainCam);
	}

	void RenderManager::Render(GLFWwindow* window, ResourceManager* rcManager)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
		//IDPick(mousePosX, mousePosY);

		Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().UseShader(rcManager);
		UpdateCamGPU(window, rcManager);
		
		Core::GetInstance()->GetSystem<WorldSpaceGraphicsSystem>().Render(window, rcManager);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//std::swap(pboIdx[0], pboIdx[1]);
	}

	void RenderManager::UpdateCamGPU(GLFWwindow* window, ResourceManager* rcManager)
	{
		auto& camera = Core::GetInstance()->GetRegistry().get<Camera>(mainCam);
		auto& camTrans = Core::GetInstance()->GetRegistry().get<Transform>(mainCam);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, camera.textureID, 0); // GL_COLOR_ATTACHMENT0 - First Out
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, camera.depthTex, 0);

		glm::vec3 target{ 1.f, 0.f, 0.f }, up{ 0.f, 1.f, 0.f };
		glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));

		glm::mat4 V = glm::lookAt(camTrans.position, camTrans.position + rot * target, rot * up);

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		glm::mat4 P = glm::perspective(glm::radians(camera.pov), static_cast<float>(width) / static_cast<float>(height), camera.near, camera.far);


		GLint uniformLoc;
		uniformLoc = glGetUniformLocation(rcManager->GetShader().s, "V");
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &V[0][0]);
		uniformLoc = glGetUniformLocation(rcManager->GetShader().s, "P");
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &P[0][0]);
	}

	void RenderManager::CreateFramebuffer(int width, int height)
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


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			SLICE_LOG_WARNING("Framebuffer not complete");
		}

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
	GLuint RenderManager::GetTexture()
	{
		return Core::GetInstance()->GetRegistry().get<Camera>(mainCam).textureID;
	}
	Transform& RenderManager::GetMainCameraTransform()
	{
		return Core::GetInstance()->GetRegistry().get<Transform>(mainCam);
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