/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SoundSystem.cpp

 author:	  Lee Yong Yee

 email:       l.yongyee@digipen.edu

 brief:		  Defines the AudioManager class and related audio structures for handling sound playback
			  within the engine using the FMOD sound library. This system manages loading, playing,
			  and updating 2D and 3D sounds, maintaining category-based volume control, and handling
			  sound states such as pause, looping, and positional audio.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "SoundSystem.h"
#include "../Audio/AudioManager.h"
#include "Systems/SceneSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Graphics/RenderManager.h"


namespace SliceEngine
{
#pragma region AUDIO SOURCE
	void AudioSourceSystem::BindToAudioSource()
	{
		mRegistry->on_update<AudioSource>().connect<&AudioSourceSystem::ComponentUpdate>(this);
		//mRegistry->on_update<AudioSource>().connect<&SoundSystem::onPauseUpdated>(this);
	}

	void AudioSourceSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		
		
		//auto& audioComp = reg.get<AudioSource>(entity);
		
		

		
		//audioManager->PlaySound(audioComp.soundName, SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, audioComp.is3D, audioComp.isPaused, audioComp.isLoop, audioComp.currentVolume, entity, transform.position);

		

		std::cout << "Entity entering sound system" << std::endl;
	}

	void AudioSourceSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);


		if (audioManager->IsChannelPlaying(audioComp.channel))
		{
			audioManager->StopSound(audioComp.channel);
		}
		else if (audioManager->IsChannelPlaying(audioComp.previewChannel))
		{
			audioManager->StopSound(audioComp.previewChannel);
		}

		std::cout << "Entity exiting sound system" << std::endl;
	}

	void AudioSourceSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto sceneSystem = Core::GetInstance()->GetSceneSystem();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);
		glm::vec3 entityVel = { 0.f ,0.f,0.f};

		if (!audioComp.componentEnabled) // if not enabled do not need to update entity
			return;

		if (sceneSystem->mCurrentState == SceneState::PLAY_SCENE)
		{
			if (audioComp.previewChannel && audioComp.playPreview == true)
			{
				audioComp.playPreview = false;
				audioManager->StopSound(audioComp.previewChannel);

			}

			if ((audioComp.channel == nullptr && audioComp.playOnAwake == true))
			{
				
				audioComp.channel = audioManager->PlaySound(audioComp, transform.position, entityVel);

			}
		}

		if (sceneSystem->mCurrentState == SceneState::PAUSE_SCENE)
		{
			if (audioComp.channel)
			{
				audioComp.isPaused = true;
				audioManager->SetPauseState(audioComp.channel, audioComp.isPaused);
			}
		}

		if (sceneSystem->mNextState == SceneState::STOP_SCENE)
		{
			if (audioComp.channel)
			{
				audioManager->StopSound(audioComp.channel);
			}
		}

		if (audioComp.spatialBlend > 0.0f)
		{
			if (audioComp.channel && sceneSystem->mCurrentState == SceneState::PLAY_SCENE)
			{
				
				audioManager->SetSound3DPosition(audioComp.channel,audioComp.spatialBlend, transform.position, entityVel);

			}
			else if (audioComp.previewChannel && sceneSystem->mCurrentState == SceneState::DEFAULT)
			{
				audioManager->SetSound3DPosition(audioComp.previewChannel, audioComp.spatialBlend, transform.position, entityVel);
			}

		}
		
	}

	void AudioSourceSystem::ComponentUpdate(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);
		glm::vec3 entityVel = { 0.f ,0.f,0.f };
			
		
		if (audioComp.channel != nullptr)
		{
			audioManager->UpdateChannelFromComponent(audioComp.channel, audioComp);
		}

		// works for now but will get back with yy if the way i do it is wrong(should be wrong lol)
		if (!audioComp.componentEnabled)
		{
			if(audioManager->IsChannelPlaying(audioComp.channel))
			audioManager->StopSound(audioComp.channel);

			if (audioManager->IsChannelPlaying(audioComp.previewChannel))
			audioManager->StopSound(audioComp.previewChannel);
		}
		else if(audioComp.componentEnabled && audioComp.playOnAwake)
		{
			if(!audioManager->IsChannelPlaying(audioComp.channel))
			audioComp.channel = audioManager->PlaySound(audioComp, transform.position, entityVel);
		}
		// end of my changes
		
		if (audioComp.componentEnabled && audioComp.playPreview && (audioManager->IsChannelPlaying(audioComp.previewChannel) == false || audioComp.previewChannel == nullptr))
		{	

			audioComp.previewChannel = audioManager->PlayEditorPreview(audioComp);
  
		}
		else if(audioComp.playPreview == false && audioManager->IsChannelPlaying(audioComp.previewChannel) == true)
		{

			audioManager->StopSound(audioComp.previewChannel);
			
		}
	}
