#include <pch.h>
#include "AnimationWindow.h"
#include "Selection/SelectionManager.h"
#include <Systems/FramerateManager.h>
#include <Animator/AnimatorSystem.h>
#include <Animator/BoneSystem.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>	//just to get it working for now

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

		mTimeline.isPlaying = false;
		mTimeline.isLoop = false;
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
				{
					LoadDataFromAnimator(anim);
					//mCurrentTransform = &SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(entity);
				}

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

		
		animationClips.reserve(mCurrentAnimator->curr_anim_pkg.animations.size());
		animationClips.clear();

		for (auto& anim : mCurrentAnimator->curr_anim_pkg.animations)
		{
			animationClips.push_back(&anim);
		}

		// add 0 check for size()
		mCurrentClipIndex = 0;
		LoadDataFromAnimationClip(*animationClips[0]);
	}

	void AnimationWindow::LoadDataFromAnimationClip(SliceEngine::SliceEngineTypes::Animation& animClip)
	{
		endFrame = animClip.num_frames;
		startFrame = 0;
		currentFrame = 0;

		mCurrentTime = 0;
	}

	void AnimationWindow::ClearData()
	{
		if (!mCurrentAnimator)
			return;

		mCurrentAnimator = nullptr;
	}

	void AnimationWindow::UpdateTransform(SliceEngine::SliceEngineTypes::Animation* animClip, float time)
	{
		animClip->UpdateTransforms(mCurrentAnimator->final_tforms, time, *mCurrentAnimator->Handle_skeleton.get());
	}

	void AnimationWindow::UpdateBoneScene(Entity ent)
	{
		auto core = SliceEngine::Core::GetInstance();

		auto const& bone = core->GetRegistry().get<SliceEngine::Bone>(ent);
		Entity root_entity = bone.skeleton_root;
		if (root_entity == ent) {
			return;
		}

		//auto& animator = core->GetRegistry().get<SliceEngine::Animator>(root_entity);
		auto& transform = core->GetRegistry().get<SliceEngine::Transform>(ent);

		//if (!mTimeline.isPlaying)
			///continue;

		//some pseudo code
		glm::mat4 const& frame = mCurrentAnimator->GetFinalTform()[bone.frame_idx];
		glm::vec3 translation, scale, skew;
		glm::vec4 perspective;
		glm::quat rotation;
		glm::decompose(frame, scale, rotation, translation, skew, perspective);
		transform.position = translation;
		transform.rotation = rotation;
		transform.scale = scale;

		//if is a renderer, tell skeleton to calculate inverse for this index
		if (core->GetRegistry().any_of<SliceEngine::Renderer>(ent)) {
			mCurrentAnimator->inverse_flags.set(bone.frame_idx);
		}
	}

	void AnimationWindow::UpdateBones(Entity ent)
	{
		//SliceEngine::Animator& animator = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Animator>(entity);
		SliceEngine::Transform& transform = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(ent);
		if (mCurrentAnimator->is_bone)
		{
			auto& anim = animationClips[mCurrentClipIndex];

			anim->ApplyParentTransforms(mCurrentAnimator->final_tforms, *mCurrentAnimator->Handle_skeleton.get(), transform.transform);
			mCurrentAnimator->SetInverseRoots();
			anim->ApplyInverseBind(mCurrentAnimator->final_tforms, *mCurrentAnimator->Handle_skeleton.get());
		}
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

		bool ret = false;

		// Controls

		std::string playOrPause = mTimeline.isPlaying ? "Pause" : "Play";

		if (ImGui::Button(playOrPause.c_str()))
		{
			mTimeline.isPlaying = !mTimeline.isPlaying;
		}

		ImGui::SameLine();

		if (ImGui::Button("Stop"))
		{
			auto core = SliceEngine::Core::GetInstance();

			mTimeline.isPlaying = false;
			LoadDataFromAnimationClip(*animationClips[mCurrentClipIndex]);

			// update trf, bones scenegraph and bones update function
			//animationClips[mCurrentClipIndex]->UpdateTransforms(mCurrentAnimator->final_tforms, 0, *mCurrentAnimator->Handle_skeleton.get());
			UpdateTransform(animationClips[mCurrentClipIndex], 0);
			// update scenegraph
			auto viewBone = core->GetRegistry().view<SliceEngine::Bone_Entity>();

			for (auto entity : viewBone)
			{
				UpdateBoneScene(entity);
			}

			auto viewAnimator = core->GetRegistry().view<SliceEngine::Animator>();
			for (auto entity : viewAnimator)
			{
				UpdateBones(entity);
			}
		}

		ImGui::SameLine();

		bool wasLoop = mTimeline.isLoop;
		if (wasLoop)
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

		if (ImGui::Button("Loop"))
		{
			mTimeline.isLoop = !mTimeline.isLoop;
		}

		if (wasLoop)
			ImGui::PopStyleColor();


		// run timeline here temporarily
		
		
		if (mCurrentAnimator)
		{
			if (mTimeline.isPlaying)
			{
				currentFrame++;
				if (currentFrame > endFrame)
				{
					currentFrame = startFrame;
				}

				auto core = SliceEngine::Core::GetInstance();

				//core->GetSystem<SliceEngine::AnimatorSystem>().UpdateAnimation(*mCurrentAnimator, core->GetFramerateManager()->getDeltaTime());

				//Bone animation
				if (mCurrentAnimator->is_bone)
				{
					auto& anim = animationClips[mCurrentClipIndex];
					if (anim->duration <= 0.0f)
					{
						mCurrentTime = 0.0f;
					}
					else
					{
						float dt = static_cast<float>(core->GetFramerateManager()->getFixedDeltaTime());
						mCurrentTime += dt;

						if (mCurrentTime > anim->duration)
						{

							if (!mTimeline.isLoop)
							{
								mTimeline.isPlaying = false;
								currentFrame = startFrame;
								ret = true;
							}
							else
							{
								mTimeline.isPlaying = true;
								mCurrentTime = std::fmod(mCurrentTime, anim->duration);

							}
						}
					}
					if(!ret)
					{
						float safe_time = std::min(mCurrentTime, anim->duration);
						//anim->UpdateTransforms(mCurrentAnimator->final_tforms, safe_time, *mCurrentAnimator->Handle_skeleton.get());
						UpdateTransform(anim, safe_time);
					}
				}

				//core->GetSystem<SliceEngine::BoneSystem>().Update_Scenegraph();
				if (!ret)
				{
					// update scenegraph
					auto viewBone = core->GetRegistry().view<SliceEngine::Bone_Entity>();

					for (auto entity : viewBone)
					{
						UpdateBoneScene(entity);
					}

					auto viewAnimator = core->GetRegistry().view<SliceEngine::Animator>();
					for (auto entity : viewAnimator)
					{
						UpdateBones(entity);
					}
				}
					
				//core->GetSystem<SliceEngine::AnimatorSystem>().BoneUpdate();
			}
			
		}

		ImGui::EndGroup();
#pragma endregion

		ImGui::Separator();

#pragma region Animation Timeline
		ImGui::BeginGroup();

		std::string preview = "No Animations";

		if (hasAnimator)
		{
			if (animationClips.size() > 0)
			{
				if (animationClips[mCurrentClipIndex]->name.empty())
				{
					preview = std::to_string(mCurrentClipIndex);
				}
				else
				{
					preview = animationClips[mCurrentClipIndex]->name;
				}
				

			}
		}

		if (ImGui::BeginCombo("##anim_clips", preview.c_str()))
		{
			for (size_t i = 0; i < animationClips.size(); i++)
			{
				std::string anim_name = animationClips[i]->name;
				if (anim_name.empty())
				{
					anim_name = std::to_string(i);
				}

				if (ImGui::Selectable(anim_name.c_str()))
				{
					mCurrentClipIndex = i;
					LoadDataFromAnimationClip(mCurrentAnimator->Handle_curr_anim_pkg.get()->animations[i]);
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