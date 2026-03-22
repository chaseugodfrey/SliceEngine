/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Engine.h
 author:
 email:
 brief:			Main Engine

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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

	public:

		// Gonna try to grp these tgt with the other systems in core.h
		//GLFWwindow* window;
		/*std::unique_ptr<InputSystem> inputs;
		std::unique_ptr<AudioManager> audio;*/
		//std::unique_ptr<ResourceManager> mResource;
		//std::unique_ptr<RenderManager> mRender;

		FramerateManager* frm ;

		float deltaTimeUnscaled;
		float deltaTimeScaled;
		float fixedDeltaTime;
		float fixedDeltaTimeScaled;

		Engine();
		~Engine();

		void Test();
		void Init();

		void Update();
		void SceneChangeEvent(const OnSceneChangeEvent& event);
		void OnPlayStart();
		void OnStopStart();
		void OnPauseStart();
		void OnPlayStarted();
		void WindowSizeSwitch();
		void Draw();
		void EndFrame();
		void Exit();

	};
}

#endif 