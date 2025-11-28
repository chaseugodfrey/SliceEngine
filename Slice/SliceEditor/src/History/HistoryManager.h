#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include "../Core/IBaseManager.h"

namespace SliceEditor
{
	using HistoryStack = std::vector<std::unique_ptr<Command>>;
	class Registry;

	class HistoryManager : public IBaseManager
	{
		size_t stackSize = 50;
		std::optional<size_t> checkpoint = std::nullopt;
		HistoryStack undoStack;
		HistoryStack redoStack;

	public:
		HistoryManager(Registry& reg) : IBaseManager(reg) {};
		~HistoryManager() = default;

		void Init() override;
		void Update() override;
		void AddCommand(std::unique_ptr<Command> command);
		void AddCommandFromEvent(AddCommandEvent& event);
		void SetStackSize(size_t size);
		
		// temporary, may be useful for future as well
		void CreateCheckpoint();
		void ClearFromCheckpoint();

		const HistoryStack& GetUndoStack();
		const HistoryStack& GetRedoStack();

		void Undo();
		void Redo();
	};
}
#endif