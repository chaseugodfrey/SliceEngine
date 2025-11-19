#include <pch.h>
#include "ECS/ECSTypes.h"
#include "AnimatorSystem.h"
#include <glm/gtc/matrix_transform.hpp> // For translate, rotate, scale
#include <glm/gtc/quaternion.hpp>      // For quaternions
#include "../Core/Core.h"
#include "../Resource/Skeleton.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace SliceEngine
{
	AnimatorSystem::AnimatorSystem() 
	{
		//final_tforms.resize(MAX_BONES, glm::mat4(1.0f));
	}

	void AnimatorSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto core = Core::GetInstance();

		Animator& animator = reg.get<Animator>(entity);

		animator.final_tforms.resize(MAX_BONES, glm::mat4(1.0f));
		//animator.stateMachine.EFSM = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::StateMachine>((GUID)9857886709116471337);
		animator.Handle_stateMachine = core->GetResourceManager()->get<SliceEngineTypes::StateMachine>((GUID)9857886709116471337);

		animator.Handle_skeleton = core->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Skeleton>(static_cast<GUID>(11169558507216259861));
		animator.Handle_curr_anim_pkg = core->GetResourceManager()->get<SliceEngine::SliceEngineTypes::AnimationPackage>(static_cast<GUID>(16139273559357172266));

		if(animator.IsValid())
		{
			animator.curr_anim_pkg = *animator.Handle_curr_anim_pkg.get();
			animator.stateMachine.EFSM = *animator.Handle_stateMachine.get();

			animator.stateMachine.InitState(animator.curr_anim_pkg);
		}
	}

	void AnimatorSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		Animator& animator = reg.get<Animator>(entity);
		animator.stateMachine.OnExit();
	}
	void AnimatorSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{

		Animator& animator = reg.get<Animator>(entity);

		animator.stateMachine.CheckStates();

		animator.stateMachine.UpdateState(animator.current_time);

		UpdateAnimation(animator, dt);
	}

	void AnimatorSystem::UpdateAnimation(Animator& animator, float dt)
	{
		//if (!animator.stateMachine.EFSM.IsValid()) return;
		if (!animator.IsValid()) return;

		if (animator.stateMachine.stateChanged)
		{
			if (animator.is_bone)
			{
				auto& prevanim = animator.curr_anim_pkg.animations[animator.stateMachine.EFSM.stateMap[animator.stateMachine.EFSM.prevState].curr_anim_idx];
				auto& curranim = animator.curr_anim_pkg.animations[animator.stateMachine.EFSM.currState->curr_anim_idx];

				for (int i = 0; i < prevanim.boneKeyFrames.size(); i++)
				{
					if(prevanim.boneKeyFrames[i].animated && curranim.boneKeyFrames[i].animated)
						curranim.boneKeyFrames[i].transforms[0] = SliceEngineTypes::Frame::Blend(prevanim.boneKeyFrames[i].transforms[prevanim.boneKeyFrames[i].transforms.size() - 1], curranim.boneKeyFrames[i].transforms[0], dt);
				}
			}
			animator.stateMachine.stateChanged = false;
		}

		//if (animator.timeline.isPlaying)
		{
			//Bone animation
			if (animator.is_bone) {
				auto const& anim = animator.curr_anim_pkg.animations[animator.stateMachine.EFSM.currState->curr_anim_idx];
				if (anim.duration <= 0.0f)
				{
					// This is a static pose. Don't advance time, just hold frame 0.
					animator.current_time = 0.0f;
				}
				else
				{
					animator.current_time += dt;
					animator.stateMachine.UpdateCurrentTime(animator.current_time);
					if (animator.current_time > anim.duration)
					{

						if (!animator.stateMachine.EFSM.currState->isLoop)
						{
							animator.timeline.isPlaying = false;
							animator.current_time = 0.0f;

							return;
						}
						else
						{
							animator.timeline.isPlaying = true;
							animator.current_time = std::fmod(animator.current_time, anim.duration);
						}
					}

				}
				float safe_time = std::min(animator.current_time, anim.duration);
				anim.UpdateTransforms(animator.final_tforms, safe_time, *animator.Handle_skeleton.get());
			}
			//non bone animation
			else {

			}
		}
	}
	void AnimatorSystem::BoneUpdate() {
		auto view = SliceEngine::Core::GetInstance()->GetRegistry().view<Animator>();
		for (auto entity : view)
		{
			Animator& animator = SliceEngine::Core::GetInstance()->GetRegistry().get<Animator>(entity);
			Transform& transform = SliceEngine::Core::GetInstance()->GetRegistry().get<Transform>(entity);

			//if (!animator.stateMachine.EFSM.IsValid()) return;
			if (!animator.IsValid()) return;

			//if (animator.timeline.isPlaying)
			{
				
				if (animator.is_bone) {
					auto const& anim = animator.curr_anim_pkg.animations[animator.stateMachine.EFSM.currState->curr_anim_idx];

					anim.ApplyParentTransforms(animator.final_tforms, *animator.Handle_skeleton.get(), transform.transform);
					animator.SetInverseRoots();
					anim.ApplyInverseBind(animator.final_tforms, *animator.Handle_skeleton.get());
				}
				
			}

		}

	}


	void AnimatorSystem::InitSystem()
	{
		auto core = Core::GetInstance();

		for (auto entity : core->GetRegistry().view<Animator>())
		{
			Animator& animator = core->GetRegistry().get<Animator>(entity); 
			
			if (!animator.IsValid()) return;

			animator.stateMachine.InitState(animator.curr_anim_pkg);

			animator.timeline.isPlaying = true;


			animator.timeline.isLoop = animator.stateMachine.EFSM.currState->isLoop;

		}
	}
}