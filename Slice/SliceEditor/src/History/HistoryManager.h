#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

namespace SliceEditor
{
	struct Command;

	class HistoryManager
	{
		size_t stackSize = 50;
		std::stack<std::unique_ptr<Command>> undoStack;
		std::stack<std::unique_ptr<Command>> redoStack;

	public:
		HistoryManager() = default;
		~HistoryManager() = default;

		void AddCommand(const Command& command);
		void SetStackSize(size_t size);

		
		void Undo();
		void Redo();
	};
}
#endif