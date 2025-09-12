#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include "ECS/BaseSystem.h"
#include "AudioManager.h"
#include <iostream>
#include <string>
// for testing purpose


namespace SliceEngine
{

	struct SoundFile
	{
		std::string filePath;
		float currentVolume;
		bool isLoop;
		bool is3D;
	};

	// for keeping track of entities that belong to sound system
	struct SoundEntity {};

	struct SoundSystem : BaseSystem<SoundEntity, SoundFile>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override
		{
			//auto& soundFile = reg.get<SoundFile>(entity);

			//auto& audio = SliceEngine::AudioManager::Get();
			//audio.LoadSound(soundFile.filePath, soundFile.filePath, soundFile.is3D, soundFile.isLoop);
			//audio.PlaySound(soundFile.filePath, SliceEngine::SoundCategory::SFX, /*internal*/{}, soundFile.isLoop, soundFile.currentVolume);
			std::cout << "Entity entering sound system" << std::endl;

		}

		void EntityOnExit(entt::registry& reg, entt::entity entity) override
		{
			std::cout << "Entity exiting sound system" << std::endl;
		}

		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override
		{
			//auto& soundFile = reg.get<SoundFile>(entity);

			//std::cout << "Update Entity " << soundFile << std::endl;
		}
	};
}

#endif