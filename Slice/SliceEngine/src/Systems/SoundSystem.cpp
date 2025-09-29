#include <pch.h>
#include "SoundSystem.h"
#include "AudioManager.h"


namespace SliceEngine
{
	void SoundSystem::BindToAudioSource()
	{
		mRegistry->on_update<AudioSource>().connect<&SoundSystem::ComponentUpdate>(this);
		//mRegistry->on_update<AudioSource>().connect<&SoundSystem::onPauseUpdated>(this);
	}

	void SoundSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);
		
		if (audioComp.soundName == "")
		{
			audioComp.soundName = "3DAudioTest";
		}
		audioComp.isPaused = false;
		audioComp.is3D = true;
		audioComp.currentVolume = 0.3f;

		
		audioManager->PlaySound(audioComp.soundName, SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, audioComp.is3D, audioComp.isPaused, audioComp.isLoop, audioComp.currentVolume, entity, transform.position);

		

		std::cout << "Entity entering sound system" << std::endl;
	}

	void SoundSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();

		audioManager->StopSound(entity);

		std::cout << "Entity exiting sound system" << std::endl;
	}

	void SoundSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);

		audioManager->SetSound3DPosition(entity, transform.position);
		
	}

	void SoundSystem::ComponentUpdate(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);
		auto& transform = reg.get<Transform>(entity);

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
	}

	/*void SoundSystem::onPauseUpdated(entt::registry& reg, entt::entity entity)
	{
		auto audioManager = Core::GetInstance()->GetAudioManager();
		auto& audioComp = reg.get<AudioSource>(entity);

		audioManager->UpdatePauseSound(entity, audioComp.isPaused);
	}*/

}