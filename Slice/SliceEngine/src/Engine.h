#ifndef ENGINE_H
#define ENGINE_H
//
//#include "Window.h"
//#include "Input/InputSystem.h"
//#include "AudioManager.h"
//#include "TransformSystem.h"
//#include "Graphics/ResourceManager.h"
//#include "Graphics/RenderManager.h"
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
		// I'm testing a way to store all the systems in an array
		// to loop and unbind in exit
		// if it doesnt work down the line then just manually unbind all the systems instead.

		//std::unordered_map<std::string, std::shared_ptr<IBaseSystem>> mSystems;

		//std::vector<std::shared_ptr<IBaseSystem>> mSystems;


		// Gonna try to grp these tgt with the other systems in core.h
		//std::unique_ptr<InputSystem> inputs;
		//std::unique_ptr<AudioManager> audio;
		//std::unique_ptr<ResourceManager> mResource;
		//std::unique_ptr<RenderManager> mRender;
		//std::unique_ptr< FramerateManager> framerateManager;


		//std::shared_ptr<TransformSystem> mTransform;
		//std::shared_ptr<WorldSpaceGraphicsSystem> mWorldSpaceGraphics;
		//std::shared_ptr<PhysicSystem> mPhysicsTest;
		//Registry mRegistry;

		void Init();

		void Update();
		void Exit();

	};
}

#endif 