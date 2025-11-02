#include <pch.h>
#include "AnimatorSystem.h"
#include <glm/gtc/matrix_transform.hpp> // For translate, rotate, scale
#include <glm/gtc/quaternion.hpp>      // For quaternions
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
		if (curr_anim) 
		{
			current_time += curr_anim->ticks_per_second * dt;	//needed to convert actual time to animation time
			//for now just inf loop it
			while (current_time > curr_anim->duration)
			{
				current_time -= curr_anim->duration;
			}
			glm::mat4 id = glm::mat4(1.0f);
			CalculateBoneTransform(curr_anim->root, id);
		}
	}

	void AnimatorSystem::PlayAnimation(Animation* anim) 
	{
		current_time = 0.f;
		curr_anim = anim;
	}

	/*
	* There is a clash here because this takes into account of hierachy, where the model load dosent
	* probably need to resolve that somehow
	*/
	void AnimatorSystem::CalculateBoneTransform(SliceEngineTypes::ModelNode const& node, glm::mat4 const& parent_tform) 
	{
		std::string node_name = node.name;

		// 1. Create the Translation Matrix
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), node.position);

		// 2. Create the Rotation Matrix (via Quaternion)
		//    This is the robust way to avoid Gimbal Lock.
		glm::quat rotationQuat = glm::quat(node.rotation);
		glm::mat4 rotationMatrix = glm::toMat4(rotationQuat);

		// 3. Create the Scale Matrix
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), node.scale);

		// 4. Combine them in T * R * S order
		glm::mat4 tform = translationMatrix * rotationMatrix * scaleMatrix;

		//glm::mat4 tform = node;
		glm::mat4 bone_tform;
		//node may not be a bone
		Bone* b{};
		for (auto& bone : curr_anim->bones)
		{
			if (bone.name == node_name)
			{
				b = &bone;
				break;
			}
		}
		//if (b) {
		//	b->Update(current_time);
		//	//tform = b->Get_LocalTform();
		//	bone_tform = b->Get_LocalTform();
		//	auto& boneinfo = curr_anim->bone_map[node_name];
		//	final_tforms[boneinfo.id] = bone_tform * boneinfo.offset;
		//	tform = bone_tform;
		//}
		//else {
		//	tform = parent_tform * tform;
		//}
		if (b) {
			b->Update(current_time);
			//tform = b->Get_LocalTform();
			//bone_tform = b->Get_LocalTform();
		}

		tform = parent_tform * tform;

		if (b) 
		{
			auto& boneinfo = curr_anim->bone_map[node_name];
			bone_tform = bone_tform * boneinfo.offset;
			final_tforms[boneinfo.id] = tform * boneinfo.offset;
		}

		for (auto& c : node.children) 
		{
			CalculateBoneTransform(c, tform);
		}
	}

	void Bone::Update(float anim_time) {
		glm::mat4 translation = InterpolatePosition(anim_time);
		glm::mat4 rotation = InterpolateRotation(anim_time);
		glm::mat4 scale = InterpolateScale(anim_time);
		local_tform = translation * rotation * scale;
	}

	float Bone::GetInterpValue(float last_time, float next_time, float anim_time) {
		return (anim_time - last_time) / (next_time - last_time);
	}

	glm::mat4 Bone::InterpolatePosition(float anim_time) {
		if (positions.size() == 1) {
			return glm::translate(glm::mat4(1.0f), positions[0].pos);
		}

		//Get keyframe idx
		unsigned int idx = GetKeyPositionIdx(anim_time);
		float interp = GetInterpValue(positions[idx].timestamp, positions[idx + 1].timestamp, anim_time);

		glm::vec3 interp_pos = glm::mix(positions[idx].pos, positions[idx + 1].pos, interp);
		return glm::translate(glm::mat4(1.0f), interp_pos);
	}
	glm::mat4 Bone::InterpolateRotation(float anim_time) {
		if (rotations.size() == 1) {
			glm::quat q = glm::normalize(rotations[0].orientation);
			return glm::toMat4(q);
		}

		//Get keyframe idx
		unsigned int idx = GetKeyRotationIdx(anim_time);
		float interp = GetInterpValue(rotations[idx].timestamp, rotations[idx + 1].timestamp, anim_time);

		glm::quat interp_rot = glm::normalize(glm::slerp(rotations[idx].orientation, rotations[idx + 1].orientation, interp));
		return glm::toMat4(interp_rot);
	}
	glm::mat4 Bone::InterpolateScale(float anim_time) {
		if (scales.size() == 1) {
			return glm::scale(glm::mat4(1.0f), scales[0].scale);
		}

		//Get keyframe idx
		unsigned int idx = GetKeyScaleIdx(anim_time);
		float interp = GetInterpValue(scales[idx].timestamp, scales[idx + 1].timestamp, anim_time);

		glm::vec3 interp_scale = glm::mix(scales[idx].scale, scales[idx + 1].scale, interp);
		return glm::scale(glm::mat4(1.0f), interp_scale);
	}

	unsigned int Bone::GetKeyPositionIdx(float time) {
		for (unsigned int i{}; i < positions.size() - 1; ++i) {
			if (time > positions[i + 1].timestamp) {
				return i;
			}
		}
		return 0;
	}
	unsigned int Bone::GetKeyRotationIdx(float time) {
		for (unsigned int i{}; i < rotations.size() - 1; ++i) {
			if (time > rotations[i + 1].timestamp) {
				return i;
			}
		}
		return 0;
	}
	unsigned int Bone::GetKeyScaleIdx(float time) {
		for (unsigned int i{}; i < scales.size() - 1; ++i) {
			if (time > scales[i + 1].timestamp) {
				return i;
			}
		}
		return 0;
	}

}