project "SliceEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    architecture "x64"

    targetdir ("")
    -- objdir ("bin-int/%{cfg.buildcfg}")

    files { "src/**"}

    includedirs {
        "src",
        "thirdparty/entt",
        "thirdparty/glew",
        "thirdparty/glfw/include",
        "thirdparty/glm",
        "thirdparty/gli",
        --"thirdparty/xprop",
        "thirdparty/JoltPhysics",
        "thirdparty/fmod/include",
        "thirdparty/nlohmann/include",
        ThirdParty.RTTR_INC,
        ThirdParty.MONO_INC
    }

    externalincludedirs {
        "thirdparty/Mono/bin"
    }

    libdirs {
        "thirdparty/glew",
        "thirdparty/glfw/lib-vc2022",
        "thirdparty/fmod/lib",
        ThirdParty.RTTR_LIB,
        ThirdParty.JOLT_LIB_D,
        ThirdParty.JOLT_LIB_R,
        ThirdParty.MONO_LIB
        }

    links {
        "glew32",
        "opengl32",
        "glfw3",
        "fmod_vc",
        "mono-2.0-sgen.lib",
        "MonoPosixHelper.lib"
        --"rttr_core"
        }

    defines
    {
        "RTTR_DLL",
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS",
        "JPH_ENABLE_ASSERTS",
        "GLFW_INCLUDE_NONE"
    }

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    filter { "files:thirdparty/**.cpp" }
        flags { "NoPCH" }
    filter {}

    buildoptions { "/bigobj" }

    filter "configurations:EditorDebug"
        --defines {"DEBUG_MODE" }
       -- staticruntime "off" -- Comment this back in to get release to work but debug will break
       buildoptions { "/bigobj" }
        symbols "On"
        
         links {
            "rttr_core_d",
            "Jolt_d.lib"
            }
        linkoptions { "/IGNORE:4204", "/IGNORE:4006", "/IGNORE:4098" }
         postbuildcommands {
                '{COPYFILE} "' .. ThirdParty.JOLT_PDB_D .. '" "%{cfg.targetdir}"'
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
            "Jolt_r.lib"
            }
         postbuildcommands {
                '{COPYFILE} "' .. ThirdParty.JOLT_PDB_R .. '" "%{cfg.targetdir}"'
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