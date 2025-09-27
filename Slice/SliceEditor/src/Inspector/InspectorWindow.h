#ifndef INSPECTOR_WINDOW_H
#define INSPECTOR_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEngine
{

}

namespace SliceEditor
{
	class InspectorManager;

	class InspectorWindow : public EditorWindow
	{
		InspectorManager& mManager;

		// to do: change later
		std::optional<entt::entity> selected_entity;

		void DisplayEntityData();
		void DisplayTransform();
		void DisplayMeshRenderer();
		void DisplayRigidbody();
		void DisplayCollider3D();
		void AddComponentButton();

	public:

		InspectorWindow(InspectorManager& manager);
		~InspectorWindow() = default;
		void Draw() override final;
	};
}

#endif