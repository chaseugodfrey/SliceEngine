#include "pch.h"
#include "Core.h"

namespace SliceEngine
{
	Core::Core()
	{

	}

	Core::~Core()
	{

	}

	void Core::InitFactory()
	{
		mFactory.mRegistry = &mRegistry;
	}

	void Core::UnbindSystems()
	{
		for (auto& system : mSystems)
		{
			system.second->Unbind();
			//system->Unbind();
		}

	}
}