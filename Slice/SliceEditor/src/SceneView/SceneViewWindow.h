/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SceneViewWindow.h

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the SceneViewWindow class, which is responsible for drawing the Scene View window and its contents.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef EDITOR_SCENE_VIEW_H
#define EDITOR_SCENE_VIEW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class SceneViewManager;

	class SceneViewWindow : public EditorWindow
	{
		SliceEngine::GameObject camObj;
		SceneViewManager& mManager;
		GLuint tex_id;

	public:

		SceneViewWindow(SceneViewManager& manager, SliceEngine::GameObject cam);
		~SceneViewWindow() = default;

		// TO DO: replace this with proper camera attaching
		void SetCameraTexture(GLuint texture_id);
		void Draw() override final;
	};
}

#endif