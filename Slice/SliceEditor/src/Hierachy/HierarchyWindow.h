/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        HierarchyWindow.h

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the HierarchyWindow class, which is responsible for rendering the hierarchy window in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef HIERARCHY_WINDOW_H
#define HIERARCHY_WINDOW_H

#include "../WindowManager/EditorWindow.h"
#include <vector>

namespace SliceEditor
{
	class HierarchyManager;
	class SelectionManager;
	struct TestNode;

	class HierarchyWindow : public EditorWindow
	{
		HierarchyManager& mManager;

		void DrawNode(SelectionManager& mSelection, entt::entity entity, SliceEngine::SceneGraph& graph);
		void DrawSceneNode(TestNode& node);
		void DrawNodeGraph();

		void EntityContextPopUp(entt::entity entity);

		std::unordered_set<TestNode*> set;
		std::unordered_set<entt::entity> selected;
		std::unordered_set<entt::entity> deselected;

	public:

		HierarchyWindow(HierarchyManager& manager);
		~HierarchyWindow() = default;
		void Draw() override final;
	};
}

#endif