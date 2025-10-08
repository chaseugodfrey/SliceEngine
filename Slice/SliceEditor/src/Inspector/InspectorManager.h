/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        InspectorManager.h

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the InspectorManager class, which manages the data of the inspector window of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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

		std::unique_ptr<EditorWindow> CreateEditorWindow() override;

		void Init() override;

		// temp, todo: replace this w listener
		std::unordered_set<entt::entity>& GetSelectedEntities();
	};
}

#endif