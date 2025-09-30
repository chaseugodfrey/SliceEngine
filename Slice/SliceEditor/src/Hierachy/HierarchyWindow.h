#ifndef HIERARCHY_WINDOW_H
#define HIERARCHY_WINDOW_H

#include "../WindowManager/EditorWindow.h"
#include <vector>

namespace SliceEditor
{
	class HierarchyManager;
	class SelectionSystem;
	struct TestNode;

	class HierarchyWindow : public EditorWindow
	{
		HierarchyManager& mManager;
		// temporary solution todo: remove this and call from registry/event bus
		SelectionSystem& mSelection;

		void DrawNode(TestNode& node);
		void DrawSceneNode(TestNode& node);
		void DrawNodeGraph();

		void EntityContextPopUp(TestNode& node);

		std::unordered_set<TestNode*> set;
		std::unordered_set<entt::entity> selected;
		std::unordered_set<entt::entity> deselected;

	public:

		HierarchyWindow(HierarchyManager& manager, SelectionSystem& selection);
		~HierarchyWindow() = default;
		void Draw() override final;
	};
}

#endif