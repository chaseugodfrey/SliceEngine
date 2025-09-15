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
    }

    -- libdirs {
    --     "thirdparty/glew",
    --     "thirdparty/glfw/lib-vc2022",
    --     "thirdparty/rttr/lib",
    --     "thirdparty/fmod/lib",
    --     "thirdparty/rttr/lib"
    -- }

    -- links {
    --     "glew32",
    --     "opengl32",
    --     "glfw3",
    --     "fmod_vc"
    -- }

    -- filter "configurations:EditorDebug"
    --     defines {"DEBUG_MODE" }
    --     symbols "On"
    --     links {"rttr_core_d"}
    
    -- filter "configurations:EditorRelease"
    --     defines { "RELEASE_MODE " }
    --     optimize "On"
    --     links {"rttr_core"}

    pchheader "pch.h"
    pchsource "src/pch.cpp"
    
    -- filter "configurations:Release"
    -- postbuildcommands {
    --     '{COPY} SliceEngine.lib ../ReleaseLibs/SliceEngine.lib'
    -- }

print("engine")