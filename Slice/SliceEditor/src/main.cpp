#include "Engine.h"

int main()
{
	SliceEngine::Engine engine{};
	engine.Test();
	engine.Init();
	engine.Update();
	engine.Exit();

	return 0;
}
