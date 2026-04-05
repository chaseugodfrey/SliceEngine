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
#include "Selection/SelectionManager.h"
#include "Resource/ResourceManager.h"

namespace SliceEditor
{
	ContentBrowserWindow::ContentBrowserWindow(ContentBrowserManager& man, Registry& reg) : EditorWindow(reg), mManager(man)
	{
		mSearchBuffer[0] = '\0';
		mSearchPrompt.clear();
	}

	void ContentBrowserWindow::Init()
	{

	}

	void ContentBrowserWindow::Draw()
	{
		if (ImGui::Begin("Content Browser"))
		{

			/*ImGuiID contentDock = ImGui::GetID("contentDock");
			ImGui::DockSpace(contentDock, ImVec2(0, 0), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);*/

			if (ImGui::Button("Reload"))
			{
				mManager.RebuildDirectory();
			}

			/*Setting the ItemSpacing Style to 0, 0 for the 2 child windows*/
			//ImGuiStyle& style = ImGui::GetStyle();
			//SLICE_LOG("Style Padding:" + std::to_string(style.ItemSpacing.x) + " " + std::to_string(style.ItemSpacing.y));
			//style.ItemSpacing = ImVec2(0, 0);

			/*Asset Directory*/
			ImVec2 left_region = ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y);

			if (left_region.x > 0 && left_region.y > 0)
			{
				if (ImGui::BeginChild("##dir", left_region, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX, ImGuiWindowFlags_AlwaysVerticalScrollbar))
				{
					DisplayFolders(*mManager.rootNode);
					//ImGui::Text("Directory Here!");

					ImGui::SeparatorText("Categories");

					DisplayCategories();

					ImGui::EndChild();
				}
			}

			ImGui::SameLine();
			/*Folder Directory*/
			ImVec2 right_region = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);

			if (right_region.x > 0 && right_region.y > 0)
			{
				if (ImGui::BeginChild("##folder", right_region, ImGuiChildFlags_Borders))
				{
					//Pop-up General Context for File Creation
					if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
					{
						ImGui::OpenPopup("menu_create");
					}

					if (ImGui::BeginPopupContextWindow("menu_create"))
					{
						EditorUtilities::MenuList_CreateFiles(mRegistry, mManager.selectedFolder->fullPath);

						ImGui::EndPopup();
					}

					ImGui::SeparatorText(mManager.selectedFolder->fileName.c_str());
					DisplayItems(*mManager.selectedFolder);
					ImGui::EndChild();
				}
			}

			if (!mManager.mPendingDrops.empty() && !mManager.mActiveDrop)
			{
				mManager.mActiveDrop = std::move(mManager.mPendingDrops.front());
			}

