#include <pch.h>
#include "PhysicsSettings.h"
#include "Core/Core.h"
#include "Systems/LayerManager.h"

namespace SliceEngine
{
	void PhysicsSettings::LoadSettings(nlohmann::json settings)
	{
		auto layerManager = SliceEngine::Core::GetInstance()->GetLayerManager();
		std::string matrix = settings["Layer Collision Matrix"].get<std::string>();

		for (uint32_t i = 0; i < 32; i++)
		{
			for (uint32_t j = 0; j < 32 - i; j++)
			{
				bool interact = static_cast<bool>(*(matrix.begin() + 32 * i + j));
				auto row_name = layerManager->GetLayerName(i);
				auto col_name = layerManager->GetLayerName(j);
				layerManager->AssignLayerInteraction(row_name, col_name, interact);
			}
		}
	}

	void PhysicsSettings::SaveSettings()
	{
		std::ofstream outFile{ filepath };
		nlohmann::json physicsSettingsJson;

		std::string matrix{};

		auto layerManager = SliceEngine::Core::GetInstance()->GetLayerManager();

		for (auto& mask : layerManager->collisionMask)
		{
			matrix += std::to_string(mask.second);
		}

		physicsSettingsJson["Layer Collision Matrix"] = matrix;

		physicsSettingsJson >> outFile;

		outFile.close();
	}

	void PhysicsSettings::Init()
	{

	}

	void PhysicsSettings::Exit()
	{

	}

	void PhysicsSettings::ApplySettings()
	{

	}
}