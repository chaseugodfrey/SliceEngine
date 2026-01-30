#ifndef CUSTOMSHADER_WINDOW_H
#define CUSTOMSHADER_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class Registry;
	class SelectionManager;
	class SessionManager;

	struct EntityNode;

	class CustomShaderWindow : public EditorWindow
	{

	public:


		CustomShaderWindow(Registry& reg) : EditorWindow(reg) {};
		~CustomShaderWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}
#endif