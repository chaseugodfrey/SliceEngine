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
        "thirdparty/JoltPhysics",
        "thirdparty/fmod/include",
        "thirdparty/nlohmann/include",
        ThirdParty.RTTR_INC
    }

    libdirs {
        "thirdparty/glew",
        "thirdparty/glfw/lib-vc2022",
        "thirdparty/fmod/lib",
        ThirdParty.RTTR_LIB,
        ThirdParty.JOLT_LIB
        }

    links {
        "glew32",
        "opengl32",
        "glfw3",
        "fmod_vc",
        --"rttr_core"
        }

    defines
    {
        "RTTR_DLL",
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS"
    }

    pchheader "pch.h"
    pchsource "src/pch.cpp"


    filter "configurations:EditorDebug"
        --defines {"DEBUG_MODE" }
       -- staticruntime "off" -- Comment this back in to get release to work but debug will break
        symbols "On"

        defines
        {
         "JPH_ENABLE_ASSERTS"
        }
        
         links {
            "rttr_core_d",
            "Jolt_d"
            }

         --defines { "JPH_ENABLE_ASSERTS" }

        -- includedirs
        -- {
        --     ThirdParty.RTTR_INC
        -- }
        -- Reset filter
    filter {}

    filter "configurations:EditorRelease"
        --defines { "RELEASE_MODE " }
       -- staticruntime "on"
        optimize "On"
        
         links {
            "rttr_core",
            "Jolt_r"
            }

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