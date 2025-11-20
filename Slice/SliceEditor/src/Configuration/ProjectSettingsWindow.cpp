#include "pch.h"
#include "ProjectSettingsWindow.h"
#include "Core/Registry.h"
#include "Configuration/ProjectSettings.h"
#include "Inspector/ComponentPropertiesGUI.h"

#include <Core/Core.h>
#include <Physics/PhysicsSystem.h>
#include <Systems/LayerManager.h>

namespace SliceEditor
{
	void ProjectSettingsWindow::Init()
	{
		mSettingsList.push_back(std::make_unique<AudioSettingsDisplay>(mRegistry, "Audio"));
		mSettingsList.push_back(std::make_unique<PhysicsSettingsDisplay>(mRegistry, "Physics"));
		mSettingsList.push_back(std::make_unique<ProjectSettingsDisplay>(mRegistry, "Project"));
	}

	void ProjectSettingsWindow::Draw()
	{
		ImVec2 window_size = ImVec2(800, 600);
		ImGui::SetNextWindowSize(window_size);
		bool isOpen;
		if (ImGui::Begin("Project Settings Window", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
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


			ImVec2 right_size = ImVec2(window_size.x * 0.9f, window_size.y);
			ImGui::BeginChild("##right_group", right_size, ImGuiChildFlags_Borders);
			mCurrentSettings->DisplayHeader();
			mCurrentSettings->DisplaySettings();
			ImGui::EndChild();
		}

		if (!isOpen)
			markForRemoval = true;

		ImGui::End();

	}
	void BaseSettingsDisplay::DisplayHeader()
	{
		ImGui::PushFont(NULL, ImGui::GetFontSize() * 1.25f);
		ImGui::Text(name.c_str());
		ImGui::PopFont();
	}

	void AudioSettingsDisplay::DisplaySettings()
	{
		// Master Volume
		float float_buffer{};
		DragFloatInputHeader(mRegistry, "Master Volume", "##master_vol", float_buffer);

		// SFX list
		static int count = 1;
		ImGui::Text("SFX List");
		ImGui::BeginChild("##sfx_list", ImVec2(), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		if (ImGui::TreeNodeEx("list", ImGuiTreeNodeFlags_Framed))
		{
			for (int i = 0; i < count; i++)
			{
				// temp buffer
				// Key entry name
				std::string string_buffer = "entry_" + std::to_string(i);
				float float_buffer{};
				bool bool_buffer{};
				if (ImGui::TreeNodeEx(string_buffer.c_str(), ImGuiTreeNodeFlags_Framed))
				{
					StringInputHeader(mRegistry, "Key", ("##key_" + string_buffer).c_str(), string_buffer);
					DragFloatInputHeader(mRegistry, "Volume", ("##vol_" + string_buffer).c_str(), float_buffer);
					DragFloatInputHeader(mRegistry, "Max Instances", ("##maxInstances_" + string_buffer).c_str(), float_buffer);
					BoolInputHeader(mRegistry, "Is 3D", ("##is3D_" + string_buffer).c_str(), bool_buffer);
					DragFloatInputHeader(mRegistry, "Spatial Blend", ("##spatialBlend_" + string_buffer).c_str(), float_buffer);
					DragFloatInputHeader(mRegistry, "Min Distance", ("##minDistance" + string_buffer).c_str(), float_buffer);
					DragFloatInputHeader(mRegistry, "Max Distance", ("##maxDistance" + string_buffer).c_str(), float_buffer);
					DragFloatInputHeader(mRegistry, "Interval", ("##interval" + string_buffer).c_str(), float_buffer);
					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::Button("+"))
		{
			count++;
		}
		ImGui::SameLine();
		if (ImGui::Button("-"))
		{
			count--;
		}

		ImGui::EndChild();
	}

	void PhysicsSettingsDisplay::DisplaySettings()
	{
		// Retrieve variables
		auto layerManager = SliceEngine::Core::GetInstance()->GetLayerManager();
		auto& physicsSystem = SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PhysicsSystem>();
		auto& matrixMap = layerManager->nameToLayer; 
		
		std::vector<std::string> layerNames{};
		layerNames.reserve(matrixMap.size());
		for (auto& [name, layer] : matrixMap)
			layerNames.push_back(name);

		const int n = static_cast<int>(layerNames.size());

		if (ImGui::CollapsingHeader("Broad Phase Layer Optimization"))
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Optimization for layers. GameObject Layers in different Broad Phase layers will be filtered out to increase the optimization of collision detection.");
			}

			static std::vector<std::string> bplayer_to_name_list{  "Non-moving","Moving" };

			if (ImGui::BeginTable("##bplayer", 2, ImGuiTableFlags_Borders))
			{
				ImGui::TableSetupColumn("Layer",
					ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_WidthFixed);

				ImGui::TableSetupColumn("BP Layer",
					ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_WidthFixed);

				for (size_t i = 0; i < n; i++)
				{
					std::string layerName = layerNames[i];
					auto layer = matrixMap.at(layerName);
					auto bp_layer = physicsSystem.GetBroadPhaseLayer(layer);
					auto bp_layer_index = bp_layer.GetValue();

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					ImGui::Text(layerName.c_str());

					ImGui::TableSetColumnIndex(1);

					if (ComboHeader(mRegistry, "", ("##bp_" + layerName).c_str(), bp_layer_index, bplayer_to_name_list))
					{
						JPH::BroadPhaseLayer new_bp_layer(bp_layer_index);
						physicsSystem.SetObjectBroadPhaseLayer(layer, new_bp_layer);
					}
				}

				ImGui::EndTable();
			}
		}


		// 
		if (ImGui::CollapsingHeader("Layer Collision Matrix"))
		{
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Set the interaction between each layer to determine if objects in the column layer collide with objects in row layer.");
			}

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
						bool collides = layerManager->CheckLayerInteraction(colName, rowName);

						ImGui::TableSetColumnIndex(col + 1);
						ImGui::PushID((row << 16) | logicalColIndex);

						// checkbox
						if (ImGui::Checkbox("##cell", &collides))
						{
							layerManager->AssignLayerInteraction(colName, rowName, collides);
						}

						ImGui::PopID();
					}
				}

				ImGui::EndTable();
			}

		}
	}

	void ProjectSettingsDisplay::DisplaySettings()
	{
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
}
