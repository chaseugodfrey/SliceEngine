/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ProjectSettings.h
 author:		Chase Roderigues
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
    // ProjectSettingsService.h
    struct ProjectSettings {
        std::string productName = "Untitled";
        int width = 1280, height = 720;
        bool vsync = true;
        std::vector<std::string> scenes;
        std::string startupScene;
    };

    class ProjectSettingsService {
    public:
        explicit ProjectSettingsService(std::filesystem::path path)
            : m_path(std::move(path)) {
            Load();
        }

        const ProjectSettings& Get() const { return m_settings; }
        ProjectSettings& Edit() { m_dirty = true; return m_settings; }

        bool IsDirty() const { return m_dirty; }
        void MarkClean() { m_dirty = false; m_lastChange = {}; }

        bool Load();
        bool Save(); // atomic
        void DebouncedAutosave(double dtSeconds, double delay = 0.6); // call per-frame
        bool DetectExternalChange(); // poll last_write_time


    private:
        std::filesystem::path m_path;
        ProjectSettings m_settings;
        bool m_dirty = false;
        double m_lastChange = 0.0; // seconds since start
        std::filesystem::file_time_type m_onDiskTime{};
    };

}

#endif