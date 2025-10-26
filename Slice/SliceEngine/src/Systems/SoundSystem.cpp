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
		auto& audioComp = reg.get<AudioSource>(entity);
		
		

		
		//audioManager->PlaySound(audioComp.soundName, SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, audioComp.is3D, audioComp.isPaused, audioComp.isLoop, audioComp.currentVolume, entity, transform.position);

		

		std::cout << "Entity entering sound system" << std::endl;
	}

	void SoundSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();

		if (audioManager->IsChannelPlaying(entity))
		{
			//audioManager->StopSound(entity);

		}
		else if (audioManager->IsPreviewChannelPlaying(entity))
		{
			//audioManager->StopEditorPreview(entity);
		}

		std::cout << "Entity exiting sound system" << std::endl;
	}

	void SoundSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& transform = reg.get<Transform>(entity);

		audioManager->SetSound3DPosition(entity, transform.position);
		
	}

	void SoundSystem::ComponentUpdate(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);
		
		/*if (!audioManager->IsChannelNull(entity))
		{

			if (audioComp.currentVolume != audioManager->GetCurrentTrackVolume(entity))
			{
				
				audioManager->UpdateSoundVolume(entity, audioComp.currentVolume);
			
			}

			if (audioComp.isPaused != audioManager->GetPauseState(entity))
			{
				audioManager->UpdatePauseSound(entity, audioComp.isPaused);
			}

			if (audioComp.is3D != audioManager->IsFMOD3D(entity))
			{
				audioManager->UpdateFMODMode(entity, audioComp.is3D);
				if (audioComp.is3D == false)
				{
					audioManager->SetSound3DPosition(entity, glm::vec3{ 0.f, 0.f, 0.f });
				}
				else if (audioComp.is3D && audioManager->GetSound3DPosition(entity) == glm::vec3{ 0.f,0.f,0.f })
				{
					audioManager->SetSound3DPosition(entity, transform.position);
				}
			}

		}*/

		bool playPreviewFlag = false;

		audioComp.previewChannel->isPlaying(&playPreviewFlag);

		if (audioComp.playPreview && audioComp.previewChannel == nullptr)
		{
			
			audioComp.previewChannel = audioManager->PlayEditorPreview(audioComp.soundGUID, audioComp.is3D);
  			
			

		}
		else if(audioComp.playPreview == false && playPreviewFlag == true)
		{
			/*if (audioManager->IsPreviewChannelPlaying(entity))
			{
				audioManager->StopEditorPreview(entity);
				
			}*/

			audioManager->StopEditorPreview(audioComp.previewChannel);
			
		}
		
	}

}