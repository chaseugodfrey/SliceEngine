#ifndef HIERARCHY_WINDOW_H
#define HIERARCHY_WINDOW_H

#include "../WindowManager/EditorWindow.h"
#include <vector>

namespace SliceEditor
{
	class HierarchyManager;

	class HierarchyWindow : public EditorWindow
	{
		HierarchyManager& mManager;

	public:

		HierarchyWindow(HierarchyManager& manager);
		~HierarchyWindow() = default;
		void Draw() override final;
	};
}

#endif