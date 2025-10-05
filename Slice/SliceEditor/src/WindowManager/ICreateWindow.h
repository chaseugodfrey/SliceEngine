/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ICreateWindow.h

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the base interface for creating editor windows.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef INTERFACE_CREATE_WINDOW_H
#define INTERFACE_CREATE_WINDOW_H

#include "EditorWindow.h"

namespace SliceEditor
{
	class ICreateWindow
	{
	public:
		virtual ~ICreateWindow() = default;
		virtual std::unique_ptr<EditorWindow> CreateEditorWindow() = 0;
	};
}
#endif