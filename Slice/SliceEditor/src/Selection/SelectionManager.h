/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SelectionSystem.h

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the SelectionSystem class, which manages the selection of entities in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef SELECTION_MANAGER_H
#define SELECTION_MANAGER_H

#include "Core/IBaseManager.h"
#include "ISelectionListener.h"

namespace SliceEditor
{
	class Registry;

	class SelectionManager : public IBaseManager
	{

		// to do:: make it non-static later
		std::vector<ISelectionListener*> mListeners;
		//std::unordered_set<entt::entity> mSelectedEntities;
		std::unordered_set<SelectionNode*> mSelectedNodes;
		std::vector<SelectionNode*> mSelectionOrder;

		//void UpdateManagers();

	public:

		SelectionManager(Registry& reg) : IBaseManager(reg) {};
		~SelectionManager() = default;

		void Init() override;
		void Update() override;
		void RegisterListener(ISelectionListener* listener);
		void SelectSingle(SelectionNode* node, bool suppressHistory = false);
		void SelectSingle(entt::entity entity, bool suppressHistory = false);
		void SelectSingleAdd(entt::entity entity, bool suppressHistory = false);
		void AddBetweenEntities(SelectionNode* rightNode);
		void ShiftAddEntities(Entity targetEntity, SliceEngine::SceneGraph::Direction direction);
		bool TraverseAndSelect(Entity currentEntity, Entity targetEntity, SliceEngine::SceneGraph::Direction direction);
		void ProcessNodeSelection(EntityNode* currentNode);
		bool SceneGraphRightTraversal(Entity currentEntity, Entity targetEntity);
		void SelectSingleAdd(SelectionNode* node, bool suppressHistory = false);
		//void UpdateDeslected(entt::entity entity, bool suppressHistory = false);
		void SelectMultiple(std::unordered_set<SelectionNode*> selectedNodes, bool suppressHistory = false);
		//void UpdateDeslected(std::unordered_set<entt::entity>& entities, bool suppressHistory = false);
		void ClearSelectionEventHandler(ClearSelectionEvent& event);
		void ClearSelection(bool suppressHistory = false);
		void DeleteSelectedObjects();
		void CloneSelectedObjects();


		//std::unordered_set<entt::entity>& GetSelectedEntities();
		std::unordered_set<SelectionNode*>& GetSelectedNodes();
		SelectionNode* GetLastSelectedNode();

		SelectionType mSelectionType{};
	};
}

#endif