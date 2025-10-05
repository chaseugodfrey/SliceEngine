#ifndef ENGINE_H
#define ENGINE_H
//
//#include "Window.h"

struct GLFWwindow;

#include <memory>
#include "Core/Core.h"

namespace SliceEngine
{
	class InputSystem;
	class AudioManager;
	class ResourceManager;
	class RenderManager;
	class FramerateManager; //here

	class Engine
	{
		bool isRunning;

		void LoadProjectSettings();

	public:

		// Gonna try to grp these tgt with the other systems in core.h
		//GLFWwindow* window;
		std::unique_ptr<InputSystem> inputs;
		std::unique_ptr<AudioManager> audio;
		//std::unique_ptr<ResourceManager> mResource;
		//std::unique_ptr<RenderManager> mRender;

		FramerateManager &frm ;

		Engine();
		~Engine();

		void Test();
		void Init();

		void Update();
		void EndFrame();
		void Exit();

	};
}

#endif 