#include "pch.h"
#include "ProjectSettingsWindow.h"
#include "Configuration/ProjectSettings.h"

#include <Core/Core.h>
#include <Physics/PhysicsSystem.h>
#include <Systems/LayerManager.h>

namespace SliceEditor
{
	void ProjectSettingsWindow::Init()
	{
		mSettingsList.push_back(std::make_unique<DAudioSettings>("Audio"));
		mSettingsList.push_back(std::make_unique<DPhysicsSettings>("Physics"));
		mSettingsList.push_back(std::make_unique<DProjectSettings>("Project"));
	}

	void ProjectSettingsWindow::Draw()
	{
		ImVec2 window_size = ImVec2(800, 600);
		ImGui::SetNextWindowSize(window_size);
		bool isOpen;
		if (ImGui::Begin("Project Settings Window", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
		{
			auto gSettings = SliceEngine::Core::GetInstance()->GetProjectSettingsService();

			ImVec2 left_size = ImVec2(window_size.x * 0.1f, window_size.y);
			if (ImGui::BeginChild("##left_group", left_size, ImGuiChildFlags_Borders))
			{
				for (size_t i = 0; i < mSettingsList.size(); i++)
				{
					auto settings = mSettingsList[i].get();
					if (ImGui::Selectable(settings->name.c_str()))
					{
						mCurrentSettingsIndex = (SettingsType)i;
					}
				}
			}
			ImGui::EndChild();

			//
			//

			auto& mCurrentSettings = mSettingsList[(size_t)mCurrentSettingsIndex];

			ImGui::SameLine();


			ImVec2 right_size = ImVec2(ImGui::GetWindowWidth() - left_size.x, ImGui::GetContentRegionAvail().y);
			ImGui::BeginChild("##right_group", right_size);
			mCurrentSettings->DisplayHeader();
			mCurrentSettings->DisplaySettings();
			ImGui::EndChild();

			//auto& s = gSettings->Edit(); // we�ll set dirty only if something changes

			//bool changed = false;
			//if (ImGui::InputText("Product Name", &s.productName)) { changed = true; }
			//int w = s.width, h = s.height;
			//if (ImGui::InputInt("Width", &w)) { s.width = std::max(16, w); changed = true; }
			//if (ImGui::InputInt("Height", &h)) { s.height = std::max(16, h); changed = true; }

			//// Scenes list (very basic)
			//for (size_t i = 0;i < s.scenes.size();++i) {
			//	ImGui::PushID((int)i);
			//	ImGui::InputText("Scene Path", &s.scenes[i]); // path string edit
			//	if (ImGui::SmallButton("Up") && i > 0) { std::swap(s.scenes[i], s.scenes[i - 1]); changed = true; }
			//	ImGui::SameLine();
			//	if (ImGui::SmallButton("Down") && i + 1 < s.scenes.size()) { std::swap(s.scenes[i], s.scenes[i + 1]); changed = true; }
			//	ImGui::SameLine();
			//	if (ImGui::SmallButton("X")) { s.scenes.erase(s.scenes.begin() + i); changed = true; ImGui::PopID(); break; }
			//	ImGui::PopID();
			//}
			//if (ImGui::Button("+ Add Scene")) { s.scenes.emplace_back("Assets/Scenes/New.scene"); changed = true; }

			//// Startup scene combo
			//if (!s.scenes.empty()) {
			//	int current = 0;
			//	for (int i = 0;i < (int)s.scenes.size();++i) if (s.scenes[i] == s.startupScene) current = i;
			//	if (ImGui::BeginCombo("Startup Scene", s.scenes[current].c_str())) {
			//		for (int i = 0;i < (int)s.scenes.size();++i) {
			//			bool sel = (i == current);
			//			if (ImGui::Selectable(s.scenes[i].c_str(), sel)) { s.startupScene = s.scenes[i]; changed = true; }
			//		}
			//		ImGui::EndCombo();
			//	}
			//}

			//// Save/Reload row
			//if (ImGui::Button("Save")) gSettings->Save();
			//ImGui::SameLine();
			//if (ImGui::Button("Reload")) { gSettings->Load(); }

			//// Set dirty timing + optional autosave
			//if (changed) {
			//	// touching Edit() already marked dirty; reset the debounce timer by re-setting the change time
			//	// simplest: mark as dirty again; DebouncedAutosave accumulates time each frame
			//}

			//gSettings->DebouncedAutosave(1.0f / 60.0f, /*delay*/0.75);

			//// External change detection (prompt)
			//if (gSettings->DetectExternalChange()) {
			//	ImGui::TextDisabled("ProjectSettings.json changed on disk.");
			//	ImGui::SameLine();
			//	if (ImGui::Button("Reload from Disk")) gSettings->Load();
			//}

		}

		if (!isOpen)
			markForRemoval = true;

		ImGui::End();

	}
	void ProjectSettingsWindow::DSettings::DisplayHeader()
	{
		ImGui::PushFont(NULL, ImGui::GetFontSize() * 20.0f);
		ImGui::Text(name.c_str());
		ImGui::PopFont();
	}

	void ProjectSettingsWindow::DAudioSettings::DisplaySettings()
	{

	}

	void ProjectSettingsWindow::DPhysicsSettings::DisplaySettings()
	{
		// Retrieve variables
		auto mgr = SliceEngine::Core::GetInstance()->GetLayerManager();
		auto& matrixMap = mgr->collisionMask; 

		// 
		ImGui::Text("Layer Collision Matrix");

		// 1. Build an indexable list of layer names
		std::vector<std::string> layerNames;
		layerNames.reserve(matrixMap.size());
		for (auto& [name, layer] : matrixMap)
			layerNames.push_back(name);

		const int n = static_cast<int>(layerNames.size());
		const int columns_count = n + 1; // 1 for row header + n for each layer

		if (ImGui::BeginTable("##collision_matrix", columns_count, ImGuiTableFlags_None))
		{
			ImGui::TableSetupColumn("all_layers",
				ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_WidthFixed);

			for (int col = 0; col < n; ++col)
			{
				// to flip so that the collision matrix looks like this
				//    e d c b a
				//  a x x x x x
				//  b x x x x 
				//  c x x x
				//  d x x
				//  e x

				int logicalColIndex = n - 1 - col;               
				const std::string& colName = layerNames[logicalColIndex];
				ImGui::TableSetupColumn(colName.c_str(), column_flags);
			}

			ImGui::TableAngledHeadersRow();

			for (int row = 0; row < n; ++row)
			{
				const std::string& rowName = layerNames[row];

				// Row set up + label
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(rowName.c_str());

				// Row details
				for (int col = 0; col < n - row; ++col)
				{
					int logicalColIndex = n - 1 - col;           // flip columns
					const std::string& colName = layerNames[logicalColIndex];
					bool collides = mgr->CheckLayerInteraction(colName, rowName);

					ImGui::TableSetColumnIndex(col + 1);
					ImGui::PushID((row << 16) | logicalColIndex);

					// checkbox
					if (ImGui::Checkbox("##cell", &collides))
					{
						bool check = mgr->CheckLayerInteraction(colName, rowName);
						mgr->AssignLayerInteraction(colName, rowName, collides);
					}

					ImGui::PopID();
				}
			}

			ImGui::EndTable();
		}
	}
	void ProjectSettingsWindow::DProjectSettings::DisplaySettings()
	{
	}
}
