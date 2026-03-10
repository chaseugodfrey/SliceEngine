#include <pch.h>
#include "SkyboxSettings.h"
#include "Core/Core.h"
#include "Graphics/RenderManager.h"
#include <Serializer/JSONSerializer.h>

namespace SliceEngine
{
	void SkyboxSettings::LoadSettings(nlohmann::json settings)
	{
		auto& skyboxData = SliceEngine::Core::GetInstance()->GetRenderManager()->skyboxData;
		
		if (settings.contains("Lighting Power"))
			skyboxData.lightingPower = settings["Lighting Power"].get<float>();
		
		if (settings.contains("Zenith Color"))
			from_json(settings["Zenith Color"], skyboxData.zenithColor);

		if(settings.contains("Horizon Color"))
			from_json(settings["Horizon Color"], skyboxData.horizonColor);

		if(settings.contains("Ground Color"))
			from_json(settings["Ground Color"], skyboxData.groundColor);

		if(settings.contains("Sun Position"))
			from_json(settings["Sun Position"], skyboxData.sunPos);

		if(settings.contains("Sun Color"))
			from_json(settings["Sun Color"], skyboxData.sunCol);
	}

	void SkyboxSettings::SaveSettings()
	{
		std::ofstream outFile{ filepath };
		nlohmann::json skyboxSettingsJson;
		
		auto& skyboxData = SliceEngine::Core::GetInstance()->GetRenderManager()->skyboxData;

		to_json(skyboxSettingsJson["Lighting Power"], skyboxData.lightingPower);
		to_json(skyboxSettingsJson["Zenith Color"], skyboxData.zenithColor);
		to_json(skyboxSettingsJson["Horizon Color"], skyboxData.horizonColor);
		to_json(skyboxSettingsJson["Ground Color"], skyboxData.groundColor);
		to_json(skyboxSettingsJson["Sun Position"], skyboxData.sunPos);
		to_json(skyboxSettingsJson["Sun Color"], skyboxData.sunCol);

		outFile << skyboxSettingsJson.dump(4);
		
		outFile.close();
	}

	void SkyboxSettings::Init()
	{

	}

	void SkyboxSettings::Exit()
	{

	}

	void SkyboxSettings::ApplySettings()
	{

	}
}