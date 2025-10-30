#include <pch.h>
#include "AnimationWindow.h"

namespace SliceEditor
{
	AnimationWindow::~AnimationWindow()
	{
	}

	void AnimationWindow::Init()
	{

	}

	void AnimationWindow::Draw()
	{
		ImGui::Begin("Animation");
		ImGui::BeginNeoSequencer("Animation Sequencer", &currentFrame, &startFrame, &endFrame);
		ImGui::EndNeoSequencer();
		ImGui::End();
	}
}