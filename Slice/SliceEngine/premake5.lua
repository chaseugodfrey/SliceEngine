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
        --"thirdparty/xprop",
        "thirdparty/Jolt",
        "thirdparty/fmod/include",
        ThirdParty.RTTR_INC
    }

    libdirs {
        "thirdparty/glew",
        "thirdparty/glfw/lib-vc2022",
        "thirdparty/fmod/lib",
        ThirdParty.RTTR_LIB
        }

    links {
        "glew32",
        "opengl32",
        "glfw3",
        "fmod_vc",
        "rttr_core"
        }

    defines
    {
    --    "RTTR_DLL"
    }

    pchheader "pch.h"
    pchsource "src/pch.cpp"


    filter "configurations:EditorDebug"
        --defines {"DEBUG_MODE" }
        staticruntime "off"
        symbols "On"
        
        -- links {"rttr_core_d"}

        -- includedirs
        -- {
        --     ThirdParty.RTTR_INC
        -- }
    
    filter "configurations:EditorRelease"
        --defines { "RELEASE_MODE " }
        staticruntime "off"
        optimize "On"
        
        -- links {"rttr_core"}

        -- includedirs
        -- {
        --     ThirdParty.RTTR_INC
        -- }

        
    filter{}
    -- filter "configurations:Release"
    -- postbuildcommands {
    --     '{COPY} SliceEngine.lib ../ReleaseLibs/SliceEngine.lib'
    -- }

print("engine")