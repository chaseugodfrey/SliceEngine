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
		message = "Create Entity ";
		EditorUtilities::GameObject_CreateEmpty();
	}

	void CreateEntityCommand::Undo()
	{
		message = "Delete Entity ";
		EditorUtilities::GameObject_Destroy(entity);
	}

	void ParentEntityCommand::Redo()
	{
		message = "Parent ";
		EditorUtilities::GameObject_Parent(child, newParent);
	}

	void ParentEntityCommand::Undo()
	{
		message = "Unparent ";
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

	/*void SelectNodeCommand::Redo()
	{
		message = "Selected " + ConvertSelectionTypeToString(oldNodes);
		sSelection.SelectMultiple(newNodes, true);
	}

	void SelectNodeCommand::Undo()
	{
		message = "Selected " + ConvertSelectionTypeToString(newNodes);
		sSelection.SelectMultiple(oldNodes, true);
	}*/

	std::string const Command::GetCommandMessage()
	{
		return message;
	}
}