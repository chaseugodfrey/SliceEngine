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


		if (ImGui::BeginPopupModal("##RenameFile"))
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
}