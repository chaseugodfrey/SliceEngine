#include <pch.h>
#include "LightingSystem.h"

#include "../Core/Core.h"

namespace SliceEngine
{
	void LightingSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& light = reg.get<Light>(entity);

		// Setup Basic Camera Components
		light.color = glm::vec3(1.0f, 1.0f, 1.0f);
		light.intensity = 0.5f;
	}
	void LightingSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{

	}
	void LightingSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{

	}
	void LightingSystem::SetLightingParams(GLuint shader)
	{
		auto view = Core::GetInstance()->GetRegistry().view<lightingEntity>();
		int i{};
		std::stringstream ss{};
		for (auto entity : view)
		{
			auto& light = Core::GetInstance()->GetRegistry().get<Light>(entity);
			auto& transform = Core::GetInstance()->GetRegistry().get<Transform>(entity);

			GLint uniformLoc;
			ss.str("");
			ss << "uLight[" << i << "].position";
			uniformLoc = glGetUniformLocation(shader, ss.str().c_str());
			glUniform3f(uniformLoc, transform.position.x, transform.position.y, transform.position.z);

			ss.str("");
			ss << "uLight[" << i << "].color";
			uniformLoc = glGetUniformLocation(shader, ss.str().c_str());
			glm::vec3 col = light.color * light.intensity;
			glUniform3f(uniformLoc, col.r, col.g, col.b);

			++i;
		}
	}
}