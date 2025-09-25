#ifndef INSPECTOR_MANAGER_H
#define INSPECTOR_MANAGER_H

#include "../WindowManager/ICreateWindow.h"

namespace SliceEditor
{
	class InspectorManager : public ICreateWindow
	{
	public:
		InspectorManager() = default;
		~InspectorManager() = default;

		std::unique_ptr<EditorWindow> CreateWindow() override;

		void Init();
	private:
	};
}

#endif