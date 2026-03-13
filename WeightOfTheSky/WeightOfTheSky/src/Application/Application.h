#ifndef APP_H
#define APP_H

#include <Engine.h>
#include <Input/InputSystem.h>

#include <Systems/SceneSystem.h>

namespace SliceEngine
{
	struct OnGameStopEvent;

}

namespace SliceBuild
{
	class Application
	{
		SliceEngine::Engine engine;

	public:
		void QuitGameEvent(SliceEngine::OnGameStopEvent e);
		void Init();
		void Run();
		void Exit();
	};
}

#endif