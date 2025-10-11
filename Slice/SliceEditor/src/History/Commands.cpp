#include <pch.h>
#include "Commands.h"
#include "Selection/SelectionManager.h"

namespace SliceEditor
{

	void SelectEntityCommand::Undo()
	{
		sSelection.SelectMultiple(oldEntities, true);
	}

	void SelectEntityCommand::Redo()
	{
		sSelection.SelectMultiple(newEntities, true);
	}
	
	void CreateEntityCommand::Redo()
	{
		EditorUtilities::GameObject_CreateEmpty(entity, true);
	}

	void CreateEntityCommand::Undo()
	{
		EditorUtilities::GameObject_Destroy(entity, true);
	}

	void ParentEntityCommand::Redo()
	{
		EditorUtilities::GameObject_Parent(child, newParent, true);
	}

	void ParentEntityCommand::Undo()
	{
		EditorUtilities::GameObject_Parent(child, oldParent, true);
	}

	void DeleteEntityCommand::Redo()
	{

	}

	void DeleteEntityCommand::Undo()
	{

	}

	//void AddComponentCommand::Redo()
	//{
	//	//go.AddComponent<ComponentType>(newValue);
	//}

	//void AddComponentCommand::Undo()
	//{
	//	//go.RemoveComponent<ComponentType>();
	//}

	void RemoveComponentCommand::Redo()
	{
		//go.RemoveComponent<ComponentType>();
	}

	void RemoveComponentCommand::Undo()
	{
		//go.AddComponent<ComponentType>(oldValue);
	}
}