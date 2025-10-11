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
		int i{1};
		bool hasDirLight {false};
		std::stringstream ss{};
		GLint uniformLoc;
		for (auto entity : view)
		{
			auto& light = Core::GetInstance()->GetRegistry().get<Light>(entity);
			auto& transform = Core::GetInstance()->GetRegistry().get<Transform>(entity);

			int lightNum = i;
			if (light.type == Light::LightType::Directional)
			{
				lightNum = 0;
				hasDirLight = true;
			}

			ss.str("");
			ss << "uLight[" << lightNum << "].position";
			uniformLoc = glGetUniformLocation(shader, ss.str().c_str());
			glUniform3f(uniformLoc, transform.position.x, transform.position.y, transform.position.z);

			ss.str("");
			ss << "uLight[" << lightNum << "].color";
			uniformLoc = glGetUniformLocation(shader, ss.str().c_str());
			glm::vec3 col = light.color * light.intensity;
			glUniform3f(uniformLoc, col.r, col.g, col.b);

			if (i++ > 32)
				break;
		}
		uniformLoc = glGetUniformLocation(shader, "numLights");
		if(hasDirLight)
			glUniform1i(uniformLoc, i - 1);
		else
			glUniform1i(uniformLoc, i);

	}
}