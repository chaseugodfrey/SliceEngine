#include <pch.h>
#include <Core/Core.h>
#include "BuildSettings.h"

namespace SliceEngine
{
	void BuildSettings::LoadSettings(nlohmann::json settings)
	{
		if (!settings.contains("Build"))
		{
			SLICE_LOG_ERROR("App does not contain Build Settings.");
			return;
		}
		
		auto resourceManager = SliceEngine::Core::GetInstance()->GetResourceManager();

		auto const& build = settings.at("Build");
		auto scenes = build["Scenes"].get<std::vector<uint64_t>>();

		for (uint32_t i = 0; i < scenes.size(); i++)
		{
			auto guid = static_cast<SliceEngine::GUID>(scenes[i]);
			auto handle = resourceManager->get<SliceEngineTypes::Scene>(guid);

			if (handle.IsValid())
				mSceneList.push_back(handle);
		}
	}

	void BuildSettings::Init()
	{

	}

	void BuildSettings::Exit()
	{

	}

	void BuildSettings::SaveSettings()
	{
		std::ofstream file(filepath);
		nlohmann::json json;

		auto resourceManager = Core::GetInstance()->GetResourceManager();

		std::vector<uint64_t> scenes{};

		for (auto const& handle : mSceneList)
		{
			scenes.push_back(handle.getGUID().GetGUID());
		}

		json["Build"] =
		{
			{ "Scenes", scenes }
		};

		file << json.dump(4);
		file.close();
	}

	void BuildSettings::ApplySettings()
	{

	}
}