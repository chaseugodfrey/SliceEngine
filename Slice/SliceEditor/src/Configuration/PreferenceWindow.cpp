#include <pch.h>
#include "PreferenceWindow.h"
#include "Core/Registry.h"
#include "Configuration/PreferenceManager.h"
#include <Inspector/ComponentPropertiesGUI.h>

namespace SliceEditor
{
	ImVec4 operator* (const ImVec4 v, const float f)
	{
		return ImVec4(std::clamp(v.x * f, 0.f, 255.f), (std::clamp(v.y * f, 0.f, 255.f)), (std::clamp(v.z * f, 0.f, 255.f)), (std::clamp(v.w * f, 0.f, 255.f)));
	}

	//float* ImVec4ToFloatArray(ImVec4 v)
	//{
	//	float arr[4]
	//	{
	//		v.x,
	//		v.y,
	//		v.z,
	//		v.w
	//	};

	//	return arr;
	//}

	ImVec4 FloatArrayToImVec4(float* arr)
	{
		return ImVec4(arr[0], arr[1], arr[2], arr[3]);
	}

	void PreferenceWindow::Init()
	{
		mPreferenceList.push_back(std::make_unique<ThemePreferenceDisplay>(mRegistry, "Theme"));
		mPreferenceList.push_back(std::make_unique<ScenePreferenceDisplay>(mRegistry, "Scene"));
	}

	void PreferenceWindow::Draw()
	{
		ImVec2 window_size = ImVec2(800, 600);
		ImGui::SetNextWindowSize(window_size);
		bool isOpen;

		if (ImGui::Begin("Preferences", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			//auto gSettings = SliceEngine::Core::GetInstance()->GetProjectSettingsService();

			auto& mPreferences = mRegistry.GetManager<PreferenceManager>("Preferences")->GetPreferences();

			ImVec2 left_size = ImVec2(window_size.x * 0.1f, window_size.y);
			if (ImGui::BeginChild("##left_group", left_size, ImGuiChildFlags_Borders))
			{
				for (size_t i = 0; i < mPreferenceList.size(); i++)
				{
					auto settings = mPreferenceList[i].get();
					if (ImGui::Selectable(settings->name.c_str()))
					{
						mCurrentPreferenceIndex = (PreferenceType)i;
					}
				}
			}
			ImGui::EndChild();

			auto& mCurrentPreference = mPreferenceList[(size_t)mCurrentPreferenceIndex];

			ImGui::SameLine();

			ImVec2 right_size = ImVec2(window_size.x * 0.9f, window_size.y);
			ImGui::BeginChild("##right_group", right_size, ImGuiChildFlags_Borders);
			mCurrentPreference->DisplayHeader();
			mCurrentPreference->DisplayPreferences(mPreferences);
			if (ImGui::Button("Save Changes"))
			{
				mRegistry.GetManager<PreferenceManager>("Preferences")->SavePreferences(false);
			}
			ImGui::EndChild();
		}

		if (!isOpen)
			markForRemoval = true;

		ImGui::End();
	}

	void BasePreferenceDisplay::DisplayHeader()
	{
		ImGui::PushFont(NULL, ImGui::GetFontSize() * 1.25f);
		ImGui::Text(name.c_str());
		ImGui::PopFont();
	}

	void ThemePreferenceDisplay::DisplayPreferences(Preferences& preferences)
	{
		ImGui::SeparatorText("Themes");
		
		if (ImGui::BeginCombo("##Theme", EditorThemes[preferences.theme.ID]))
		{
			for (int i = 0; i < EditorThemes.size(); i++)
			{
				if (ImGui::Selectable(EditorThemes[i]))
				{
					preferences.theme.ID = EditorThemeType(i);
					EditorUtilities::SetTheme(preferences.theme.ID);
				}
			}

			ImGui::EndCombo();
		}
	}

	void ScenePreferenceDisplay::DisplayPreferences(Preferences& preferences)
	{
		auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();

		ImGui::SeparatorText("Scene");

		auto startingSceneHandle = resourceMgr->get<SliceEngine::SliceEngineTypes::Scene>(preferences.scene.startingID);
		auto startingSceneString = startingSceneHandle.IsValid() ? startingSceneHandle->GetFilePath().string() : "None.";

		if (HandleDragDropInputHeader(mRegistry, "Startup Scene", "##startup_scene", startingSceneHandle, "Scene"))
		{
			preferences.scene.startingID = startingSceneHandle.getGUID();
		}
		
	}
}