#pragma endregion

#pragma region AUDIO LISTENER

	void AudioListenerSystem::BindToAudioListener()
	{

	}

	void AudioListenerSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto renderManager = Core::GetInstance()->GetRenderManager();


		auto& transform = reg.get<Transform>(entity);
		//glm::vec3 entityVel = Core::GetInstance()->GetSystem<PhysicsSystem>().GetLinearVelocity(entity);
		glm::vec3 up, forward, right;
		glm::vec3 vel(0.f);

		GameObject camera = FactoryInstance.GetGOByEntity(entity);
		renderManager->GetCameraAxis(camera, forward, right, up);
		audioManager->SetListenerAttributes(transform.position, vel, forward, up);
		
	}

	void AudioListenerSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{

	}

	void AudioListenerSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto renderManager = Core::GetInstance()->GetRenderManager();

		auto& transform = reg.get<Transform>(entity);
		auto& audioListener = reg.get<AudioListener>(entity);

		if (!audioListener.componentEnabled)
			return;

		//glm::vec3 entityVel = Core::GetInstance()->GetSystem<PhysicsSystem>().GetLinearVelocity(entity);
		glm::vec3 up, forward, right;
		glm::vec3 vel( 0.f);

		GameObject camera = FactoryInstance.GetGOByEntity(entity);
		renderManager->GetCameraAxis(camera, forward, right, up);
		audioManager->SetListenerAttributes(transform.position, vel, forward, up);
	}
#pragma endregion

	void SoundSystem::Update(float dt)
	{
		//auto reg = Core::GetInstance()->
		//auto view = Core::GetInstance()->GetRegistry().view<AudioSourceEntity>();

		//for (auto entity : view)
		//{
		//	auto audioManager = Core::GetInstance()->GetAudioManager();
		//	auto sceneSystem = Core::GetInstance()->GetSceneSystem();
		//	auto& audioComp = reg.get<AudioSource>(entity);
		//	auto& transform = reg.get<Transform>(entity);
		//	//glm::vec3 entityVel = Core::GetInstance()->GetSystem<PhysicsSystem>().GetLinearVelocity(entity);

		//	if (sceneSystem->mCurrentState == SceneState::PLAY_SCENE)
		//	{
		//		if (audioComp.previewChannel && audioComp.playPreview == true)
		//		{
		//			audioComp.playPreview = false;
		//			audioManager->StopSound(audioComp.previewChannel);

		//		}

		//		if (audioComp.channel == nullptr && audioComp.playOnAwake == true)
		//		{

		//			audioComp.channel = audioManager->PlaySound(audioComp, transform.position, glm::vec3(0.f));

		//		}
		//	}

		//	if (sceneSystem->mCurrentState == SceneState::PAUSE_SCENE)
		//	{
		//		if (audioComp.channel)
		//		{
		//			audioComp.isPaused = true;
		//			audioManager->SetPauseState(audioComp.channel, audioComp.isPaused);
		//		}
		//	}

		//	if (sceneSystem->mNextState == SceneState::STOP_SCENE)
		//	{
		//		if (audioComp.channel)
		//		{
		//			audioManager->StopSound(audioComp.channel);
		//		}
		//	}

		//	if (audioComp.spatialBlend > 0.0f)
		//	{
		//		if (audioComp.channel && sceneSystem->mCurrentState == SceneState::PLAY_SCENE)
		//		{

		//			audioManager->SetSound3DPosition(audioComp.channel, audioComp.spatialBlend, transform.position, glm::vec3(0.f));

		//		}
		//		else if (audioComp.previewChannel && sceneSystem->mCurrentState == SceneState::DEFAULT)
		//		{
		//			audioManager->SetSound3DPosition(audioComp.previewChannel, audioComp.spatialBlend, transform.position, glm::vec3(0.f));
		//		}

		//	}
		//}
	}
}