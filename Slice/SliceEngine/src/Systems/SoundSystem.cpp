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

		//std::cout << "Entity entering sound system" << std::endl;
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

		//std::cout << "Entity exiting sound system" << std::endl;
	}

	void AudioSourceSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto sceneSystem = Core::GetInstance()->GetSceneSystem();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);
		glm::vec3 entityVel = { 0.f ,0.f,0.f };
		glm::vec3 audioWorldPos = transform.GetWorldPosition();
		//SLICE_LOG("transform world pos: (" + std::to_string(audioWorldPos.x) + "," + std::to_string(audioWorldPos.y) +"," + std::to_string(audioWorldPos.z) + ")");
		if (!audioComp.componentEnabled) // if not enabled do not need to update entity
			return;


		

		if (audioComp.channel != nullptr)
		{
			bool isPlaying = false;
			FMOD_RESULT res = audioComp.channel->isPlaying(&isPlaying);

			
			if (res != FMOD_OK || !isPlaying)
			{
				audioComp.channel = nullptr;

				if (audioComp.destroyOnEnd)
				{
					FactoryInstance.Destroy(entity);
					return; // Entity destroyed, skip further updates
				}
			}
		}

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
				audioComp.channel = audioManager->PlaySound(audioComp, audioWorldPos, entityVel);

			}

			if (audioComp.channel && audioComp.spatialBlend > 0.0f)
			{
				glm::vec3 listenerPos, lVel, lForward, lUp;
				audioManager->Get3DListenerAttributes(listenerPos, lVel, lForward, lUp);

				// Calculate the actual distance between the listener and this audio source
				//float distance = glm::distance(listenerPos, audioWorldPos);

				// Debug Log: Check if this value is changing as you move
				//SLICE_LOG("Distance to Sound: " + std::to_string(distance));

				FMOD_RESULT res = audioComp.channel->set3DAttributes(
					(FMOD_VECTOR*)&audioWorldPos,
					(FMOD_VECTOR*)&entityVel
				);

				if (res != FMOD_OK)
				{
					SLICE_LOG_ERROR("FMOD Error updating 3D Position: " + std::to_string(res));
				}
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
				audioManager->SetSound3DPosition(audioComp.previewChannel, audioComp.spatialBlend, audioWorldPos, entityVel);
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
		//auto renderManager = Core::GetInstance()->GetRenderManager();


		auto& transform = reg.get<Transform>(entity);
		glm::vec3 worldPos = transform.GetWorldPosition();
		glm::vec3 velocity(0.f);

		// Use the same convention as RenderManager::GetCameraAxis
		// Forward = col 0 (X), Up = col 1 (Y), Right = col 2 (Z)
		glm::vec3 forward = glm::normalize(glm::vec3(transform.transform[2]));
		glm::vec3 up = glm::normalize(glm::vec3(transform.transform[1]));
		glm::vec3 right = glm::normalize(glm::vec3(transform.transform[0]));

		
		//auto& cameraOpt = Core::GetInstance()->GetRenderManager()->GetGameCamera();
		/*if (cameraOpt.has_value() && cameraOpt.value() == entity)
		{
			
			GameObject cameraObj = FactoryInstance.GetGOByEntity(entity);
			Core::GetInstance()->GetRenderManager()->GetCameraAxis(cameraObj, forward, right, up);
		}*/

		audioManager->SetListenerAttributes(worldPos, velocity, forward, up);

	}

	void AudioListenerSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{

	}

	void AudioListenerSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		//auto renderManager = Core::GetInstance()->GetRenderManager();

		auto& transform = reg.get<Transform>(entity);
		glm::vec3 worldPos = transform.GetWorldPosition();
		glm::vec3 velocity(0.f);

		// Use the same convention as RenderManager::GetCameraAxis
		// Forward = col 0 (X), Up = col 1 (Y), Right = col 2 (Z)
		glm::vec3 forward = glm::normalize(glm::vec3(transform.transform[2]));
		glm::vec3 up = glm::normalize(glm::vec3(transform.transform[1]));
		glm::vec3 right = glm::normalize(glm::vec3(transform.transform[0]));


		auto& cameraOpt = Core::GetInstance()->GetRenderManager()->GetGameCamera();
		if (cameraOpt.has_value() && cameraOpt.value() == entity)
		{

			GameObject cameraObj = FactoryInstance.GetGOByEntity(entity);
			Core::GetInstance()->GetRenderManager()->GetCameraAxis(cameraObj, forward, right, up);
		}

		audioManager->SetListenerAttributes(worldPos, velocity, forward, up);
	}
#pragma endregion

	void SoundSystem::Update(float dt)
	{
		
	}
}