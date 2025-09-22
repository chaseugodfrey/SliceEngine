#include <pch.h>
#include "Core.h"
#include "../../SliceEngine/src/Core/Core.h"

namespace SliceEditor
{
	SliceEngine::Core* GetEngineCore()
	{
		return SliceEngine::Core::GetInstance();
	}
}
