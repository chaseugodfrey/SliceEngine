/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ProjectSettings.h
 author:		Chase Rodrigues
 email:			roderigues.i@digipen.edu
 brief:			Handles project settings

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

namespace SliceEngine
{
	
	struct ProjectSettings
	{
		bool isDirty;
		std::string filepath;
		
		ProjectSettings(std::string name) : isDirty(false), filepath("ProjectSettings/" + name + ".asset") {};
		virtual ~ProjectSettings() = default;

		void LoadFromFile();
		virtual void Init() = 0;
		virtual void LoadSettings(nlohmann::json) = 0;
		virtual void SaveSettings() = 0;
		virtual void Exit() = 0;

		// per-frame update for dirty checking
		// only call in editor mode
		void CheckDirty();

		// override only for settings that needs to change systems
		// e.g. audio, physics 
		virtual void ApplySettings() {};
	};
}

#endif