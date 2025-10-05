/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        IBaseManager.h

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the Base Manager interface for all editor managers.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef EDITOR_BASE_MANAGER_H
#define EDITOR_BASE_MANAGER_H

namespace SliceEditor
{
	class Registry;

	class IBaseManager
	{

	protected:
		Registry& registry;

	public:
		IBaseManager(Registry& reg) : registry(reg) {};
		virtual ~IBaseManager() = default;
		virtual void Init() = 0;
		Registry& GetRegistry() { return registry; }
	};
}

#endif