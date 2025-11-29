#ifndef HISTORY_WINDOW_H
#define HISTORY_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class Registry;
	class HistoryManager;

	class HistoryWindow : public EditorWindow
	{
		HistoryManager* mHistoryManager = nullptr;

		void DrawUndoHistory();
		void DrawRedoHistory();

	public:

		HistoryWindow(Registry& reg) : EditorWindow(reg) {};
		~HistoryWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif