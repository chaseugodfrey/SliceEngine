#ifndef CONSOLE_WINDOW_H
#define CONSOLE_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class Registry;

	class ConsoleWindow : public EditorWindow
	{

	public:
		ConsoleWindow(Registry& reg) : EditorWindow(reg) {};
		~ConsoleWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif