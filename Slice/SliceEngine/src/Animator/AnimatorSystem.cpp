#include <pch.h>
#include "AnimatorSystem.h"
#include <glm/gtc/matrix_transform.hpp> // For translate, rotate, scale
#include <glm/gtc/quaternion.hpp>      // For quaternions

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace SliceEngine
{
	AnimatorSystem::AnimatorSystem() 
	{
		final_tforms.resize(MAX_BONES, glm::mat4(1.0f));
	}

	void AnimatorSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		Animator& animator = reg.get<Animator>(entity);
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

		/*
		use .compare
		if(animator.stateMachine.prevState != animator.stateMachine.currState->stateName)
		{
			interp (animator.stateMachine.stateMap[stateMachine.prevState],animator.stateMachine.currState)

			animator.stateMachine.prevState = animator.stateMachine.currState->stateName;
		}
		*/
	}

	void AnimatorSystem::UpdateAnimation(float dt) 
	{
		current_time += dt;

		//Bone animation
		if (is_bone) {
			auto const& anim = curr_anim_pkg->animations[curr_anim_idx];

			while (current_time > anim.duration) {
				current_time -= anim.duration;
				curr_anim_idx = (curr_anim_idx + 1) % curr_anim_pkg->animations.size();
				if (anim.duration <= 0.f) {
					return;
				}
			}

			anim.UpdateTransforms(final_tforms, current_time, *skeleton);
		}
		//non bone animation
		else {

		}
	}
	void AnimatorSystem::BoneUpdate() {
		if (is_bone) {
			auto const& anim = curr_anim_pkg->animations[curr_anim_idx];

			anim.ApplyParentTransforms(final_tforms, *skeleton, glm::identity<glm::mat4>());
			anim.ApplyInverseBind(final_tforms, *skeleton);
		}
	}

	void AnimatorSystem::SetAnimationPackage(SliceEngineTypes::AnimationPackage* anim)
	{
		curr_anim_pkg = anim;
	}
	void AnimatorSystem::SetSkeleton(SliceEngineTypes::Skeleton* skele) {
		skeleton = skele;
	}
	void AnimatorSystem::PlayAnimation(unsigned int idx) {
		curr_anim_idx = idx;
	}

}