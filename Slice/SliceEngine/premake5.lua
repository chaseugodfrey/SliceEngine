project "SliceEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    architecture "x64"

    targetdir ("")
    -- objdir ("bin-int/%{cfg.buildcfg}")

    files { "src/**" }

    includedirs {
        "src",
        "thirdparty/entt",
        "thirdparty/glew",
        "thirdparty/glfw/include",
        "thirdparty/glm",
        "thirdparty/xprop",
        "thirdparty/rttr/include",
        "thirdparty/Jolt",
        "thirdparty/fmod/include",
        "thirdparty/nlohmann/include"
    }

    libdirs {
        "thirdparty/glew",
        "thirdparty/glfw/lib-vc2022",
        "thirdparty/rttr/lib",
        "thirdparty/fmod/lib"
    }

    links {
        "glew32",
        "opengl32",
        "glfw3",
        "rttr_core",
        "fmod_vc"
    }

    pchheader "pch.h"
    pchsource "src/pch.cpp"
    
    -- filter "configurations:Release"
    -- postbuildcommands {
    --     '{COPY} SliceEngine.lib ../ReleaseLibs/SliceEngine.lib'
    -- }

print("engine")