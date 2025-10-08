#include <pch.h>
#include "HistoryManager.h"

namespace SliceEditor
{
	void HistoryManager::Init()
	{
		undoStack = std::stack<std::unique_ptr<Command>>();
		redoStack = std::stack<std::unique_ptr<Command>>();

		//EventManager::GetInstance()->Subscribe<AddCommandEvent, &HistoryManager::AddCommand>(this);
	}

	void HistoryManager::AddCommand(std::unique_ptr<Command> command)
	{
		undoStack.push(std::move(command));
	}
}