#include "pch.h"
#include "Core.h"

namespace SliceEngine
{
	void Core::UnbindSystems()
	{
		for (auto& system : mSystems)
		{
			system.second->Unbind();
			//system->Unbind();
		}

	}
}