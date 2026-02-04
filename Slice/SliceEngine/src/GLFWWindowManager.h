/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			GLFWWindowManager.h
 author:		Chase Roderigues
 email:			roderigues.i@digipen.edu
 brief:			Handles GLFW Window

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef GLFW_WINDOW_MANAGER_H
#define GLFW_WINDOW_MANAGER_H

struct GLFWwindow;

namespace SliceEngine
{
	class GLFWWindowManager
	{
		GLFWwindow* window;

	public:
		GLFWwindow* CreateGLFWWindow();
		void ResizeWindow(int width, int height);
		void FullScreenWindow();
		void CloseWindow();
		GLFWwindow* GetWindow();
	};
}

#endif