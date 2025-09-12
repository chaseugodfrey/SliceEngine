#ifndef WINDOW_H
#define WINDOW_H

struct GLFWwindow;

namespace SliceEngine
{
	class Window
	{
	public:
		static GLFWwindow* CreateWindow();
		static void ResizeWindow(GLFWwindow* window, int width, int height);
		static void CloseWindow(GLFWwindow* window);
	};
}

#endif