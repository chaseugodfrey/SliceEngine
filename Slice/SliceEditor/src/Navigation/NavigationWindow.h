

#ifndef NAVIGATION_WINDOW_H
#define NAVIGATION_WINDOW_H

#include "../WindowManager/EditorWindow.h"
#include "RecastNavmesh.h"

namespace SliceEditor
{
	class Registry;
	class SelectionManager;

	class NavigationWindow : public EditorWindow
	{

	public:
		RecastNavMesh mCompiler;

		NavigationWindow(Registry& reg) : EditorWindow(reg) {};
		~NavigationWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif
