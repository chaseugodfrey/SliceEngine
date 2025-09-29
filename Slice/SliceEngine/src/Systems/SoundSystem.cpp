#include <pch.h>
#include "SoundSystem.h"
#include "AudioManager.h"


namespace SliceEngine
{
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
		std::cout << "Entity exiting sound system" << std::endl;
	}

	void SoundSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{

	}

}