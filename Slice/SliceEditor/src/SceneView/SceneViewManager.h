#ifndef SCENE_VIEW_MANAGER_H
#define SCENE_VIEW_MANAGER_H

#include "../WindowManager/ICreateWindow.h"

namespace SliceEditor
{
	class SceneViewManager : public ICreateWindow
	{

	public:

		void Init();
		std::unique_ptr<EditorWindow> CreateWindow() override;

	};
}
#endif
