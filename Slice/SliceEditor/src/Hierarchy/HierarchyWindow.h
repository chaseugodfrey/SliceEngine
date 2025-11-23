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

namespace SliceEditor
{
	class Registry;
	class SelectionManager;
	class SessionManager;

	struct EntityNode;

	class HierarchyWindow : public EditorWindow
	{

		void DrawNode(SelectionManager& mSelection, SessionManager& mSession, entt::entity entity, SliceEngine::SceneGraph& scene_graph,bool isPrefab);
		void DrawSceneNode();
		void DrawPrefabNode();
		void DrawNodeGraph();
		void Reset();
		void EntityContextPopUp(entt::entity entity);

	public:


		HierarchyWindow(Registry& reg) : EditorWindow(reg) {};
		~HierarchyWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif