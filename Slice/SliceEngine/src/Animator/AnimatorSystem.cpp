#include <pch.h>
#include "ECS/ECSTypes.h"
#include "AnimatorSystem.h"
#include <glm/gtc/matrix_transform.hpp> // For translate, rotate, scale
#include <glm/gtc/quaternion.hpp>      // For quaternions
#include "../Core/Core.h"

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
		Animator& animator = reg.get<Animator>(entity);

		animator.final_tforms.resize(MAX_BONES, glm::mat4(1.0f));
		animator.Handle_skeleton = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Skeleton>(static_cast<GUID>(02020202020202020202));
		animator.Handle_curr_anim_pkg = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::AnimationPackage>(static_cast<GUID>(03030303030303030303));

		animator.curr_anim_pkg = *animator.Handle_curr_anim_pkg.get();

		animator.stateMachine.InitState();

		animator.animTimer = 0.0f;
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

		animator.stateMachine.UpdateState();

		{
			animator.animTimer += dt;
		}

		UpdateAnimation(animator, dt);

		/*
		use .compare
		if(animator.stateMachine.prevState != animator.stateMachine.currState->stateName)
		{
			interp (animator.stateMachine.stateMap[stateMachine.prevState],animator.stateMachine.currState)

			animator.stateMachine.prevState = animator.stateMachine.currState->stateName;
		}
		*/
	}

	void AnimatorSystem::UpdateAnimation(Animator& animator, float dt)
	{
		animator.current_time += dt;

		//Bone animation
		if (animator.is_bone) {
			auto const& anim = animator.curr_anim_pkg.animations[animator.curr_anim_idx];

			while (animator.current_time > anim.duration) {
				animator.current_time -= anim.duration;
				animator.curr_anim_idx = (animator.curr_anim_idx + 1) % animator.curr_anim_pkg.animations.size();
				if (anim.duration <= 0.f) {
					return;
				}
			}

			anim.UpdateTransforms(animator.final_tforms, animator.current_time, *animator.Handle_skeleton.get());
		}
		//non bone animation
		else {

		}
	}
	void AnimatorSystem::BoneUpdate() {
		auto view = SliceEngine::Core::GetInstance()->GetRegistry().view<Animator>();
		for (auto entity : view)
		{
			Animator& animator = SliceEngine::Core::GetInstance()->GetRegistry().get<Animator>(entity);


			if (animator.is_bone) {
				auto const& anim = animator.curr_anim_pkg.animations[animator.curr_anim_idx];

				anim.ApplyParentTransforms(animator.final_tforms, *animator.Handle_skeleton.get(), glm::identity<glm::mat4>());
				animator.SetInverseRoots();
				anim.ApplyInverseBind(animator.final_tforms, *animator.Handle_skeleton.get());
			}
		}
	}
}