/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ContentBrowserWindow.cpp

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Defines the Content Browser Window class. It is responsible for rendering the Content Browser window in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "ContentBrowserWindow.h"

namespace SliceEditor
{
	ContentBrowserWindow::ContentBrowserWindow(ContentBrowserManager& man, Registry& reg) : EditorWindow(reg), mManager(man)
	{}

	void ContentBrowserWindow::Init()
	{
	}

	void ContentBrowserWindow::Draw()
	{
		ImGui::Begin("Content Browser");
		/*ImGuiID contentDock = ImGui::GetID("contentDock");
		ImGui::DockSpace(contentDock, ImVec2(0, 0), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);*/

		if (ImGui::Button("Reload"))
		{
			mManager.RebuildDirectory(*mManager.rootNode);
		}

		/*Setting the ItemSpacing Style to 0, 0 for the 2 child windows*/
		//ImGuiStyle& style = ImGui::GetStyle();
		//SLICE_LOG("Style Padding:" + std::to_string(style.ItemSpacing.x) + " " + std::to_string(style.ItemSpacing.y));
		//style.ItemSpacing = ImVec2(0, 0);

		/*Asset Directory*/
		ImVec2 left_region = ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y);

		if(left_region.x > 0 && left_region.y > 0)
		{
			if (ImGui::BeginChild("##dir", left_region, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX))
			{

				DisplayFolders(*mManager.rootNode);
				//ImGui::Text("Directory Here!");

				ImGui::EndChild();
			}
		}

