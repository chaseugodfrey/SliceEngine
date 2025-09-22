#ifndef PROFILER_MANAGER_H
#define PROFILER_MANAGER_H

#include <memory>
#include "../WindowManager/ICreateWindow.h"

namespace SliceEditor
{

	class ProfilerManager : public ICreateWindow
	{

	public:

		void Init();

		std::unique_ptr<EditorWindow> CreateWindow() override;
	};
}

#endif