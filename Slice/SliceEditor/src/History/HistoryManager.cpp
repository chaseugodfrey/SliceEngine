#include <pch.h>
#include "HistoryManager.h"

namespace SliceEditor
{
	void HistoryManager::Init()
	{
		undoStack = HistoryStack();
		redoStack = HistoryStack();

		EventManager::GetInstance()->Subscribe<UndoEvent, &HistoryManager::Undo>(this);
		EventManager::GetInstance()->Subscribe<RedoEvent, &HistoryManager::Redo>(this);
	}

	void HistoryManager::Update()
	{

	}

	void HistoryManager::AddCommand(std::unique_ptr<Command> command)
	{
		undoStack.push_back(std::move(command));
		if (!redoStack.empty())
			redoStack.clear();
	}

	void HistoryManager::AddCommandFromEvent(AddCommandEvent& event)
	{
		AddCommand(std::move(event.command));
	}

	void HistoryManager::Undo()
	{
		if (undoStack.empty())
			return;

		auto command = std::move(undoStack.back());
		undoStack.pop_back();
		command->Undo();
		redoStack.push_back(std::move(command));
	}

	void HistoryManager::Redo()
	{
		if (redoStack.empty())
			return;

		auto command = std::move(redoStack.back());
		redoStack.pop_back();
		command->Redo();
		undoStack.push_back(std::move(command));
	}

	void HistoryManager::SetStackSize(size_t size)
	{
		//stackSize = size;
		//while (undoStack.size() > stackSize)
		//	undoStack.pop();
		//while (redoStack.size() > stackSize)
		//	redoStack.pop();
	}

	void HistoryManager::CreateCheckpoint()
	{
		checkpoint = undoStack.size();
	}

	void HistoryManager::ClearFromCheckpoint()
	{
		if (!checkpoint.has_value())
			return;

		if (checkpoint.value() > undoStack.size())
		{
			checkpoint.reset();
			return;
		}

		undoStack.erase(undoStack.begin() + checkpoint.value(), undoStack.end());
	}

	const HistoryStack& HistoryManager::GetUndoStack()
	{
		return undoStack;
	}
	const HistoryStack& HistoryManager::GetRedoStack()
	{
		return redoStack;
	}
}