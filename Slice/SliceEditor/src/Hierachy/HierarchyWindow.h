#ifndef HIERARCHY_WINDOW_H
#define HIERARCHY_WINDOW_H

#include "../WindowManager/EditorWindow.h"
#include <vector>

namespace SliceEditor
{
	class HierarchyManager;
	struct TestNode;

	class HierarchyWindow : public EditorWindow
	{
		HierarchyManager& mManager;
		bool isDirty;

		void DrawNode(TestNode& node);
		void DrawSceneNode(TestNode& node);
		void DrawNodeGraph();

		void EntityContextPopUp(TestNode& node);

	public:

		HierarchyWindow(HierarchyManager& manager);
		~HierarchyWindow() = default;
		void Draw() override final;
	};
}

#endif