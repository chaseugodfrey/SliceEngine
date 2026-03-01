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
		auto scenes = build["Scenes"].get<std::vector<std::pair<uint64_t, std::string>>>();

		for (uint32_t i = 0; i < scenes.size(); i++)
		{
			SceneEntry entry;
			auto guid = static_cast<SliceEngine::GUID>(scenes[i].first);
			entry.handle = resourceManager->get<SliceEngineTypes::Scene>(guid);

			// check if filename has extension/is filepath
			std::string filename = scenes[i].second;
			std::filesystem::path path = filename;

			if (path.has_extension())
				filename = path.stem().string();

			entry.filename = filename;

			mSceneList.push_back(entry);
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

		//auto resourceManager = Core::GetInstance()->GetResourceManager();

		auto& scenes = json["Build"]["Scenes"];

		for (auto const& entry : mSceneList)
		{
			scenes.push_back({ entry.handle.getGUID().GetGUID(), entry.filename });
		}

		file << json.dump(4);
		file.close();
	}

	void BuildSettings::ApplySettings()
	{

	}

	Handle<SliceEngineTypes::Scene> BuildSettings::GetSceneHandleByIndex(size_t index)
	{
		if (index < mSceneList.size())
			return mSceneList[index].handle;

		return Handle<SliceEngineTypes::Scene>();
	}

	Handle<SliceEngineTypes::Scene> BuildSettings::GetSceneHandleByName(const std::string& name)
	{
		for (auto const& entry : mSceneList)
		{
			if (entry.filename == name)
				return entry.handle;
		}

		return Handle<SliceEngineTypes::Scene>();
	}
}