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

	struct SelectionNode
	{
		enum class SelectionType
		{
			ENTITY,
			FILE
		} type;
		bool isSelected;
	};

	struct EntityNode : SelectionNode
	{
		//std::string name;
		entt::entity entity = entt::null;
		EntityNode(entt::entity ent) : entity(ent)
		{
			type = SelectionType::ENTITY;
			isSelected = false;
		}
	};

	struct DirectoryNode : SelectionNode
	{
		std::string fileName;
		bool isDirectory = false;
		std::filesystem::path path;
		DirectoryNode* parent = nullptr;
		std::map<std::string, DirectoryNode> children;

		DirectoryNode()
		{
			type = SelectionType::FILE;
			isSelected = false;	
		}

	};

	struct FilePayload : SelectionNode
	{
		SliceEngine::GUID guid;
		AssetType assetType;

		FilePayload()
		{
			type = SelectionType::FILE;
			isSelected = false;
			assetType = AssetType::Texture; //Default to Texture for now
		}
	};
}

#endif