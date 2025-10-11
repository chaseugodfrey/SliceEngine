#ifndef EDITOR_UTILS_H
#define EDITOR_UTILS_H

namespace SliceEditor
{
	namespace EditorUtilities
	{
		static SliceEngine::GameObject GameObject_CreateEmpty(entt::entity = entt::null, bool suppressHistory = false);
		static void GameObject_Destroy(entt::entity target, bool suppressHistory = false);
		static void GameObject_Parent(entt::entity child, entt::entity parent = entt::null, bool suppressHistory = false);
		static void GameObject_Unparent(entt::entity child, bool suppressHistory = false);
		static void GameObject_SetSibling(entt::entity target, entt::entity destination, bool suppressHistory = false);


	}
}

#endif