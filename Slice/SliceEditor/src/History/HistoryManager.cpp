#include <pch.h>
#include "HistoryManager.h"

namespace SliceEditor
{
	void HistoryManager::Init()
	{
		undoStack = std::stack<std::unique_ptr<Command>>();
		redoStack = std::stack<std::unique_ptr<Command>>();
		EventManager::GetInstance()->Subscribe<UndoEvent, &HistoryManager::Undo>(this);
		EventManager::GetInstance()->Subscribe<RedoEvent, &HistoryManager::Redo>(this);
	}

	void HistoryManager::AddCommand(std::unique_ptr<Command> command)
	{
		undoStack.push(std::move(command));
		while (!redoStack.empty())
			redoStack.pop();
	}

	void HistoryManager::AddCommandFromEvent(AddCommandEvent& event)
	{
		AddCommand(std::move(event.command));
	}

	void HistoryManager::Undo()
	{
		if (undoStack.empty())
			return;

		SLICE_LOG("undo stack size: " + std::to_string(undoStack.size()));
		auto& command = undoStack.top();
		command->Undo();
		redoStack.push(std::move(command));
		undoStack.pop();
	}

	void HistoryManager::Redo()
	{
		if (redoStack.empty())
			return;

		SLICE_LOG("redo stack size: " + std::to_string(redoStack.size()));
		auto& command = redoStack.top();
		command->Redo();
		undoStack.push(std::move(command));
		redoStack.pop();
	}

	void HistoryManager::SetStackSize(size_t size)
	{
		stackSize = size;
		while (undoStack.size() > stackSize)
			undoStack.pop();
		while (redoStack.size() > stackSize)
			redoStack.pop();
	}
}