#include <pch.h>
#include "PhysicsSettings.h"
#include "Core/Core.h"
#include "Systems/LayerManager.h"
#include "Physics/PhysicsSystem.h"

namespace SliceEngine
{
	void PhysicsSettings::LoadSettings(nlohmann::json settings)
	{
		auto layerManager = SliceEngine::Core::GetInstance()->GetLayerManager();
		auto physicsSystem = &SliceEngine::Core::GetInstance()->GetSystem<PhysicsSystem>();

		std::string matrix = settings["Broad Layer Collision Matrix"].get<std::string>();

		for (size_t i = 0; i < matrix.size(); i++)
		{
			std::string chunk = matrix.substr(i, 1);
			JPH::BroadPhaseLayer::Type bpLayer = std::stoi(chunk);
			physicsSystem->SetObjectBroadPhaseLayer(static_cast<uint32_t>(i), JPH::BroadPhaseLayer(bpLayer));
		}

		matrix = settings["Layer Collision Matrix"].get<std::string>();

		std::stringstream ss{};
		std::string buffer{};
		auto ptr = layerManager->indexToLayerName.begin();

		for (size_t i = 0; i < matrix.size(); i+=8)
		{
			std::string chunk = matrix.substr(i, 8);
			
			auto& mask = layerManager->collisionMask[ptr->second];
			std::from_chars(chunk.data(), chunk.data() + chunk.size(), mask, 16);
			ptr++;
		}
	}

	void PhysicsSettings::SaveSettings()
	{
		std::ofstream outFile{ filepath };
		nlohmann::json physicsSettingsJson;

		auto layerManager = SliceEngine::Core::GetInstance()->GetLayerManager();
		auto physicsSystem = &SliceEngine::Core::GetInstance()->GetSystem<PhysicsSystem>();

		std::stringstream ss{};
		std::string matrix{};

		for (auto& [layer, name] : layerManager->indexToLayerName)
		{
			auto const& mask = physicsSystem->GetBroadPhaseLayer(layer).GetValue();
			matrix += std::to_string(mask);
		}

		physicsSettingsJson["Broad Layer Collision Matrix"] = matrix;
		matrix.clear();

		for (auto& [layer, name] : layerManager->indexToLayerName)
		{
			auto& mask = layerManager->collisionMask[name];
			ss << std::hex << mask;
			matrix += ss.str();
			ss.str(std::string());
		}

		physicsSettingsJson["Layer Collision Matrix"] = matrix;

		outFile << physicsSettingsJson.dump(4);

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