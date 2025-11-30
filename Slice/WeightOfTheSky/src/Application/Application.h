#ifndef APP_H
#define APP_H

#include <Engine.h>

struct OnGameStopEvent;

namespace SliceBuild
{
	class Application
	{
		SliceEngine::Engine engine;

	public:
		void QuitGameEvent(OnGameStopEvent e);
		void Init();
		void Run();
		void Exit();
	};
}

#endif