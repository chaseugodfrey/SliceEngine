#ifndef EDITOR_UTILS_H
#define EDITOR_UTILS_H

namespace SliceEditor
{
	class HistoryManager;

	namespace EditorUtilities
	{
		SliceEngine::GameObject GameObject_CreateEmpty(entt::entity = entt::null, HistoryManager* history = nullptr);
		void GameObject_Destroy(entt::entity target, HistoryManager* history = nullptr);
		void GameObject_Parent(entt::entity child, entt::entity parent = entt::null, HistoryManager* history = nullptr);
		void GameObject_Unparent(entt::entity child, HistoryManager* history = nullptr);
		void GameObject_SetSibling(entt::entity target, entt::entity destination, HistoryManager* history = nullptr);
	}
}

#endif