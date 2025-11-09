#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include "../Core/IBaseManager.h"

namespace SliceEditor
{
	class Registry;

	class HistoryManager : public IBaseManager
	{
		size_t stackSize = 50;
		std::stack<std::unique_ptr<Command>> undoStack;
		std::stack<std::unique_ptr<Command>> redoStack;

	public:
		HistoryManager(Registry& reg) : IBaseManager(reg) {};
		~HistoryManager() = default;

		void Init() override;
		void Update() override;
		void AddCommand(std::unique_ptr<Command> command);
		void AddCommandFromEvent(AddCommandEvent& event);
		void SetStackSize(size_t size);

		void Undo();
		void Redo();
	};
}
#endif