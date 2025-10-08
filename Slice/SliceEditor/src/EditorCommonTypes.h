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
	
	struct TreeNode
	{
		std::string name;
		TreeNode* parent;
		TreeNode* child;
		TreeNode* previous;
		TreeNode* next;
	};

	struct DirectoryNode
	{
		std::string fileName;
		bool isDirectory = false;
		std::filesystem::path path;
		DirectoryNode* parent = nullptr;
		std::map<std::string, DirectoryNode> children;
	};

	class Command
	{
	public:
		virtual void Execute() = 0;
		virtual void Undo() = 0;
		virtual ~Command() = default;
	};

	template<typename T>
	class ValueCommand : public Command
	{
		T& ref, oldValue, newValue;

	public:

		ValueCommand(T& r, T oldV, T newV) : ref(r), oldValue(oldV), newValue(newV) {}
		~ValueCommand() = default;

		void Execute() override 
		{ 
			ref = newValue; 
		}

		void Undo() override 
		{ 
			ref = oldValue; 
		}
	};
}

#endif