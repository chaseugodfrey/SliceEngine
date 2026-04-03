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
		InitAnimatorEntity(reg, entity);
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

		animator.stateMachine.UpdateState(animator.current_time,dt);

		UpdateAnimation(reg,entity,animator, dt);
	}

	void AnimatorSystem::UpdateAnimation(entt::registry& reg, entt::entity entity, Animator& animator, float dt)
	{
		//if (!animator.stateMachine.EFSM.IsValid()) return;
		if (!animator.Handle_stateMachine.IsValid())
		{
			InitAnimatorEntity(reg, entity);
		}


		// have a blending timer, have blending per frame until timer reach
		// then add blending weight blending timer/ blend length
		if (animator.stateMachine.stateChanged)
		{
			if (animator.timeline.isPlaying)
			{
				if (animator.is_bone)
				{ 
					if (animator.curr_anim_pkg.animations.size() > 0 && animator.curr_anim_pkg.animations.size() > animator.stateMachine.EFSM.currState->curr_anim_idx
						&& animator.curr_anim_pkg.animations.size() > animator.stateMachine.EFSM.stateMap[animator.stateMachine.EFSM.prevState].curr_anim_idx)
					{
						auto& prevanim = animator.curr_anim_pkg.animations[animator.stateMachine.EFSM.stateMap[animator.stateMachine.EFSM.prevState].curr_anim_idx];
						auto& curranim = animator.curr_anim_pkg.animations[animator.stateMachine.EFSM.currState->curr_anim_idx];
						float frameTime = animator.current_time * prevanim.fps;
						if (!animator.stateMachine.EFSM.stateMap[animator.stateMachine.EFSM.prevState].isFinish)
						{
							for (int i = 0; i < prevanim.boneKeyFrames.size(); i++)
							{
								glm::mat4 local_tform{};
								if (prevanim.boneKeyFrames[i].animated && curranim.boneKeyFrames[i].animated)
									local_tform = SliceEngineTypes::Frame::Blend(prevanim.boneKeyFrames[i].transforms[(int)frameTime], curranim.boneKeyFrames[i].transforms[0], dt).ToMatrix();

								animator.final_tforms[i] = local_tform;
							}
						}
					}
				}
			}
			animator.timeline.isPlaying = true;
			animator.stateMachine.stateChanged = false;
		}

		if (animator.timeline.isPlaying)
		{
			//Bone animation
			if (animator.Handle_skeleton.IsValid()) {
				if(animator.curr_anim_pkg.animations.size() > 0 && animator.curr_anim_pkg.animations.size() > animator.stateMachine.EFSM.currState->curr_anim_idx)
				{
					
					auto const& anim = animator.curr_anim_pkg.animations[animator.stateMachine.EFSM.currState->curr_anim_idx];
					if (anim.duration <= 0.0f)
					{
						// This is a static pose. Don't advance time, just hold frame 0.
						animator.current_time = 0.0f;
					}
					else
					{
						animator.current_time += dt * animator.stateMachine.EFSM.currState->animationSpeed;

						// publish animation key frame event
						unsigned int currentFrame = static_cast<unsigned int>(animator.current_time * anim.fps);
						for (auto eventFrame : animator.eventFrames)
						{
							//SLICE_LOG_VALUES("Event Frame:", eventFrame.frameNumber, "Current Frame:", currentFrame, "Anim Idx:", eventFrame.animIdx, "Curr Anim Idx:", animator.stateMachine.EFSM.currState->curr_anim_idx);
							if (eventFrame.frameNumber == currentFrame && eventFrame.animIdx == animator.stateMachine.EFSM.currState->curr_anim_idx)
							{
								// publish event
								AnimationEvent addEvent{ eventFrame.scriptFunc,eventFrame.scriptName, entity };
								EventManager::GetInstance()->Publish<AnimationEvent>(addEvent);
							}
						}

						if (animator.current_time > anim.duration)
						{

							if (!animator.stateMachine.EFSM.currState->isLoop)
							{
								animator.timeline.isPlaying = false;
								// fsm set time
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
			}
			//non bone animation
			else 
			{
				if (animator.curr_anims.animations.size() > 0)
				{
					auto& anim = animator.curr_anims.animations[animator.stateMachine.EFSM.currState->curr_anim_idx];
					if (anim.duration <= 0.0f)
					{
						animator.current_time = 0.0f;
					}
					else
					{
						animator.current_time += dt * animator.stateMachine.EFSM.currState->animationSpeed;

						// publish animation key frame event
						unsigned int currentFrame = static_cast<unsigned int>(animator.current_time * anim.fps);
						for (auto eventFrame : animator.eventFrames)
						{
							//SLICE_LOG_VALUES("Event Frame:", eventFrame.frameNumber, "Current Frame:", currentFrame, "Anim Idx:", eventFrame.animIdx, "Curr Anim Idx:", animator.stateMachine.EFSM.currState->curr_anim_idx);
							if (eventFrame.frameNumber == currentFrame && eventFrame.animIdx == animator.stateMachine.EFSM.currState->curr_anim_idx)
							{
								// publish event
								AnimationEvent addEvent{ eventFrame.scriptFunc,eventFrame.scriptName, entity };
								EventManager::GetInstance()->Publish<AnimationEvent>(addEvent);
							}
						}

						if (animator.current_time > anim.duration)
						{

							if (!animator.stateMachine.EFSM.currState->isLoop)
							{
								animator.timeline.isPlaying = false;
								// fsm set time
								return;
							}
							else
							{
								animator.timeline.isPlaying = true;
								animator.current_time = std::fmod(animator.current_time, anim.duration);
							}
						}

					}

					// this has to  be my own not the skeleton 1
					float safe_time = std::min(animator.current_time, anim.duration);

					anim.UpdateTransforms(reg,entity, safe_time);
				}
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
			if (!animator.IsValid()) continue;

			if (animator.timeline.isPlaying)
			{
				
				if (animator.is_bone)
				{
					if (animator.curr_anim_pkg.animations.size() > 0 && animator.curr_anim_pkg.animations.size() > animator.stateMachine.EFSM.currState->curr_anim_idx)
					{
						auto const& anim = animator.curr_anim_pkg.animations[animator.stateMachine.EFSM.currState->curr_anim_idx];

						anim.ApplyParentTransforms(animator.final_tforms, *animator.Handle_skeleton.get(), transform.transform);
						animator.SetInverseRoots();
						anim.ApplyInverseBind(animator.final_tforms, *animator.Handle_skeleton.get());
					}
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
			
			if (!animator.Handle_stateMachine.IsValid()) return;

			animator.stateMachine.InitState(animator.curr_anim_pkg);

			animator.timeline.isPlaying = true;


			animator.timeline.isLoop = animator.stateMachine.EFSM.currState->isLoop;

		}
	}

	
	void AnimatorSystem::InitAnimatorEntity(entt::registry& reg, entt::entity entity)
	{
		auto core = Core::GetInstance();

		Animator& animator = reg.get<Animator>(entity);

		animator.final_tforms.resize(MAX_BONES, glm::mat4(1.0f));

		auto rscMgr = core->GetResourceManager();

		GUID defCtrl = rscMgr->mFileNameToGUID["Default/BasicController.controller"];

		animator.Handle_stateMachine = core->GetResourceManager()->get<SliceEngineTypes::StateMachine>(animator.Handle_stateMachine.getGUID());
		if(!animator.Handle_stateMachine.IsValid())
			animator.Handle_stateMachine = core->GetResourceManager()->get<SliceEngineTypes::StateMachine>(defCtrl);
		animator.Handle_skeleton = core->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Skeleton>(animator.Handle_skeleton.getGUID());
		animator.Handle_curr_anim_pkg = core->GetResourceManager()->get<SliceEngine::SliceEngineTypes::AnimationPackage>(animator.Handle_curr_anim_pkg.getGUID());
		animator.Handle_Anims = core->GetResourceManager()->get<SliceEngine::SliceEngineTypes::SequencePackage>(animator.Handle_Anims.getGUID());

		if (animator.Handle_stateMachine.IsValid())
		{
			animator.stateMachine.EFSM = *animator.Handle_stateMachine.get();
			animator.stateMachine.InitState();
		}

		if (animator.Handle_Anims.IsValid())
		{
			animator.curr_anims = *animator.Handle_Anims.get();
			animator.stateMachine.InitState(animator.curr_anims);
		}

		if (animator.IsValid())
		{
			animator.curr_anim_pkg = *animator.Handle_curr_anim_pkg.get();
			animator.stateMachine.InitState(animator.curr_anim_pkg);
		}

	}
	GUID AnimatorSystem::GetMdlGUID(entt::registry& reg, entt::entity entity, SceneGraph& scene_graph)
	{
		auto child_entity = scene_graph.neighbours[SceneGraph::DOWN];

		while (child_entity != entt::null)
		{
			auto& child_scene_graph = reg.get<SceneGraph>(child_entity);
			auto rend = reg.try_get<Renderer>(child_entity);

			if (rend)
			{
				std::string name = FactoryInstance.GetGOByEntity(child_entity).GetName();
				return rend->modelHandle.getGUID();
			}

			GetMdlGUID(reg, child_entity, child_scene_graph);

			child_entity = child_scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
		}
		return GUID(0);
	}
}