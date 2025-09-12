#ifndef ENGINE_H
#define ENGINE_H
//
//#include "Window.h"
#include "Input/InputSystem.h"
#include "AudioManager.h"
#include "Systems/TransformSystem.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/RenderManager.h"
#include "ECS/BaseSystem.h"
#include "ECS/PhysicSystem.h"
//#include "FrameManager/FramerateManager.h"

struct GLFWwindow;

namespace SliceEngine
{
	class Engine
	{
		GLFWwindow* window;
		bool isRunning;

	public:

		// Gonna try to grp these tgt with the other systems in core.h
		std::unique_ptr<InputSystem> inputs;
		std::unique_ptr<AudioManager> audio;
		std::unique_ptr<ResourceManager> mResource;
		std::unique_ptr<RenderManager> mRender;
		//std::unique_ptr< FramerateManager> framerateManager;

		void Init();

		void Update();
		void Exit();

	};
}

#endif 