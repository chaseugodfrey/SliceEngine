#ifndef ANIMATION_WINDOW_H
#define ANIMATION_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class Registry;

	class AnimationWindow : public EditorWindow
	{
		ImGui::FrameIndexType startFrame = 0;
		ImGui::FrameIndexType endFrame = 100;
		ImGui::FrameIndexType currentFrame = 0;

	public:
		AnimationWindow(Registry& reg) : EditorWindow(reg) {};
		~AnimationWindow();
		void Init() override;
		void Draw() override final;
	};
}

#endif