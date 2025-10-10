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
	using ComponentDrawer = std::function<void(rttr::variant&)>;

	class InspectorManager : public IBaseManager, public ICreateWindow
	{
	public:
		InspectorManager(Registry& reg) : IBaseManager(reg) {};
		~InspectorManager() = default;

		std::unique_ptr<EditorWindow> CreateEditorWindow() override;

		void Init() override;

		template<typename ComponentType>
		void RegisterDrawer()
		{
			const rttr::type key = rttr::type::get<ComponentType>().get_raw_type();

			mComponentDrawers[key] = [this](rttr::variant& var)
				{
					if (var.is_type<ComponentType*>())
					{
						if (auto* p = var.get_value<ComponentType*>())
						{
							this->DisplayComponentData<ComponentType>(*p);
						}
					}
					// back up type shit
					else if (var.can_convert<ComponentType&>())
					{
						auto& obj = var.get_value<ComponentType&>();
						this->DisplayComponentData<ComponentType>(obj);
					}
					else if (var.can_convert<ComponentType>())
					{
						auto obj = var.get_value<ComponentType>(); // Makes a copy
						this->DisplayComponentData<ComponentType>(obj);
					}
					else
					{

					}

				};
		}

		template<typename ComponentType>
		void DisplayComponentData(ComponentType& component);

		std::unordered_map<rttr::type, ComponentDrawer> mComponentDrawers;
	};
}

#endif