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


namespace SliceEngine
{
	void SoundSystem::BindToAudioSource()
	{
		mRegistry->on_update<AudioSource>().connect<&SoundSystem::ComponentUpdate>(this);
		//mRegistry->on_update<AudioSource>().connect<&SoundSystem::onPauseUpdated>(this);
	}

	void SoundSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		//auto& audioComp = reg.get<AudioSource>(entity);
		
		

		
		//audioManager->PlaySound(audioComp.soundName, SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, audioComp.is3D, audioComp.isPaused, audioComp.isLoop, audioComp.currentVolume, entity, transform.position);

		

		std::cout << "Entity entering sound system" << std::endl;
	}

	void SoundSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto audioComp = reg.get<AudioSource>(entity);


		if (audioManager->IsChannelPlaying(audioComp.channel))
		{
			audioManager->StopSound(audioComp.channel);
		}
		else if (audioManager->IsChannelPlaying(audioComp.previewChannel))
		{
			audioManager->StopEditorPreview(audioComp.previewChannel);
		}

		std::cout << "Entity exiting sound system" << std::endl;
	}

	void SoundSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);

		audioManager->SetSound3DPosition(audioComp.channel,audioComp.is3D, transform.position, glm::vec3{0.f});
		
	}

	void SoundSystem::ComponentUpdate(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);


		
		if (audioComp.channel != nullptr)
		{

			if (audioComp.currentVolume != audioManager->GetChannelVolume(audioComp.channel))
			{
				
				audioManager->SetChannelVolume(audioComp.channel, audioComp.currentVolume);
			
			}

			if (audioComp.isPaused != audioManager->GetPauseState(audioComp.channel))
			{
				audioManager->UpdatePauseSound(audioComp.channel, audioComp.isPaused);
			}

			if (audioComp.is3D != audioManager->IsFMOD3D(audioComp.channel))
			{
				audioManager->UpdateFMODMode(audioComp.channel, audioComp.is3D);
				if (audioComp.is3D)
				{
					audioManager->SetSound3DPosition(audioComp.channel, audioComp.is3D, transform.position, glm::vec3{ 0.f, 0.f, 0.f });
				}
			}

		}

		

		

		if (audioComp.playPreview && (audioManager->IsChannelPlaying(audioComp.previewChannel) == false || audioComp.previewChannel == nullptr))
		{
			
			audioComp.previewChannel = audioManager->PlayEditorPreview(audioComp.soundGUID, audioComp.is3D);
  			
			

		}
		else if(audioComp.playPreview == false && audioManager->IsChannelPlaying(audioComp.previewChannel) == true)
		{
			/*if (audioManager->IsPreviewChannelPlaying(entity))
			{
				audioManager->StopEditorPreview(entity);
				
			}*/

			audioManager->StopEditorPreview(audioComp.previewChannel);
			
		}
		
	}

}