/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        EditorCommonTypes.h

 author:	  Chase Rodrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Defines common types used across the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef EDITOR_COMMON_TYPES_H
#define EDITOR_COMMON_TYPES_H

#include "AssetManager/AssetTypes.h"

namespace SliceEditor
{

	enum class ManagerID
	{
		CONTENTBROWSER,
		HIERARCHY,
		INSPECTOR,
		SCENEVIEW,
		GAMEVIEW,
		ANIMATOR,
		NAVMESH,
		LIGHTING,
		AUDIO,
		PROFILER,
		CONSOLE,
		ID_MAX
	};

	enum class WindowID
	{
		CONTENTBROWSER,
		HIERARCHY,
		INSPECTOR,
		SCENEVIEW,
		GAMEVIEW,
		ANIMATOR,
		NAVMESH,
		LIGHTING,
		AUDIO,
		PROFILER,
		CONSOLE,
		ID_MAX
	};

	enum class SelectionType : int
	{
		NONE = 0,
		FOLDER = 1,
		ENTITY = 2,
		TEXTURE = 3,
		MODEL = 4,
		AUDIO = 5,
		SCENE = 6,
		SHADER = 7,
		MATERIAL = 8,
		PREFAB = 9,
		ANIMATION = 10,
		ANIMATOR = 11,
		TEXTFILE = 12,
		MIXED = 13,
		UNSUPPORTED = 14
	};

	struct SelectionNode
	{
		SelectionType type;
		bool isSelected;
	};

	const std::unordered_map<std::string, SelectionType> mExtensionToSelectionType =
	{
			{"",  SelectionType::FOLDER },
			{".png",  SelectionType::TEXTURE },
			{".jpg", SelectionType::TEXTURE},
			{".jpeg", SelectionType::TEXTURE},
			{".dds", SelectionType::TEXTURE},
			{".tga", SelectionType::TEXTURE},
			{".gif", SelectionType::TEXTURE},
			{".obj", SelectionType::MODEL},
			{".fbx", SelectionType::MODEL},
			{".wav", SelectionType::AUDIO},
			{".mp3", SelectionType::AUDIO},
			{".ogg", SelectionType::AUDIO},
			{".scene", SelectionType::SCENE},
			{".shader", SelectionType::SHADER},
			{".mat", SelectionType::MATERIAL},
			{".txt", SelectionType::TEXTFILE},
			{".prefab", SelectionType::PREFAB}
	};

	struct EntityNode : SelectionNode
	{
		//std::string name;
		entt::entity entity = entt::null;

		EntityNode()
			: entity(entt::null)
		{
			type = SelectionType::ENTITY;
			isSelected = false;
		}

		EntityNode(entt::entity ent) : entity(ent)
		{
			type = SelectionType::ENTITY;
			isSelected = false;
		}
	};

	struct PrefabNode : SelectionNode
	{
		std::string prefabName;
	};

	struct DirectoryNode : SelectionNode //Content Browser
	{
		std::string fileName;
		std::filesystem::path path;
		DirectoryNode* parent = nullptr;
		std::map<std::string, DirectoryNode> children;
		bool isDirectory = false;
	};

	struct DroppedFile //Dropped File (From File Explorer to Editor)
	{
		AssetType assetType;
		std::unique_ptr<MetaData> metaData;
		std::filesystem::path filePath;
	};


	// ANIMATIONS



	// THEMES

	enum EditorThemeType : int
	{
		DARK = 0,
		LIGHT = 1,
		MICROSOFT = 2
	};

	constexpr std::array<const char*, 3> EditorThemes =
	{
		"Dark",
		"Light",
		"Microsoft"
	};

	// PREFERENCES

	struct Preferences
	{
		EditorThemeType Theme;
	};
}

#endif