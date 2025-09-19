#include <pch.h>
#include "ContentBrowserWindow.h"

namespace SliceEditor
{
	ContentBrowserWindow::ContentBrowserWindow(ContentBrowserManager& man) : manager(man)
	{}

	void ContentBrowserWindow::Draw()
	{
		ImGui::Begin("Content Browser");
		//ImGuiID contentDock = ImGui::GetID("contentDock");
		//ImGui::DockSpace(contentDock, ImVec2(0, 0), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);

		if (ImGui::Button("Reload"))
		{
			manager.RebuildDirectory(*manager.rootNode);
		}

		/*Setting the ItemSpacing Style to 0, 0 for the 2 child windows*/
		//ImGuiStyle& style = ImGui::GetStyle();
		//SLICE_LOG("Style Padding:" + std::to_string(style.ItemSpacing.x) + " " + std::to_string(style.ItemSpacing.y));
		//style.ItemSpacing = ImVec2(0, 0);

		/*Asset Directory*/
		ImVec2 left_region = ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y);

		if (ImGui::BeginChild("##dir", left_region, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX))
		{

			DisplayFolders(*manager.rootNode);
			//ImGui::Text("Directory Here!");

			ImGui::EndChild();
		}

		ImGui::SameLine();
		/*Folder Directory*/
		ImVec2 right_region = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);

		if (ImGui::BeginChild("##folder", right_region, ImGuiChildFlags_Border))
		{

			DisplayItems(*manager.selectedFolder);
			ImGui::EndChild();
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
							manager.openRenameFile = true;
						}

						if (ImGui::MenuItem("Delete Folder"))
						{
							manager.DeleteFile(entry);
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



			for (auto& [name, entry] : node.children)
			{
				if (!entry.isDirectory)
				{
					ImGui::TableNextColumn();

					//DisplayButton(selectedEntry, entry, false);

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
						manager.OpenFile();
					}

					if (selectedEntry == &entry && ImGui::BeginPopupContextItem("##ItemEditPopup"))
					{
						selectedEntry = &entry;

						if (ImGui::MenuItem("Open File"))
						{
							manager.OpenFile();
						}
						if (ImGui::MenuItem("Rename File"))
						{
							manager.openRenameFile = true;
						}
						if (ImGui::MenuItem("Delete File"))
						{
							//SLICE_LOG_VALUES("Entry Filename: " + entry.fileName);
							//SLICE_LOG_VALUES("Entry Path: " + entry.path.string());
							//SLICE_LOG_VALUES("Entry Parent: " + (*entry.parent).fileName);
							manager.DeleteFile(entry);
							selectedEntry = nullptr;
							ImGui::EndPopup();
							break;
						}
						ImGui::EndPopup();
					}

					ImGui::Text("%s", name.c_str());
				}
			}

			if (manager.openRenameFile)
			{
				manager.openRenameFile = !manager.openRenameFile;
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
				manager.openRenameFile = true;
			}

			if (ImGui::MenuItem("Delete Folder"))
			{
				manager.DeleteFile(entry);
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
		manager.selectedFolder = &node;
	}

	void ContentBrowserWindow::RenameFilePopup(DirectoryNode& entry)
	{
		static char newName[256] = {};


		if (ImGui::BeginPopupModal("##RenameFile"))
		{
			if (ImGui::IsWindowAppearing()) //First-time copying the name of the file for ImGui to register it
			{
				std::memset(newName, 0, sizeof(newName));
				std::strncpy(newName, entry.fileName.c_str(), sizeof(newName) - 1);
				newName[sizeof(newName) - 1] = '\0';
			}
			ImGui::Text("New Filename : ");
			ImGui::SameLine();
			ImGui::InputText("##New Filename:", newName, sizeof(newName));

			if (ImGui::Button("Rename"))
			{
				if (newName[0] != '\0')
				{
					manager.RenameFile(entry, newName);
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