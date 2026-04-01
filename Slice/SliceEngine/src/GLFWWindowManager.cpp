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
		int width = 1920, height = 1080;

		if (monitor)
		{
			auto vidmode = glfwGetVideoMode(monitor);
			if (vidmode)
			{
				width = vidmode->width;
				height = vidmode->height;
			}
		}
		else
		{
			SLICE_LOG("No primary monitor detected. Defaulting to 1920x1080 for headless/CI environment.");
		}

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

		if (mSmokeTest)
		{
			glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			SLICE_LOG("Smoke test mode enabled: creating hidden window.");
		}

		window = glfwCreateWindow(width, height, "Slice Engine", nullptr, nullptr);

		if (!window)
		{
			SLICE_LOG("Failed to create GLFW window!");
			return nullptr;
		}

		glfwMakeContextCurrent(window);

		// Error Checking

		GLenum err = glewInit();
		if (err != GLEW_OK) {
			std::string error = reinterpret_cast<const char*>(glewGetErrorString(err));
			SLICE_LOG("GLEW initialization failed: " + error);
			return nullptr;
		}

		SLICE_LOG("Main Window Created.");

		return window;
	}

	void GLFWWindowManager::ResizeWindow(int width, int height)
	{
		glfwSetWindowSize(window, width, height);
	}

	void GLFWWindowManager::FullScreenWindow()
	{

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();

		auto vidMode = glfwGetVideoMode(monitor);

		glfwSetWindowMonitor(window, monitor, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);

		glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

		glfwMakeContextCurrent(window);


		isFullScreen = true;


	}

	void GLFWWindowManager::NonFullScreenWindow()
	{


		GLFWmonitor* monitor = glfwGetPrimaryMonitor();

		auto vidMode = glfwGetVideoMode(monitor);

		glfwSetWindowMonitor(window, nullptr, 0, 25, vidMode->width, vidMode->height, vidMode->refreshRate);

		glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);

		glfwSetWindowTitle(window, "Weight Of The Sky");

		glfwMakeContextCurrent(window);

		isFullScreen = false;

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