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
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto sceneSystem = Core::GetInstance()->GetSceneSystem();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);
		glm::vec3 entityVel = { 0.f, 0.f, 0.f }; // Placeholder for physics velocity


		if (sceneSystem->mCurrentState == SceneState::PLAY_SCENE)
		{
			if (!audioComp.channel && audioComp.playOnAwake)
			{
				audioComp.channel = audioManager->PlaySound(audioComp, transform.position, entityVel);
			}
		}

		std::cout << "Entity entering sound system" << std::endl;
	}

	void AudioSourceSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);


		if (audioComp.channel)
		{
			audioManager->StopSound(audioComp.channel);
			audioComp.channel = nullptr;
		}

		if (audioComp.previewChannel)
		{
			audioManager->StopSound(audioComp.previewChannel);
			audioComp.previewChannel = nullptr;
		}

		std::cout << "Entity exiting sound system" << std::endl;
	}

	void AudioSourceSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto sceneSystem = Core::GetInstance()->GetSceneSystem();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);
		glm::vec3 entityVel = { 0.f ,0.f,0.f };

		if (!audioComp.componentEnabled) // if not enabled do not need to update entity
			return;

		if (sceneSystem->mCurrentState == SceneState::PLAY_SCENE)
		{
			if (audioComp.previewChannel)
			{
				audioComp.playPreview = false;
				audioManager->StopSound(audioComp.previewChannel);
				audioComp.previewChannel = nullptr;

			}

			if ((audioComp.channel == nullptr && audioComp.playOnAwake == true))
			{

				audioComp.channel = audioManager->PlaySound(audioComp, transform.position, entityVel);

			}

			if (audioComp.channel && audioComp.spatialBlend > 0.0f)
			{
				audioManager->SetSound3DPosition(audioComp.channel, audioComp.spatialBlend, transform.position, entityVel);
			}
		}

		if (sceneSystem->mCurrentState == SceneState::DEFAULT)
		{
			if (audioComp.playPreview && !audioComp.previewChannel)
			{
				audioComp.previewChannel = audioManager->PlayEditorPreview(audioComp);
			}
			else if (!audioComp.playPreview && audioComp.previewChannel)
			{
				audioManager->StopSound(audioComp.previewChannel);
				audioComp.previewChannel = nullptr;
			}

			if (audioComp.previewChannel && audioComp.spatialBlend > 0.0f)
			{
				audioManager->SetSound3DPosition(audioComp.previewChannel, audioComp.spatialBlend, transform.position, entityVel);
			}

			if (audioComp.channel && !audioManager->IsChannelPlaying(audioComp.channel))
			{
				audioComp.channel = nullptr;
			}
		}

	}

	void AudioSourceSystem::ComponentUpdate(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);
		//auto& transform = reg.get<Transform>(entity);
		glm::vec3 entityVel = { 0.f ,0.f,0.f };

		if (!audioComp.componentEnabled)
		{
			if (audioComp.channel)
			{

				audioManager->StopSound(audioComp.channel);
				audioComp.channel = nullptr;
			}

			if (audioComp.previewChannel)
			{

				audioManager->StopSound(audioComp.previewChannel);
				audioComp.previewChannel = nullptr;
			}
		}

		if (audioComp.channel != nullptr)
		{
			audioManager->UpdateChannelFromComponent(audioComp.channel, audioComp);
		}

		// works for now but will get back with yy if the way i do it is wrong(should be wrong lol)
		// end of my changes

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
		glm::vec3 vel(0.f);

		auto& camera = Core::GetInstance()->GetRenderManager()->GetGameCamera();
		if (camera.has_value())
		{
			GameObject cameraObj = FactoryInstance.GetGOByEntity(camera.value());
			auto& cameraTrans = reg.get<Transform>(camera.value());
			renderManager->GetCameraAxis(cameraObj, forward, right, up);
			audioManager->SetListenerAttributes(cameraTrans.position, vel, forward, up);

		}
	}
#pragma endregion

	void SoundSystem::Update(float dt)
	{

	}
}