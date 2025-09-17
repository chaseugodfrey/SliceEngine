#ifndef GLFW_WINDOW_MANAGER_H
#define GLFW_WINDOW_MANAGER_H

struct GLFWwindow;

namespace SliceEngine
{
	class GLFWWindowManager
	{
		GLFWwindow* window;

	public:
		GLFWwindow* CreateWindow();
		void ResizeWindow(GLFWwindow* window, int width, int height);
		void CloseWindow(GLFWwindow* window);
		GLFWwindow* GetWindow();
	};
}

#endif