			if (mManager.mActiveDrop.has_value())
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

		}

		ImGui::End();

	}

	void ContentBrowserWindow::DisplayFolders(DirectoryNode& node)
	{
		if (node.fullPath.empty())
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

			bool isOpen = ImGui::TreeNodeEx(node.fileName.c_str(), flags);

			if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				SelectFolder(node);
				currentCategoryIndex = -1;
			}

			if(isOpen)
			{
				for (auto& entry : node.children)
				{
					DisplayFolders(entry.second);
				}
				ImGui::TreePop();
			}
		}
	}

	void ContentBrowserWindow::DisplayCategories()
	{
		auto& categories = mManager.categoryNodes;
		for (size_t i = 0; i < categories.size(); ++i)
		{
			bool isSelected = (currentCategoryIndex == static_cast<int>(i));
			if (ImGui::Selectable(categories[i]->fileName.c_str(), isSelected))
			{
				currentCategoryIndex = static_cast<int>(i);
				mManager.selectedFolder = categories[i].get();
			}
		}
	}

	void ContentBrowserWindow::DisplayItems(DirectoryNode& node)
	{
		static DirectoryNode* selectedEntry = nullptr;
		//auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
		//auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");

		//Search feature yippeee
		auto& buffer = mSearchBuffer;
		auto& searchPrompt = mSearchPrompt;

		if (ImGui::IsWindowAppearing())
		{
			ImGui::SetKeyboardFocusHere();
			buffer[0] = '\0';
			searchPrompt.clear();
		}

		std::string id = "##" + node.fileName;

		if (ImGui::InputText(id.c_str(), buffer, IM_ARRAYSIZE(buffer)))
		{
			searchPrompt = buffer;
		}

		if (ImGui::BeginTable("##FolderDirectory", 5))
		{
			//Section for Folders
			for (auto& [name, entry] : node.children)
			{
				if (!searchPrompt.empty() && name.find(searchPrompt) == std::string::npos)
				{
					continue;
				}

				if (entry.isDirectory)
				{
					ImGui::PushID(&entry);
					ImGui::TableNextColumn();
					DisplayFolderNode(entry);
					ImGui::PopID();
				}
			}
			//Section for Files
			for (auto& [name, entry] : node.children)
			{

				if (!searchPrompt.empty() && name.find(searchPrompt) == std::string::npos)
				{
					continue;
				}
				if (!entry.isDirectory)
				{
					ImGui::PushID(&entry);
					ImGui::TableNextColumn();
					DisplayFileNode(entry);
					ImGui::PopID();
				}
			}

			//if (mManager.openRenameFile)
			//{
			//	//mManager.openRenameFile = !mManager.openRenameFile;
			//	ImGui::OpenPopup("##RenameFile");
			//}

			//if (selectedEntry != nullptr)
			//{
			//	RenameFilePopup(*selectedEntry);
			//}

			ImGui::EndTable();
		}
	}

	void ContentBrowserWindow::DisplayFolderNode(DirectoryNode& node)
	{
		if (ImGui::ImageButton(node.fullPath.filename().string().c_str(), GetIcon(&node), ImVec2(64, 64)))
		{
		}

		if (ImGui::IsItemHovered())
		{
			if (ImGui::BeginTooltip())
			{
				ImGui::Text("%s", node.fileName.c_str());
				ImGui::EndTooltip();
			}
		}

		if (ImGui::BeginPopupContextItem("##ItemEditPopup"))
		{
			if (ImGui::MenuItem("Open Folder"))
			{
				SelectFolder(node);
				//ImGui::EndPopup();
				//mManager.SetDirty(true);
			}

			if (ImGui::MenuItem("Rename Folder"))
			{
				mManager.openRenameFile = true;
			}

			if (ImGui::MenuItem("Delete Folder"))
			{
				mManager.mDeleteList.push_back(&node);
			}
			ImGui::EndPopup();
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			SelectFolder(node);
		}

		ImGui::TextWrapped("%s", node.fileName.c_str());
	}

	void ContentBrowserWindow::DisplayFileNode(DirectoryNode& node)
	{
		auto resourceMgr = SliceEngine::Core::GetInstance()->GetResourceManager();
		auto& assetMgr = mRegistry.GetAssetManager();
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");
		std::filesystem::path filePath = node.fullPath;
		std::string fileKey = node.relativePath.generic_string();
		std::string fileExt = filePath.extension().string();
		bool canDrag = true;

		/*Temp Debug Section*/
		std::string fullPathStr = filePath.string();
		std::string relativePathStr = node.relativePath.string();

		if (resourceMgr->mFileNameToGUID.find(fileKey) == resourceMgr->mFileNameToGUID.end())
		{
			canDrag = false;
		}

		if (mRegistry.GetAssetManager().mSupportedAssetTypes.find(fileExt) == mRegistry.GetAssetManager().mSupportedAssetTypes.end())
		{
			canDrag = false;
		}

		if (ImGui::ImageButton(node.fullPath.filename().string().c_str(), GetIcon(&node), ImVec2(64, 64)))
		{
			if(!(node.type == SelectionType::PREFAB))
			{
				selectionManager->SelectSingle(&node);
			}
		}

		//Drag and Drop Payload
		if (canDrag && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			//Check that the extension exists in the map
			SliceEngine::GUID newGUID = resourceMgr->mFileNameToGUID[fileKey];
			std::string payloadType = mRegistry.GetAssetManager().mSupportedAssetTypes[fileExt].second;
			ImGui::SetDragDropPayload(payloadType.c_str(), &newGUID, sizeof(SliceEngine::GUID));

			std::string dragText = "Dragging item " + node.fileName;
			ImGui::Text(dragText.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::IsItemHovered())
		{
			if (ImGui::BeginTooltip())
			{
				ImGui::Text("%s", node.fileName.c_str());
				ImGui::EndTooltip();
			}
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			mManager.OpenFile(node);
		}

		if (ImGui::BeginPopupContextItem("##ItemEditPopup"))
		{

			if (ImGui::MenuItem("Open File"))
			{
				mManager.OpenFile(node);
			}
			if (ImGui::MenuItem("Edit File"))
			{
				mManager.EditFile(node);
			}

			if (ImGui::MenuItem("Rename File"))
			{
				mManager.openRenameFile = true;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::MenuItem("Re-compile File"))
			{
				//Get the metaData for this Asset:
				std::filesystem::path metaPath = assetMgr.GetMetaDataFromFilename(node.relativePath.generic_string());

				//Technically this is a hack. But due to lack of time, i'll leave it here for this milestone. Will fix after M2
				DroppedFile file;

				file.assetType = mRegistry.GetAssetManager().mSupportedAssetTypes[fileExt].first;
				file.filePath = node.fullPath;
				switch (file.assetType)
				{
				case AssetType::Texture:
					file.metaData = std::make_unique<TextureData>();
					break;
				case AssetType::Model:
					file.metaData = std::make_unique<ModelData>();
					break;
				case AssetType::Audio:
					file.metaData = std::make_unique<AudioData>();
					break;
				case AssetType::Scene:
					file.metaData = std::make_unique<SceneData>();
					break;
				case AssetType::Shader:
					file.metaData = std::make_unique<ShaderData>();
					break;
				case AssetType::Prefab:
					file.metaData = std::make_unique<PrefabData>();
					break;
				case AssetType::Font:
					file.metaData = std::make_unique<FontMetaData>();
					break;
				}
				//Default Init the MetaData base class
				file.metaData->Deserialize(metaPath);

				/*file.toRecompile = false;*/

				mManager.mPendingDrops.push(std::move(file));
			}

			if (ImGui::MenuItem("Delete File"))
			{
				//SLICE_LOG_VALUES("Entry Filename: " + entry.fileName);
				//SLICE_LOG_VALUES("Entry Path: " + entry.path.string());
				//SLICE_LOG_VALUES("Entry Parent: " + (*entry.parent).fileName);
				mManager.mDeleteList.push_back(&node);
			}
			ImGui::EndPopup();
		}

		if (mManager.openRenameFile)
		{
			ImGui::OpenPopup("##RenameFile");
			mManager.openRenameFile = !mManager.openRenameFile;
		}

		RenameFilePopup(node);

		ImGui::TextWrapped("%s", node.fileName.c_str());
	}

	void ContentBrowserWindow::SelectFolder(DirectoryNode& node)
	{
		mManager.selectedFolder = &node;
		memset(mSearchBuffer, 0, sizeof(mSearchBuffer));
		mSearchPrompt.clear();
	}

	void ContentBrowserWindow::RenameFilePopup(DirectoryNode& entry)
	{
		static char newName[256] = {};

		if (ImGui::BeginPopupModal("##RenameFile",0, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::IsWindowAppearing()) //First-time copying the name of the file for ImGui to register it
			{
				std::string oldFileName = entry.relativePath.stem().stem().string();
				std::snprintf(newName, sizeof(newName), "%s", oldFileName.c_str());
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

			case AssetType::Audio:
				if (auto* data = static_cast<AudioData*>(file.metaData.get()))
				{
					DisplayAudioData(data);
				}
				break;

			case AssetType::Font:
				if (auto* data = static_cast<FontMetaData*>(file.metaData.get()))
				{
					DisplayFontData(data);
					//DisplayAudioData(data);
				}
				break;
			case AssetType::SequencePackage:
				if (auto* data = static_cast<SequencePkgData*>(file.metaData.get()))
				{
					DisplayAnimsData(data);
					//DisplayAudioData(data);
				}
				break;
			}

			if (ImGui::Button("Compile"))
			{
				if (assetType == AssetType::Model)
				{
						auto* data = static_cast<ModelData*>(file.metaData.get());
					if (data->is_static == false) //It has skele and anim
					{
						/*
								std::unique_ptr<MetaData> skeleData = std::make_unique<SkeletonData>();
								skeleData->InitMetaData(filePath, AssetType::Skeleton, mAssetExtensions[AssetType::Skeleton]);
								data->skeleMetaPath = CreateResource(skeleData.get(), AssetType::Skeleton, AddToRM).string();
								data->skeletonGUID = skeleData->guid;

								std::unique_ptr<MetaData> animData = std::make_unique<AnimData>();
								animData->InitMetaData(filePath, AssetType::Animation, mAssetExtensions[AssetType::Animation]);
								data->animMetaPath = CreateResource(animData.get(), AssetType::Animation, AddToRM).string();
								data->animationGUID = animData->guid;

						*/
					//Create the skeleton and animation first
						std::unique_ptr<MetaData> skeleData = std::make_unique<SkeletonData>();
						skeleData->InitMetaData(file.filePath, AssetType::Skeleton, mRegistry.GetAssetManager().mAssetExtensions[AssetType::Skeleton]);
						// if the resource already exist, keep teh same GUID and resource path
						if (std::filesystem::exists(data->skeleMetaPath))
						{
							skeleData->resourcePath = data->skeleMetaPath;
							skeleData->guid = data->skeletonGUID;
							file.toRecompile = false;
						}

						data->skeleMetaPath = mRegistry.GetAssetManager().CreateResource(skeleData->resourcePath, skeleData.get(),true, file.toRecompile).string();
						data->skeletonGUID = skeleData->guid;

						std::unique_ptr<MetaData> animData = std::make_unique<AnimationData>();
						animData->InitMetaData(file.filePath, AssetType::Animation, mRegistry.GetAssetManager().mAssetExtensions[AssetType::Animation]);
						// if the resource already exist, keep teh same GUID and resource path
						if (std::filesystem::exists(data->animMetaPath))
						{
							animData->resourcePath = data->animMetaPath;
							animData->guid = data->animationGUID;
							file.toRecompile = false;
						}
						data->animMetaPath = mRegistry.GetAssetManager().CreateResource(animData->resourcePath, animData.get(),true, file.toRecompile).string();
						data->animationGUID = animData->guid;

					}
				}
				file.toRecompile = true;
				mRegistry.GetAssetManager().CreateResource(file.filePath, file.metaData.get(), true, file.toRecompile);
				mRegistry.GetAssetManager().CreateAssetMaps();
				AssetRecompiledEvent event;
				event.fileGUID = file.metaData.get()->guid;
				EventManager::GetInstance()->Publish<AssetRecompiledEvent>(event);
				ImGui::CloseCurrentPopup();
				willOpen = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				//mRegistry.GetAssetManager().CreateDescriptorFile(file.filePath);
				ImGui::CloseCurrentPopup();
				willOpen = false;
			}

			ImGui::EndPopup();
		}
	}

	ImTextureID ContentBrowserWindow::GetIcon(DirectoryNode* node)
	{
		using namespace SliceEngine;
		std::optional<Handle<SliceEngineTypes::Texture>> handle;

		switch (node->type)
		{
		case SelectionType::TEXTURE:
			handle = mManager.GetTextureIconHandle(node->relativePath.generic_string());
			break;

		default:
			handle = mManager.GetDefaultIconHandle(node->type);
			break;
		}

		if (handle.has_value())
		{
			auto texture = handle.value().get();
			if (texture && texture->texture_id != 0)
				return static_cast<ImU64>(texture->texture_id);
		}

		return ImTextureID_Invalid;
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


		static std::vector<std::string> usageTypeNames{ "COLOR","TANGENT,BC5", "METALLIC_ROUGHNESS,BC4" };
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

		if (data->usage_type == UsageType::Color) {
			static std::vector<std::string> compressionFormatNames{ "BC1", "BC3", "BC4", "BC5", "BC7" };
			Label("Compression Format: ");
			if (ImGui::BeginCombo("##Compression Format: ", compressionFormatNames[(int)data->cmp_format].c_str()))
			{
				for (int i = 0; i < compressionFormatNames.size(); ++i)
				{
					if (i != 2 && i != 3) {	//skip bc4 and 5

						if (ImGui::Selectable(compressionFormatNames[i].c_str()))
						{
							data->cmp_format = (CompressionFormat)i;
						}
					}
				}
				ImGui::EndCombo();
			}

			Label("Is SRGB: ");
			if (ImGui::Checkbox("##issrgb", &data->is_srgb))
			{

			}

			Label("Premultiply Alpha: ");
			if (ImGui::Checkbox("##premultiply", &data->premultiply_alpha))
			{

			}
		}
	
		Label("Compression Quality: ");
		if (ImGui::DragFloat("##Comp_Quality", &data->comp_quality, 0.1f, 0.0f, 1.0f, "%.1f"))
		{
			data->comp_quality = std::clamp(data->comp_quality, 0.0f, 1.0f);
		}

		Label("Generate Mips: ");
		if (ImGui::Checkbox("##Generate_Mips", &data->generateMips))
		{

		}

		static std::vector<std::string> mipMapFilterNames{ "NONE", "POINT", "LINEAR", "TRIANGLE", "BOX" };
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


		Label("Mip Count: ");
		int mip = data->mip_count;
		if (ImGui::DragInt("##Mip_Count", &mip, 1, 1, 12))
		{
			mip = std::clamp(mip, 1, 12);
			data->mip_count = static_cast<unsigned char>(mip);
		}
	}

	void ContentBrowserWindow::DisplayFBXData(ModelData* data)
	{
		auto Label = [&](const char* text)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(text);
				ImGui::SameLine();
				ImGui::SetCursorPosX(150.0f); // left-align all widgets at X = 150
			};
		Label("Is Static: ");
		ImGui::Checkbox("##Is_Static", &data->is_static);
	}

	void ContentBrowserWindow::DisplayFontData(FontMetaData* data)
	{
		auto Label = [&](const char* text)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(text);
				ImGui::SameLine();
				ImGui::SetCursorPosX(150.0f); // left-align all widgets at X = 150
			};
		Label("Font Resolution: ");
		int font_reso = data->font_resolution;
		if (ImGui::DragInt("##Font_Reso", &font_reso, 1, 1, 200))
		{
			font_reso = std::clamp(font_reso, 1, 200);
			data->font_resolution = font_reso;// = static_cast<unsigned char>(mip);
		}
		Label("Padding: ");
		int padding = data->padding;
		if (ImGui::DragInt("##Padding", &padding, 1, 1, 10))
		{
			padding = std::clamp(padding, 1, 10);
			data->padding = padding;// = static_cast<unsigned char>(mip);
		}
	}

	void ContentBrowserWindow::DisplayMaterialData(MaterialData* data)
	{
		auto Label = [&](const char* text)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(text);
				ImGui::SameLine();
				ImGui::SetCursorPosX(150.0f); // left-align all widgets at X = 150
			};

		//Label("Roughness: ");
		//if (ImGui::DragFloat("##Roughness", &data->roughness, 0.1f, 0.0f, 1.0f, "%.1f"))
		//{
		//	data->roughness = std::clamp(data->roughness, 0.0f, 1.0f);
		//}

		//Label("Metallic: ");
		//if (ImGui::DragFloat("##Metallic", &data->metallic, 0.1f, 0.0f, 1.0f, "%.1f"))
		//{
		//	data->metallic = std::clamp(data->metallic, 0.0f, 1.0f);
		//}
	}

	void ContentBrowserWindow::DisplayAudioData(AudioData* data)
	{
		auto Label = [&](const char* text)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(text);
				ImGui::SameLine();
				ImGui::SetCursorPosX(150.0f); // left-align all widgets at X = 150
			};

		static std::vector<std::string> streamNames{ "CREATE_SAMPLE", "CREATE_STREAM"};
		Label("Audio Stream: ");
		if (ImGui::BeginCombo("##AudioStream: ", streamNames[(int)data->stream].c_str()))
		{
			for (int i = 0; i < streamNames.size(); ++i)
			{
				if (ImGui::Selectable(streamNames[i].c_str()))
				{
					data->stream = (AudioStream)i;
				}
			}
			ImGui::EndCombo();
		}
	}

	void ContentBrowserWindow::DisplayAnimsData(SequencePkgData* data)
	{
		auto Label = [&](const char* text)
			{
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(text);
				ImGui::SameLine();
				ImGui::SetCursorPosX(150.0f); // left-align all widgets at X = 150
			};

		Label("List of Animations: ");

		for (int i = 0; i < data->animations.size(); ++i)
		{
			ImGui::Selectable(data->animations[i].c_str());
		}

	}
#pragma endregion
}