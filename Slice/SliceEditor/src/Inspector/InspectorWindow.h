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

	public:

		InspectorWindow(InspectorManager& manager);
		~InspectorWindow() = default;
		void Draw() override final;
	};
}

#endif