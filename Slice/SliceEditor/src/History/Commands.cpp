#include <pch.h>
#include "Commands.h"
#include "Selection/SelectionManager.h"

namespace SliceEditor
{

	void SelectEntityCommand::Undo()
	{

	}

	void SelectEntityCommand::Redo()
	{

	}
	
	void CreateEntityCommand::Redo()
	{
		EditorUtilities::GameObject_CreateEmpty();
	}

	void CreateEntityCommand::Undo()
	{
		EditorUtilities::GameObject_Destroy(entity);
	}

	void ParentEntityCommand::Redo()
	{
		EditorUtilities::GameObject_Parent(child, newParent);
	}

	void ParentEntityCommand::Undo()
	{
		EditorUtilities::GameObject_Parent(child, oldParent);
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

	void SelectNodeCommand::Redo()
	{
		sSelection.SelectMultiple(newNodes, true);
	}

	void SelectNodeCommand::Undo()
	{
		sSelection.SelectMultiple(oldNodes, true);
	}
}