/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        HierarchyManager.h

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the HierarchyManager class, which is responsible for managing the hierarchy of game objects in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef HIERARCHY_MANAGER_H
#define HIERARCHY_MANAGER_H

#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"
#include "../SelectionSystem/ISelectionListener.h"
#include "../../SliceEngine/src/Core/Events.h"

namespace SliceEditor
{
	struct TestNode
	{
		entt::entity entity;
		std::string name;
		bool isSelected;

		bool operator==(TestNode& rhs)
		{
			return entity == rhs.entity;
		}
	};

	class Registry;

	class HierarchyManager : public IBaseManager, public ICreateWindow, public ISelectionListener
	{
		std::map<entt::entity, TestNode> mHierarchy;

		bool isDirty;


	public:

		HierarchyManager(Registry& reg) : IBaseManager(reg), isDirty(true) {};
		~HierarchyManager() = default;

		void Init() override;
		void BuildHierarchy();
		void SetDirty();
		void CheckDirty();
		void AddEntityDirectly(entt::entity entity);
		void Reset();
		void OnSceneLoad(OnSceneLoadedEvent& event);

		TestNode& GetSceneRootNode();
		void AddGameObject();
		void RemoveGameObject(entt::entity target);
		void ParentGameObject(entt::entity child, entt::entity parent = entt::null);
		void Unparent(entt::entity child);
		void SetSiblingIndex(entt::entity target, entt::entity destination);
		void SetNewLocation(entt::entity target, entt::entity destination);

		std::unique_ptr<EditorWindow> CreateEditorWindow() override;

		std::map<entt::entity, TestNode>& GetHierarchy() { return mHierarchy; }

		// Inherited via ISelectionListener
		void OnUpdateSelected(std::unordered_set<entt::entity>& selected) override;
		void OnUpdateDeselected(std::unordered_set<entt::entity>& selected) override;
	};
}

#endif