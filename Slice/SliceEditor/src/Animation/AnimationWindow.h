#ifndef ANIMATION_WINDOW_H
#define ANIMATION_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class Registry;

	struct AnimationProperty
	{
		std::string name;
		std::vector<ImGui::FrameIndexType> keys;
	};

	// Use ImGui::BeginNeoGroup() to visualise properties
	struct AnimationPropertyGroup
	{
		std::string name;
		std::vector<AnimationProperty> properties;
		bool isOpen;
	};


	class AnimationWindow : public EditorWindow
	{
		ImGui::FrameIndexType startFrame = 0;
		ImGui::FrameIndexType endFrame = 100;
		ImGui::FrameIndexType currentFrame = 0;

		std::vector<AnimationPropertyGroup> mPropertyGroups;

		//
		std::vector<const char*> animationClipNames;
		size_t animationClipIndex;

	public:
		AnimationWindow(Registry& reg) : EditorWindow(reg) {};
		~AnimationWindow();
		void Init() override;
		void Draw() override final;
	};
}

#endif