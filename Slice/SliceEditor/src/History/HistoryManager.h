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

<<<<<<< Updated upstream
		void AddCommand(const Command& command);
=======
		void Init();
		void AddCommand(std::unique_ptr<Command> command);
		void AddCommandFromEvent(AddCommandEvent& event);
>>>>>>> Stashed changes
		void SetStackSize(size_t size);

		
		void Undo();
		void Redo();
	};
}
#endif