		ImGui::SameLine();
		/*Folder Directory*/
		ImVec2 right_region = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);

		if(right_region.x > 0 && right_region.y > 0)
		{
			if (ImGui::BeginChild("##folder", right_region, ImGuiChildFlags_Border))
			{

				DisplayItems(*mManager.selectedFolder);
				ImGui::EndChild();
			}
		}

		if (!mManager.mPendingDrops.empty() && !mManager.mActiveDrop)
		{
			mManager.mActiveDrop = std::move(mManager.mPendingDrops.front());
		}

		if(mManager.mActiveDrop.has_value())
		{
			bool isOpen = true;

			ImGui::OpenPopup("##CompileAsset");

			CompileAssetPopup(*mManager.mActiveDrop, isOpen);

			if (!isOpen) //Pop-up is closed for some reason
			{
				mManager.mPendingDrops.pop(); //The front is done, move on to next (if any)
				mManager.mActiveDrop.reset(); //Remove the current activeDrop
			}
		}

		ImGui::End();
	}

	void ContentBrowserWindow::DisplayFolders(DirectoryNode& node)
	{

		if (node.path.empty())
		{
			ImGui::Text("No Path Found!");
			return;
		}

		if (!node.isDirectory)
		{
			return;
		}

		else
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;

			if (std::none_of(node.children.begin(), node.children.end(), [](const auto& child)
				{return child.second.isDirectory; }))
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}

			if (ImGui::TreeNodeEx(node.fileName.c_str(), flags))
			{
				if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					SelectFile(node);
				}

				for (auto& entry : node.children)
				{
					DisplayFolders(entry.second);
				}
				ImGui::TreePop();
			}
		}
	}

	void ContentBrowserWindow::DisplayItems(DirectoryNode& node)
	{
		static DirectoryNode* selectedEntry = nullptr;

		if (ImGui::BeginTable("##FolderDirectory", 5))
		{

			//Section for Folders
			for (auto& [name, entry] : node.children)
			{
				if (entry.isDirectory)
				{
					ImGui::TableNextColumn();
					//DisplayButton(selectedEntry, entry, true);

					if (ImGui::ImageButton(entry.path.filename().string().c_str(), nullptr, ImVec2(64, 64)))
					{
						selectedEntry = &entry;
					}

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						selectedEntry = &entry;
					}
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						SelectFile(entry);
						selectedEntry = nullptr;
						ImGui::EndTable(); //Setting the Pre-mature Table End
						return;
					}
					if (selectedEntry == &entry && ImGui::BeginPopupContextItem("##ItemEditPopup"))
					{
						if (ImGui::MenuItem("Open Folder"))
						{
							SelectFile(entry);
							selectedEntry = nullptr;
							ImGui::EndTable(); //Setting the Pre-mature Table End
							return;
						}

						if (ImGui::MenuItem("Rename File"))
						{
							mManager.openRenameFile = true;
						}

						if (ImGui::MenuItem("Delete Folder"))
						{
							mManager.DeleteFile(entry);
							selectedEntry = nullptr;
							ImGui::EndPopup();
							ImGui::EndTable();
							return;
						}
						ImGui::EndPopup();
					}

					ImGui::Text("%s", entry.fileName.c_str());
				}
			}


			//Section for Files
			for (auto& [name, entry] : node.children)
			{
				if (!entry.isDirectory)
				{
					ImGui::TableNextColumn();

					std::filesystem::path filePath = entry.fileName;
					std::string fileKey = filePath.stem().stem().string();
					std::string fileExt = filePath.extension().string();
					bool canDrag = true;

					if (mRegistry.GetAssetManager().mDescriptorMap.find(fileKey) == mRegistry.GetAssetManager().mDescriptorMap.end())
					{
						canDrag = false;
					}

					if (mRegistry.GetAssetManager().mSupportedAssetTypes.find(fileExt) == mRegistry.GetAssetManager().mSupportedAssetTypes.end())
					{
						canDrag = false;
					}

					if (ImGui::ImageButton(entry.path.filename().string().c_str(), nullptr, ImVec2(64, 64)))
					{
						selectedEntry = &entry;
					}

					//Drag and Drop Payload
					if (canDrag && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						//Check that the extension exists in the map
						SliceEngine::GUID newGUID = SliceEngine::GUID(mRegistry.GetAssetManager().mDescriptorMap[fileKey]);
						std::string payloadType = mRegistry.GetAssetManager().mSupportedAssetTypes[fileExt].second;
						ImGui::SetDragDropPayload(payloadType.c_str(), &newGUID, sizeof(SliceEngine::GUID));

						std::string dragText = "Dragging item " + entry.fileName;
						ImGui::Text(dragText.c_str());
						ImGui::EndDragDropSource();
					}

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						selectedEntry = &entry;
					}

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						mManager.OpenFile(entry);
					}

					if (selectedEntry == &entry && ImGui::BeginPopupContextItem("##ItemEditPopup"))
					{
						selectedEntry = &entry;

						if (ImGui::MenuItem("Open File"))
						{
							mManager.OpenFile(entry);
						}
						if (ImGui::MenuItem("Rename File"))
						{
							mManager.openRenameFile = true;
						}
						if (ImGui::MenuItem("Delete File"))
						{
							//SLICE_LOG_VALUES("Entry Filename: " + entry.fileName);
							//SLICE_LOG_VALUES("Entry Path: " + entry.path.string());
							//SLICE_LOG_VALUES("Entry Parent: " + (*entry.parent).fileName);
							mManager.DeleteFile(entry);
							selectedEntry = nullptr;
							ImGui::EndPopup();
							break;
						}
						ImGui::EndPopup();
					}

					ImGui::Text("%s", name.c_str());
				}
			}

			if (mManager.openRenameFile)
			{
				mManager.openRenameFile = !mManager.openRenameFile;
				ImGui::OpenPopup("##RenameFile");
			}

			if (selectedEntry != nullptr)
			{
				RenameFilePopup(*selectedEntry);
			}

			ImGui::EndTable();
		}
	}

	void ContentBrowserWindow::DisplayButton(DirectoryNode* selectedEntry, DirectoryNode& entry, bool isDirectory)
	{
		if (ImGui::ImageButton(entry.path.filename().string().c_str(), nullptr, ImVec2(64, 64)))
		{
			selectedEntry = &entry;
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			selectedEntry = &entry;
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			SelectFile(entry);
			selectedEntry = nullptr;
			ImGui::EndTable(); //Setting the Pre-mature Table End
			return;
		}
		if (selectedEntry == &entry && ImGui::BeginPopupContextItem("##ItemEditPopup"))
		{
			if (ImGui::MenuItem("Open Folder"))
			{
				SelectFile(entry);
				selectedEntry = nullptr;
				ImGui::EndTable(); //Setting the Pre-mature Table End
				return;
			}

			if (ImGui::MenuItem("Rename File"))
			{
				mManager.openRenameFile = true;
			}

			if (ImGui::MenuItem("Delete Folder"))
			{
				mManager.DeleteFile(entry);
				selectedEntry = nullptr;
				ImGui::EndPopup();
				ImGui::EndTable();
				return;
			}
			ImGui::EndPopup();
		}

		ImGui::Text("%s", entry.fileName.c_str());
	}

	void ContentBrowserWindow::SelectFile(DirectoryNode& node)
	{
		mManager.selectedFolder = &node;
	}

	void ContentBrowserWindow::RenameFilePopup(DirectoryNode& entry)
	{
		static char newName[256] = {};

		if (ImGui::BeginPopupModal("##RenameFile",0, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::IsWindowAppearing()) //First-time copying the name of the file for ImGui to register it
			{
				std::snprintf(newName, sizeof(newName), "%s", entry.fileName.c_str());
				/*std::memset(newName, 0, sizeof(newName));
				std::strncpy(newName, entry.fileName.c_str(), sizeof(newName) - 1);
				newName[sizeof(newName) - 1] = '\0';*/
			}
			ImGui::Text("New Filename : ");
			ImGui::SameLine();
			ImGui::InputText("##New Filename:", newName, sizeof(newName));

			if (ImGui::Button("Rename"))
			{
				if (newName[0] != '\0')
				{
					mManager.RenameFile(entry, newName);
					ImGui::CloseCurrentPopup();
				}
				else
				{
					SLICE_LOG_ERROR("Trying to rename into an empty filename!");
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ContentBrowserWindow::CompileAssetPopup(DroppedFile& file, bool& willOpen)
	{
		auto Label = [&](const char* text)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(text);
				ImGui::SameLine();
				ImGui::SetCursorPosX(150.0f); // left-align all widgets at X = 150
			};

		if (ImGui::BeginPopupModal("##CompileAsset",&willOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{

			std::string pathString = file.filePath.string();
			std::string fileExt = file.filePath.extension().string();

			if (mRegistry.GetAssetManager().mSupportedAssetTypes.find(fileExt) == mRegistry.GetAssetManager().mSupportedAssetTypes.end())
			{
				ImGui::CloseCurrentPopup();
				willOpen = false;
			}
			//Name of Asset File
			//Default MetaFile stuff
			Label("Asset Name: ");
			ImGui::Text(file.metaData.get()->assetName.c_str());
			Label("GUID: ");
			ImGui::Text(std::to_string(file.metaData.get()->guid.GetGUID()).c_str());

			AssetType assetType = mRegistry.GetAssetManager().mSupportedAssetTypes[fileExt].first;

			switch (assetType)
			{
			case AssetType::Texture:
				if (auto* data = static_cast<TextureData*>(file.metaData.get()))
				{
					DisplayTextureData(data);
				}
				break;

			case AssetType::Model:
				if(auto* data = static_cast<ModelData*>(file.metaData.get()))
				{
					DisplayFBXData(data);
				}
				break;
			}

			if (ImGui::Button("Compile"))
			{
				mRegistry.GetAssetManager().CreateResource(file.metaData.get(), file.assetType);
				ImGui::CloseCurrentPopup();
				willOpen = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
				willOpen = false;
			}

			ImGui::EndPopup();
		}
	}

	#pragma region Display MetaData Region
	void ContentBrowserWindow::DisplayTextureData(TextureData* data)
	{
		auto Label = [&](const char* text)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(text);
				ImGui::SameLine();
				ImGui::SetCursorPosX(150.0f); // left-align all widgets at X = 150
			};

		static std::vector<std::string> compressionFormatNames{ "RGB_BC1" , "RGBA_BC3" };
		Label("Compression Format: ");
		if (ImGui::BeginCombo("##Compression Format: ", compressionFormatNames[(int)data->cmp_format].c_str()))
		{
			for (int i = 0; i < compressionFormatNames.size(); ++i)
			{
				if (ImGui::Selectable(compressionFormatNames[i].c_str()))
				{
					data->cmp_format = (CompressionFormat)i;
				}
			}
			ImGui::EndCombo();
		}

		Label("Compression Quality: ");
		if (ImGui::DragFloat("##Comp_Quality", &data->comp_quality, 0.1f, 0.0f, 1.0f, "%.1f"))
		{
			data->comp_quality = std::clamp(data->comp_quality, 0.0f, 1.0f);
		}

		static std::vector<std::string> mipMapFilterNames{"NONE", "POINT", "LINEAR", "TRIANGLE", "BOX"};
		Label("MipMapFilter: ");
		if (ImGui::BeginCombo("##MipMapFilter: ", mipMapFilterNames[(int)data->mip_filter].c_str()))
		{
			for (int i = 0; i < mipMapFilterNames.size(); ++i)
			{
				if (ImGui::Selectable(mipMapFilterNames[i].c_str()))
				{
					data->mip_filter = (MipMapFilter)i;
				}
			}
			ImGui::EndCombo();
		}

		Label("Generate Mips: ");
		if (ImGui::Checkbox("##Generate_Mips", &data->generateMips));

		Label("Mip Count: ");
		int mip = data->mip_count;
		if (ImGui::DragInt("##Mip_Count", &mip, 1, 1, 12))
		{
			mip = std::clamp(mip, 1, 12);
			data->mip_count = static_cast<unsigned char>(mip);
		}

		Label("Has Alpha: ");
		if (ImGui::Checkbox("##Has_Alpha", &data->hasAlpha));

		Label("Alpha_Threshold: ");
		int threshold = data->alpha_threshold;
		if (ImGui::SliderInt("##Alpha_Threshold", &threshold, 0, 255))
		{
			threshold = std::clamp(threshold, 0, 255);
			data->alpha_threshold = static_cast<unsigned char>(threshold);
		}

		static std::vector<std::string> wrapTypeNames{"CLAMP_TO_EDGE", "WRAP", "MIRROR"};
		Label("U_Wrap: ");
		if (ImGui::BeginCombo("##U_Wrap: ", wrapTypeNames[(int)data->u_wrap].c_str()))
		{
			for (int i = 0; i < wrapTypeNames.size(); ++i)
			{
				if (ImGui::Selectable(wrapTypeNames[i].c_str()))
				{
					data->u_wrap = (WrapType)i;
				}
			}
			ImGui::EndCombo();
		}

		Label("V_Wrap: ");
		if (ImGui::BeginCombo("##V_Wrap: ", wrapTypeNames[(int)data->v_wrap].c_str()))
		{
			for (int i = 0; i < wrapTypeNames.size(); ++i)
			{
				if (ImGui::Selectable(wrapTypeNames[i].c_str()))
				{
					data->v_wrap = (WrapType)i;
				}
			}
			ImGui::EndCombo();
		}

		static std::vector<std::string> usageTypeNames{ "COLOR","COLOR_ALPHA","TANGENT_NORMAL","INTENSITY" };
		Label("Usage Type: ");
		if (ImGui::BeginCombo("##UsageType: ", usageTypeNames[(int)data->usage_type].c_str()))
		{
			for (int i = 0; i < usageTypeNames.size(); ++i)
			{
				if (ImGui::Selectable(usageTypeNames[i].c_str()))
				{
					data->usage_type = (UsageType)i;
				}
			}
			ImGui::EndCombo();
		}

	}

	void ContentBrowserWindow::DisplayFBXData(ModelData* data)
	{}

	void ContentBrowserWindow::DisplayMaterialData(MaterialData* data)
	{
		auto Label = [&](const char* text)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(text);
				ImGui::SameLine();
				ImGui::SetCursorPosX(150.0f); // left-align all widgets at X = 150
			};

		Label("Roughness: ");
		if (ImGui::DragFloat("##Roughness", &data->roughness, 0.1f, 0.0f, 1.0f, "%.1f"))
		{
			data->roughness = std::clamp(data->roughness, 0.0f, 1.0f);
		}

		Label("Metallic: ");
		if (ImGui::DragFloat("##Metallic", &data->metallic, 0.1f, 0.0f, 1.0f, "%.1f"))
		{
			data->metallic = std::clamp(data->metallic, 0.0f, 1.0f);
		}
	}
#pragma endregion
}