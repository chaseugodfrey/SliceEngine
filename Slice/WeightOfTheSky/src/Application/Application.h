#ifndef APP_H
#define APP_H

#include <Engine.h>

namespace SliceBuild
{
	class Application
	{
		SliceEngine::Engine engine;

	public:
		void Init();
		void Run();
		void Exit();
	};
}

#endif