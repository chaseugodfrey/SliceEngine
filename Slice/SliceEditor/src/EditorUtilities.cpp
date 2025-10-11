#include <pch.h>
#include <Selection/SelectionManager.h>

namespace SliceEditor
{
	namespace Utilities
	{
		static SliceEngine::GameObject GameObject_CreateEmpty(entt::entity parent, bool suppressHistory)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (!suppressHistory)
				EventManager::GetInstance()->Publish<AddCommandEvent>(std::make_unique<CreateGameObjectEvent>(go.GetEntity()));
		}

		static void GameObject_Destroy(entt::entity target, bool suppressHistory)
		{
			EventManager::GetInstance()->Publish<ClearSelectionEvent>();
			SliceEngine::FactoryInstance.Destroy(target);

			// leaving undo & redo of deletion to future implementation 
			// since undoing deletion requires storing/saving data and retrieving it which is more complex
			// for now, deleting is permanent
			
			//if (!suppressHistory)
			//	EventManager::GetInstance()->Publish<AddCommandEvent>(std::make_unique<CreateGameObjectEvent>(target));
		}

		static void GameObject_Parent(entt::entity child, entt::entity parent, bool suppressHistory)
		{
			SliceEngine::FactoryInstance.SetParent(child, parent);

			if (!suppressHistory)
			{
				auto& scene_graph = SliceEngine::FactoryInstance.GetGOByEntity(child).GetComponent<SliceEngine::SceneGraph>();
				auto old_parent = scene_graph.neighbours[SliceEngine::SceneGraph::UP];
				AddCommandEvent e;
				e.command = std::make_unique<ParentEntityCommand>(child, old_parent, parent);
				EventManager::GetInstance()->Publish<AddCommandEvent>(std::move(e));
			}
		}

		static void GameObject_Unparent(entt::entity child, bool suppressHistory)
		{
			SliceEngine::FactoryInstance.Unparent(child);

			if (!suppressHistory)
				EventManager::GetInstance()->Publish<AddCommandEvent>(std::make_unique<ParentEntityCommand>(child, entt::null));
		}

		static void GameObject_SetSibling(entt::entity target, entt::entity destination, bool suppressHistory)
		{
			auto& factory = SliceEngine::FactoryInstance;

			if (destination == target)
				return;

			else
			{
				auto& destSceneGraph = factory.GetGOByEntity(destination).GetComponent<SliceEngine::SceneGraph>();
				factory.SetNewSceneGraphLocation(target, destination, destSceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);
			}
			
			if (!suppressHistory)
				EventManager::GetInstance()->Publish<AddCommandEvent>(std::make_unique<ParentEntityCommand>(target, entt::null, entt::null));
		}

		static void GameObject_RemoveComponent(entt::entity entity, const std::string& componentName, bool suppressHistory)
		{

		}

	}
}