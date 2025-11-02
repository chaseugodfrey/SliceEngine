#include <pch.h>
#include "AnimationWindow.h"
#include "Selection/SelectionManager.h"

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

	bool AnimationWindow::CheckForAnimator()
	{
		// Check if any entities selected
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");

		if (selectionManager->mSelectionType != SelectionType::ENTITY)
		{
			ClearData();
			return false;
		}

		auto& nodes = selectionManager->GetSelectedNodes();
		Entity entity = entt::null;

		// if entities present
		if (nodes.size() > 0)
		{
			EntityNode* entityNode = static_cast<EntityNode*>(*nodes.begin());
			entity = entityNode->entity;

			// check if first entity has animator component
			auto anim = SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Animator>(entity);

			// if anim exists
			if (anim)
			{
				// if current animator is null or mismatch
				// ignore if anim == mCurrentAnimator
				// either case, return true
				if (!mCurrentAnimator || anim != mCurrentAnimator)
					LoadDataFromAnimator(anim);

				return true;
			}

			// if retrieved ptr is null, unload animator and destroy data
			else
			{
				ClearData();
				return false;
			}
		}

		// if no entities present
		else
		{
			ClearData();
			return false;
		}
	}

	void AnimationWindow::LoadDataFromAnimator(SliceEngine::Animator* component)
	{
		mCurrentAnimator = component;
	}

	void AnimationWindow::ClearData()
	{
		if (!mCurrentAnimator)
			return;

		mCurrentAnimator = nullptr;
	}

	void AnimationWindow::Draw()
	{
		bool hasAnimator = CheckForAnimator();

		ImGui::Begin("Animation");

		// disable if no selection
		if (!hasAnimator)
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

#pragma region Animation Timeline
		ImGui::BeginGroup();

		std::string preview = "No Animations";

		if (hasAnimator)
		{
			if (animationClipNames.size() > 0)
				preview = animationClipNames[animationClipIndex];
		}

		if (ImGui::BeginCombo("##anim_clips", preview.c_str()))
		{
			for (size_t i = 0; i < animationClipNames.size(); i++)
			{
				if (ImGui::Selectable(animationClipNames[i]))
				{
					animationClipIndex = i;
				}
			}

			ImGui::EndCombo();
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

#pragma endregion

		if (!hasAnimator)
			ImGui::EndDisabled();

		ImGui::End();
	}
}