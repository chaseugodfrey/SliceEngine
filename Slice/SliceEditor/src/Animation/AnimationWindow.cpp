#include <pch.h>
#include "AnimationWindow.h"

namespace SliceEditor
{
	AnimationWindow::~AnimationWindow()
	{
	}

	void AnimationWindow::Init()
	{
		AnimationPropertyGroup transformGroup;

		transformGroup.name = "Transform";
		transformGroup.properties.push_back(AnimationProperty{ "Position.x", std::vector<ImGui::FrameIndexType>({0, 10, 20}) });
		transformGroup.properties.push_back(AnimationProperty{ "Position.y", std::vector<ImGui::FrameIndexType>({0, 10, 20}) });
		transformGroup.properties.push_back(AnimationProperty{ "Position.z", std::vector<ImGui::FrameIndexType>({0, 10, 20}) });

		mPropertyGroups.push_back(transformGroup);
	}

	void AnimationWindow::Draw()
	{
		bool isAnimatorSelected = animationClipNames.size() > 0;

		ImGui::Begin("Animation");

		// disable if no selection
		if (!isAnimatorSelected)
			ImGui::BeginDisabled();

#pragma region Animation Toolbar
		ImGui::BeginGroup();

		static bool loop = false;
		static bool playing = false;

		// Controls

		std::string playOrPause = playing ? "Pause" : "Play";

		if (ImGui::Button(playOrPause.c_str()))
		{
			playing = !playing;
		}

		ImGui::SameLine();

		if (ImGui::Button("Stop"))
		{
			playing = false;
		}

		ImGui::SameLine();

		bool wasLoop = loop;
		if (wasLoop)
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

		if (ImGui::Button("Loop"))
		{
			loop = !loop;
		}

		if (wasLoop)
			ImGui::PopStyleColor();

		ImGui::EndGroup();


		// run timeline here temporarily
		
		if (playing)
		{
			currentFrame++;
			if (currentFrame > endFrame)
			{
				currentFrame = startFrame;
				if (!loop)
					playing = false;
			}
		}

#pragma endregion
		ImGui::Separator();
		ImGui::BeginGroup();

		std::string preview = "No Animations";

		if (isAnimatorSelected)
			preview = animationClipNames[animationClipIndex];


		if (ImGui::BeginCombo("##anim_clips", preview.c_str()))
		{
			for (size_t i = 0; i < animationClipNames.size(); i++)
			{
				if (ImGui::Selectable(animationClipNames[i]))
				{
					animationClipIndex = i;
				}
			}
		}

		ImGui::EndGroup();

		if (ImGui::BeginNeoSequencer("Animation Sequencer", &currentFrame, &startFrame, &endFrame))
		{
			
			for (auto& group : mPropertyGroups)
			{
				if (ImGui::BeginNeoGroup(group.name.c_str(), &group.isOpen))
				{
					for (auto& property : group.properties)
					{
						if (ImGui::BeginNeoTimeline(property.name.c_str(), property.keys))
						{
							ImGui::EndNeoTimeLine();
						}
					}

					ImGui::EndNeoGroup();
				}
			}

			ImGui::EndNeoSequencer();
		}

		if (!isAnimatorSelected)
			ImGui::EndDisabled();

		ImGui::End();
	}
}