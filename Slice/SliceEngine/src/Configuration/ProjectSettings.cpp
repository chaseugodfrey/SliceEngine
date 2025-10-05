#include <pch.h>
#include "ProjectSettings.h"

namespace SliceEngine
{
    // ProjectSettingsService.cpp

    static nlohmann::json ToJson(const ProjectSettings& s) 
    {
        return {
            {"product", {{"name", s.productName}}},
            {"render",  {{"width", s.width}, {"height", s.height}, {"vsync", s.vsync}}},
            {"scenes", s.scenes},
            {"startupScene", s.startupScene}
        };
    }

    static void FromJson(ProjectSettings& s, const nlohmann::json& j) 
    {
        if (j.contains("product") && j["product"].contains("name"))
            s.productName = j["product"]["name"].get<std::string>();
        if (j.contains("render")) {
            s.width = j["render"].value("width", s.width);
            s.height = j["render"].value("height", s.height);
            s.vsync = j["render"].value("vsync", s.vsync);
        }
        if (j.contains("scenes")) s.scenes = j["scenes"].get<std::vector<std::string>>();
        s.startupScene = j.value("startupScene", s.startupScene);
    }

    bool ProjectSettingsService::Load() 
    {
        if (!std::filesystem::exists(m_path)) return false;
        std::ifstream f(m_path);
        nlohmann::json j; f >> j;
        FromJson(m_settings, j);
        m_onDiskTime = std::filesystem::last_write_time(m_path);
        MarkClean();
        return true;
    }

    bool ProjectSettingsService::Save() {
        // atomic write
        auto tmp = m_path; tmp += ".tmp";
        std::ofstream f(tmp, std::ios::trunc);
        if (!f) return false;
        f << ToJson(m_settings).dump(2);
        f.close();
        std::error_code ec;
        std::filesystem::rename(tmp, m_path, ec);
        if (ec) return false;
        m_onDiskTime = std::filesystem::last_write_time(m_path);
        MarkClean();
        return true;
    }

    void ProjectSettingsService::DebouncedAutosave(double dt, double delay) {
        if (!m_dirty) return;
        m_lastChange += dt;
        if (m_lastChange >= delay) Save();
    }

    bool ProjectSettingsService::DetectExternalChange() {
        if (!std::filesystem::exists(m_path)) return false;
        auto t = std::filesystem::last_write_time(m_path);
        if (t != m_onDiskTime) { m_onDiskTime = t; return true; }
        return false;
    }


}