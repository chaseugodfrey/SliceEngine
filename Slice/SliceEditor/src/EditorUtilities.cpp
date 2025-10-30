#include <pch.h>
#include "EditorUtilities.h"
#include <History/HistoryManager.h>
#include <Selection/SelectionManager.h>

namespace SliceEditor
{
	namespace EditorUtilities
	{
		SliceEngine::GameObject GameObject_CreateEmpty(entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateModel(entt::entity parent,SliceEngine::GUID guid, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Model(guid);

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		void GameObject_Destroy(entt::entity target, HistoryManager* history)
		{
			EventManager::GetInstance()->Publish<ClearSelectionEvent>();
			SliceEngine::FactoryInstance.Destroy(target);

			// leaving undo & redo of deletion to future implementation 
			// since undoing deletion requires storing/saving data and retrieving it which is more complex
			// for now, deleting is permanent
			
			//if (!suppressHistory)
			//	EventManager::GetInstance()->Publish<AddCommandEvent>(std::make_unique<CreateGameObjectEvent>(target));
		}

		void GameObject_Parent(entt::entity child, entt::entity parent, HistoryManager* history)
		{
			auto& scene_graph = SliceEngine::FactoryInstance.GetGOByEntity(child).GetComponent<SliceEngine::SceneGraph>();
			auto old_parent = scene_graph.neighbours[SliceEngine::SceneGraph::UP];

			if (SliceEngine::FactoryInstance.SetParent(child, parent))
			{
				if (history)
				{
					history->AddCommand(std::make_unique<ParentEntityCommand>(child, old_parent, parent));
				}
			}
		}

		void GameObject_Unparent(entt::entity child, HistoryManager* history)
		{
			auto& scene_graph = SliceEngine::FactoryInstance.GetGOByEntity(child).GetComponent<SliceEngine::SceneGraph>();
			auto old_parent = scene_graph.neighbours[SliceEngine::SceneGraph::UP];

			if (SliceEngine::FactoryInstance.Unparent(child))
			{
				if (history)
				{
					auto new_parent = scene_graph.neighbours[SliceEngine::SceneGraph::UP];

					history->AddCommand(std::make_unique<ParentEntityCommand>(child, old_parent, new_parent));
				}

			}
		}

		void GameObject_SetSibling(entt::entity target, entt::entity destination, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;

			if (destination == target)
				return;

			else
			{
				auto& destSceneGraph = factory.GetGOByEntity(destination).GetComponent<SliceEngine::SceneGraph>();
				factory.SetNewSceneGraphLocation(target, destination, destSceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);

				if (history)
				{

				}
			}
		}

		void GameObject_RemoveComponent(entt::entity entity, const std::string& componentName, HistoryManager* history = nullptr)
		{

		}

	}
}