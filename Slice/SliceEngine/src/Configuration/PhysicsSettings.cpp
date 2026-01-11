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

	}

	void PhysicsSettings::SaveSettings()
	{
		std::ofstream outFile{ filepath };
		nlohmann::json physicsSettingsJson;

		std::stringstream ss{};
		std::string buffer{};
		std::string matrix{};

		auto layerManager = SliceEngine::Core::GetInstance()->GetLayerManager();
		
		for (auto& [layer, name] : layerManager->indexToLayerName)
		{
			auto& mask = layerManager->collisionMask[name];
			ss << std::hex << mask;
			buffer = ss.str();
			SLICE_LOG(buffer);
			matrix += buffer;
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