/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        EditorWindow.h

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the EditorWindow class, which is the base class for all editor windows.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

namespace SliceEditor
{
	class EditorWindow
	{
	public:
		virtual void Draw() = 0;
		virtual ~EditorWindow() = default;
	};
}

#endif
