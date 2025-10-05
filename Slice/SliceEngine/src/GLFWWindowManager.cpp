/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			GLFWWindowManager.cpp
 author:		Chase Roderigues
 email:			roderigues.i@digipen.edu
 brief:			Handles GLFW Window

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "GLFWWindowManager.h"

namespace SliceEngine
{
	GLFWwindow* GLFWWindowManager::CreateGLFWWindow()
	{
		SLICE_LOG("Creating Main Window.");

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();

		//glClearColor(0.f, 0.f, 0.f, 1.f);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);
		glfwWindowHint(GLFW_ALPHA_BITS, 8);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		auto vidmode = glfwGetVideoMode(monitor);

		window = glfwCreateWindow(vidmode->width, vidmode->height, "Slice Engine", nullptr, nullptr);

		glfwMakeContextCurrent(window);

		// Error Checking

		GLenum err = glewInit();
		if (err != GLEW_OK) {
			SLICE_LOG("GLEW initialization failed: %s", glewGetErrorString(err));
			return nullptr;
		}

		SLICE_LOG("Main Window Created.");

		return window;
	}

	void GLFWWindowManager::ResizeWindow(int width, int height)
	{
		glfwSetWindowSize(window, width, height);
	}

	void GLFWWindowManager::CloseWindow()
	{
	
		glfwDestroyWindow(window);
		SLICE_LOG("Main Window Destroyed.");
	}

	GLFWwindow* GLFWWindowManager::GetWindow()
	{
		if (window == nullptr)
		{
			assert("Window not intialized");
		}
		return window;
	}
}