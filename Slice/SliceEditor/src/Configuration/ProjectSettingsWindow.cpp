#include "pch.h"
#include "ProjectSettingsWindow.h"
#include "Core/Registry.h"
#include "Configuration/ProjectSettings.h"
#include "Configuration/AudioSettings.h"
#include "Inspector/ComponentPropertiesGUI.h"

#include <Core/Core.h>
#include <Configuration/ProjectSettingsManager.h>
#include <Physics/PhysicsSystem.h>
#include <Audio/AudioManager.h>
#include <Systems/LayerManager.h>

namespace SliceEditor
{
	void ProjectSettingsWindow::Init()
	{
		auto* settingsManager = SliceEngine::Core::GetInstance()->GetProjectSettingsManager();
		mSettingsList.push_back(std::make_unique<AudioSettingsDisplay>(mRegistry, *settingsManager->GetSettings<SliceEngine::AudioSettings>(), "Audio"));
		mSettingsList.push_back(std::make_unique<PhysicsSettingsDisplay>(mRegistry, *settingsManager->GetSettings<SliceEngine::PhysicsSettings>(), "Physics"));
		//mSettingsList.push_back(std::make_unique<ProjectSettingsDisplay>(mRegistry, "Project"));
	}

	void ProjectSettingsWindow::Draw()
	{
		ImVec2 window_size = ImVec2(800, 600);
		ImGui::SetNextWindowSize(window_size);
		bool isOpen;
		if (ImGui::Begin("Project Settings Window", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
		{
			//auto gSettings = SliceEngine::Core::GetInstance()->GetProjectSettingsService();

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
		SliceEngine::AudioSettings* audioSettings = SliceEngine::Core::GetInstance()->GetProjectSettingsManager()->GetSettings<SliceEngine::AudioSettings>();
		auto audioManager = SliceEngine::Core::GetInstance()->GetAudioManager();
		const std::filesystem::path AUDIO_SETTINGS_PATH = std::filesystem::path("src/ProjectSettings/AudioSettings.asset");

		if (!audioSettings)
		{
			return;
		}

		bool hasChanged = false;

		// Master Volume
		float float_buffer = audioManager->GetCategoryVolume(0);
		if (DragFloatInputHeader(mRegistry, "Master Volume", "##master_vol", float_buffer))
		{
			if (std::abs(audioManager->GetCategoryVolume(0) - float_buffer) > 0.001f)
			{
				audioManager->SetCategoryVolume(0, float_buffer);
			}
		}

		// SFX list
		static int count = 1;
		ImGui::Text("SFX List");
		ImGui::BeginChild("##sfx_list", ImVec2(), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		if (ImGui::TreeNodeEx("list", ImGuiTreeNodeFlags_Framed))
		{
			std::string groupToDelete = "";

			for (auto& [key, entry] : audioSettings->mSFXMap)
			{
				std::string name = key;
				int int_buffer{};
				bool bool_buffer{};

				float current_volume = entry.volume;
				int current_max_instances = entry.maxInstances;
				bool changeSpatial = false;
				float current_interval = entry.minInterval; // Assuming 'Interval' corresponds to minInterval
				if (ImGui::TreeNodeEx(key.c_str(), ImGuiTreeNodeFlags_Framed))
				{
					
					if (StringInputHeader(mRegistry, "Key", ("##key_" + key).c_str(), name));


					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						if (name != key)
						{
							// set new name
							audioSettings->ReplaceExistingEntry(key, name);
							hasChanged = true;

						}

					}

					ImGui::SameLine();
					if (ImGui::Button(("Remove Entry " + key).c_str()))
					{
						groupToDelete = key;
					}

					//hasChanged = DragFloatInputHeader(mRegistry, "Volume", ("##vol_" + key).c_str(), entry.volume, "%.3f", 0.f, 1.0f) || hasChanged;
					if (DragFloatInputHeader(mRegistry, "Volume", ("##vol_" + key).c_str(), current_volume, "%.3f", 0.f, 1.0f))
					{
						//Not sure if i should add a check but imma just write
						audioSettings->SetSoundGroupVolume(key, current_volume);
						hasChanged = true;
					}
					if (DragIntInputHeader(mRegistry, "Max Instances", ("##maxInstances_" + key).c_str(), current_max_instances, "%d", -1, 64))
					{
						if (current_max_instances != audioSettings->GetMaxInstances(key))
						{
							audioSettings->SetMaxInstances(key, current_max_instances);
							hasChanged = true;
						}
					}
					hasChanged = BoolInputHeader(mRegistry, "Is 3D", ("##is3D_" + key).c_str(), entry.isSpatial) || hasChanged;
					/*if (BoolInputHeader(mRegistry, "Is 3D", ("##is3D_" + key).c_str(), current_is_spatial))
					{
						if (current_is_spatial != audioSettings->GetSoundGroupSpatialBlendBool(key))
						{
							audioSettings->SetSoundGroupSpatialBlendBool(key, current_is_spatial);
							if (current_is_spatial == true)
							{
								changeSpatial = true;
							}
							hasChanged = true;
						}
					}*/

					if (entry.isSpatial == true)
					{
						changeSpatial = true;
					}
					hasChanged = DragFloatInputHeader(mRegistry, "Spatial Blend", ("##spatialBlend_" + key).c_str(), entry.spatialBlend, "%.3f", 0.0f, 1.0f) || hasChanged;
					/*if (DragFloatInputHeader(mRegistry, "Spatial Blend", ("##spatialBlend_" + key).c_str(), current_spatial_blend, "%.3f", 0.0f, 1.0f))
					{
						if (std::abs(current_spatial_blend - audioSettings->GetSoundGroupSpatialBlend(key)) > 0.001f)
						{
							audioSettings->SetSoundGroupSpatialBlend(key, current_spatial_blend);
							hasChanged = true;
						}
					}*/
					hasChanged = DragFloatInputHeader(mRegistry, "Min Distance", ("##minDistance" + key).c_str(), entry.minDistance, "%.3f", 0.0f, entry.maxDistance) || hasChanged;
					/*if (DragFloatInputHeader(mRegistry, "Min Distance", ("##minDistance" + key).c_str(), current_min_distance, "%.3f", 0.0f, current_max_distance))
					{
						if (std::abs(current_min_distance - audioSettings->GetMinDistance(key)) > 0.001f)
						{
							audioSettings->SetMinDistance(key, current_min_distance);
							hasChanged = true;
						}
					}*/
					hasChanged = DragFloatInputHeader(mRegistry, "Max Distance", ("##maxDistance" + key).c_str(), entry.maxDistance, "%.3f", entry.minDistance) || hasChanged;
					/*if (DragFloatInputHeader(mRegistry, "Max Distance", ("##maxDistance" + key).c_str(), current_max_distance, "%.3f", current_min_distance))
					{
						if (std::abs(current_max_distance - audioSettings->GetMaxDistance(key)) > 0.001f)
						{
							audioSettings->SetMaxDistance(key, current_max_distance);
							hasChanged = true;
						}
					}*/
					hasChanged = DragFloatInputHeader(mRegistry, "Interval", ("##interval" + key).c_str(), entry.minInterval) || hasChanged;

					if (changeSpatial == true)
					{
						entry.spatialBlend = 1.0f;
						changeSpatial = false;
					}
					
					
					for (auto& clip : entry.AudioClips)
					{
						ImGui::PushID(&clip);

						auto oldClip = clip;
						std::function<void(SliceEngine::GUID)> setFunc = [&](SliceEngine::GUID guid)
							{
								//Take out key from parameter
								audioSettings->ChangeAudioClip(key, oldClip, guid, entry.AudioClips);
								hasChanged = true;
							};
						//std::string audioClipLabel = "Audio Clips_" + std::to_string(std::distance(entry.AudioClips.begin(),entry.AudioClips.size()));

						GUIDDragDropInputHeader(mRegistry, "Audio Clip", "##audio_clips", clip, "Audio", setFunc);

						ImGui::PopID();
					}

					if (ImGui::Button("+"))
					{
						
						audioSettings->AddAudioClip(entry.soundGroup, SliceEngine::GUID(10155432597037438324), entry.AudioClips);
						hasChanged = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("-"))
					{
						if (!entry.AudioClips.empty())
						{
							
							audioSettings->RemoveAudioClip(entry.AudioClips);
							hasChanged = true;

						}
					}
					ImGui::TreePop();
				}
			}

			if (!groupToDelete.empty())
			{
				// Assuming you have or will add a RemoveSoundGroup overload that takes a key.
				// If this function doesn't exist in AudioSettings, you will need to add it 
				// or use: audioSettings->mSFXMap.erase(groupToDelete);
				audioSettings->mSFXMap.erase(groupToDelete);
				hasChanged = true;
			}

			ImGui::TreePop();
		}

		if (ImGui::Button("+"))
		{
			size_t nextIndex = audioSettings->mSFXMap.size() + 1;
			std::string keyName = "New_Group_" + std::to_string(nextIndex);

			audioSettings->CreateSoundGroup(keyName);
			hasChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("-"))
		{
			audioSettings->RemoveSoundGroup();
			hasChanged = true;
		}

		if (hasChanged)
		{
			audioSettings->SaveSettings();
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
