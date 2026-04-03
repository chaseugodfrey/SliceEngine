/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			EditorEvents.h
 author:		Chase Rodrigues
 email:			rodrigues.i@digipen.edu
 brief:			Events used in the editor

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef EDITOR_EVENTS_H
#define EDITOR_EVENTS_H
namespace SliceEngine
{
	struct GUID;
}

namespace SliceEditor
{
	class Command;

	enum class FileAction
	{
		Added,
		Removed,
		Modified,
		Moved
	};

	struct AssetFileChangedEvent
	{
		bool assetChangeSuccess;
	};

	struct AssetRecompiledEvent
	{
		SliceEngine::GUID fileGUID;
	};

	struct UndoEvent
	{

	};

	struct UndoDisabledEvent
	{

	};

	struct RedoEvent
	{

	};

	struct OnSceneSaveEvent
	{

	};

	struct OnAnimatorChangedEvent
	{
		entt::entity ent;
	};

	struct ClearSelectionEvent
	{
		bool suppressHistory;
	};

	struct CreateGameObjectEvent
	{
		entt::entity entity;
	};

	struct AddCommandEvent
	{
		std::unique_ptr<Command> command;
	};

	struct GetGUIDEvent
	{
		std::string fileName;
	};

	struct RefreshContentBrowser
	{

	};

	struct DeleteSelectedEntities
	{

	};

	struct CloneSelectedEntities
	{

	};

	struct PrefabInspectedEvent
	{
		SliceEngine::GUID prefabGUID;
		bool prefabBeingInspected;
	};

	struct ShaderGraphInspectedEvent
	{
		SliceEngine::GUID shaderGraphGUID;
	};
	
	struct GameObjectScriptSelectedUpdate
	{
		Entity entity;
		bool toAdd;
	};
}

#endif