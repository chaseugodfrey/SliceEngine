#ifndef INSPECTOR_MANAGER_H
#define INSPECTOR_MANAGER_H

#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"

namespace SliceEditor
{
	class InspectorManager : public IBaseManager, public ICreateWindow
	{
	public:
		InspectorManager(Registry& reg) : IBaseManager(reg) {};
		~InspectorManager() = default;

		std::unique_ptr<EditorWindow> CreateWindow() override;

		void Init() override;

		// temp, todo: replace this w listener
		std::unordered_set<entt::entity>& GetSelectedEntities();
	private:
	};
}

#endif