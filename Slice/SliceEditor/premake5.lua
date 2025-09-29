project "SliceEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("%{wks.location}/build/bin-int/%{cfg.buildcfg}/%{prj.name}")

    files { "src/**", "thirdparty/imgui/include/**" }

    --rtti "On"

    includedirs {
        "src",
        IncludeDir.EnginePublic,
        ThirdParty.GLEW_INC,
        ThirdParty.GLFW_INC,
        ThirdParty.FMOD_INC,
        ThirdParty.JSON_INC,
        ThirdParty.RTTR_INC,
        IncludeDir.EnTT,
        ThirdParty.GLM_INC,
        ThirdParty.JOLT_INC,
        ThirdParty.MONO_INC,
        "thirdparty/imgui/include"
        
    }

    libdirs {
        ThirdParty.GLEW_LIB,
        ThirdParty.GLFW_LIB,
        ThirdParty.FMOD_LIB,
        ThirdParty.RTTR_LIB,
        ThirdParty.JOLT_LIB,
        ThirdParty.MONO_LIB
    }

    links { 
        "SliceEngine",
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
        "JPH_ENABLE_ASSERTS"
    }

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    -- Disable PCH for external files
    filter "files:thirdparty/**"
        flags { "NoPCH" }

    filter "configurations:EditorDebug"
        --defines {"DEBUG_MODE" }
       -- staticruntime "off" -- Comment this back in to get release to work but debug will break
        symbols "On"
        
        links {
            "rttr_core_d",
            "Jolt_d"
             }
        -- includedirs
        -- {
        --     ThirdParty.RTTR_INC
        -- }
    
    filter "configurations:EditorRelease"
        --defines { "RELEASE_MODE " }
        --staticruntime "off"
        
        optimize "On"
        
         links {
            "rttr_core",
            "Jolt_r"
            }

        -- includedirs
        -- {
        --     ThirdParty.RTTR_INC
        -- }
    -- Reset filter
    filter {}

 
    prebuildcommands {
        '{COPYFILE}  "%{engine_lib_path}" "%{cfg.targetdir}"',
        '{COPYFILE}  "%{script_lib_path}" "%{cfg.targetdir}/../SliceScript"'
    }

    postbuildcommands {
        '{COPYFILE} "%{ThirdParty.GLEW_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.GLFW_DLL}" "%{cfg.targetdir}"',    
        '{COPYDIR} "%{assets_folder_path}" "%{cfg.targetdir}/Assets"',
        '{COPYFILE} "%{ThirdParty.FMOD_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.RTTR_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.RTTR_DLL_DEBUG}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.MONO_DLL}" "%{cfg.targetdir}"',
        '{COPYDIR} "%{wks.location}/SliceEditor/thirdparty/Mono/bin" "%{cfg.targetdir}/thirdparty/Mono/bin"'

    }

print("editor")
