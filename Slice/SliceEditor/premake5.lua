project "SliceEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("%{wks.location}/build/bin-int/%{cfg.buildcfg}/%{prj.name}")

    files { "src/**", "thirdparty/imgui/include/**" }

    rtti "On"

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
        "thirdparty/imgui/include"
    }

    libdirs {
        ThirdParty.GLEW_LIB,
        ThirdParty.GLFW_LIB,
        ThirdParty.FMOD_LIB,
        ThirdParty.RTTR_LIB
    }

    links { 
        "SliceEngine",
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

    -- Disable PCH for external files
    filter "files:thirdparty/**"
        flags { "NoPCH" }

    filter "configurations:EditorDebug"
        --defines {"DEBUG_MODE" }
       -- staticruntime "off" -- Comment this back in to get release to work but debug will break
        symbols "On"
        
         links {"rttr_core_d"}

        -- includedirs
        -- {
        --     ThirdParty.RTTR_INC
        -- }
    
    filter "configurations:EditorRelease"
        --defines { "RELEASE_MODE " }
        staticruntime "off"
        
        optimize "On"
        
         links {"rttr_core"}

        -- includedirs
        -- {
        --     ThirdParty.RTTR_INC
        -- }
    -- Reset filter
    filter {}

 
    prebuildcommands {
        '{COPYFILE}  "%{engine_lib_path}" "%{cfg.targetdir}"'
    }

    postbuildcommands {
        '{COPYFILE} "%{ThirdParty.GLEW_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.GLFW_DLL}" "%{cfg.targetdir}"',    
        '{COPYDIR} "%{assets_folder_path}" "%{cfg.targetdir}/Assets"',
        '{COPYFILE} "%{ThirdParty.FMOD_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.RTTR_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.RTTR_DLL_DEBUG}" "%{cfg.targetdir}"'

    }

print("editor")
