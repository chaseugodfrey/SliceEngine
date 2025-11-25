/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Audio.h
 author:		Lee Yong Yee
 email:			l,yongyee@digipen.edu
 brief:			Loads Audio

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef AUDIO_H
#define AUDIO_H

#include <../fmod/include/fmod.hpp>

namespace SliceEngine 
{
	namespace SliceEngineTypes
	{
		class Audio
		{
		public:	

			bool LoadAudioResource(std::string const&);
			FMOD::Sound* GetSound();
			void DestroyAudio();
		private:
			FMOD::Sound* sound = nullptr;
		};
	}
}
#endif // !AUDIO_H
