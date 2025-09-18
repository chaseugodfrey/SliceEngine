#include <pch.h>
#include "ResourceManager.h"
#include "WorldSpaceGraphicsSystem.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/euler_angles.hpp"

namespace SliceEngine
{
	void WorldSpaceGraphicsSystem::UseShader(ResourceManager* rcManager)
	{
		mShader = rcManager->GetShader();
		glUseProgram(mShader.s);
	}
	void WorldSpaceGraphicsSystem::Render(ResourceManager* rcManager)
	{
		glClearColor(0.75294f, 1.f, 0.93333f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		tempModel = rcManager->GetModel();

		Update(1.0f);
	}

	void WorldSpaceGraphicsSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{

	}

	void WorldSpaceGraphicsSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{

	}

	void WorldSpaceGraphicsSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		glBindVertexArray(tempModel.vao);

		auto& transform = reg.get<Transform>(entity);

		glm::mat4x4 M(1.f);
		M = glm::translate(M, transform.position);
		M *= glm::eulerAngleXYZ(glm::radians(transform.rotation.x), glm::radians(transform.rotation.y), glm::radians(transform.rotation.z));
		M = glm::scale(M, transform.scale);

		transform.transform = M;
		// Transformation code for child - continuing from parent
		//M = glm::translate(M, glm::vec3(2.f, -2.f, 2.f));
		//glm::mat4x4 M2 = glm::eulerAngleXYZ(glm::radians(45.f), glm::radians(0.f), glm::radians(0.f));
		//M2 = glm::scale(M2, glm::vec3(0.5f, 0.5f, 0.5f));
		//M = M2 * M;

		GLint uniformLoc;
		uniformLoc = glGetUniformLocation(mShader.s, "M");
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &M[0][0]);

		glDrawArrays(tempModel.drawMode, 0, tempModel.drawCnt);
	}

}