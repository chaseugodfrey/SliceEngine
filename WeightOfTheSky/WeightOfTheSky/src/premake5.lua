project "WeightOfTheSky"
    kind "ConsoleApp"           
    language "C++"
    cppdialect "C++20"            

    targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}")
    debugdir ("%{wks.location}/build/bin/%{cfg.buildcfg}")
    objdir ("%{wks.location}/build/bin-int/%{cfg.buildcfg}")


    files {
        "**.h",
        "**.cpp",
        "pch.h",
        "Engine.h",
        "../../../Slice/SliceEngine/thirdparty/recast/Recast/Source/*.cpp",
        "../../../Slice/SliceEngine/thirdparty/recast/Detour/Detour/Source/*.cpp",
        "../../../Slice/SliceEngine/thirdparty/recast/Detour/DetourCrowd/Source/*.cpp",
        "../../../Slice/SliceEngine/thirdparty/recast/Detour/DetourTileCache/Source/*.cpp",
    }

    pchheader "pch.h"
    pchsource "pch.cpp"

    includedirs {
        ".",                         
        IncludeDir.EnginePublic,    
        IncludeDir.EnTT,
        ThirdParty.GLEW_INC,
        ThirdParty.GLFW_INC,
        ThirdParty.GLM_INC,
        ThirdParty.FMOD_INC,
        ThirdParty.JSON_INC,
        ThirdParty.JOLT_INC,
        ThirdParty.RTTR_INC,
        ThirdParty.MONO_INC,
        ThirdParty.RECAST_INC,
        ThirdParty.DETOUR_INC,
        ThirdParty.DETOUR_CROWD_INC,
        ThirdParty.DETOUR_TILE_INC,
    }

    links {
        "SliceEngine",
        "mono-2.0-sgen.lib",
        "MonoPosixHelper.lib",
        "opengl32",        
        "fmod_vc",         
        "glfw3",
        "glew32"           
    }

    libdirs {
        dev_wks_path .. "/SliceEngine",
        ThirdParty.RTTR_LIB,
        ThirdParty.GLFW_LIB,
        ThirdParty.GLEW_LIB,
        ThirdParty.FMOD_LIB,
        ThirdParty.MONO_LIB,
        ThirdParty.JOLT_LIB_D,
        ThirdParty.JOLT_LIB_R,
    }

    filter { "files:../../../Slice/SliceEngine/thirdparty/**" }
        flags { "NoPCH" }

    filter {}

    defines
    {
        "RTTR_DLL",
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS",
        "JPH_ENABLE_ASSERTS"
    }

    filter "configurations:Release"
        --defines { "DEBUG", "SLICE_DEBUG" }
        -- runtime "Release"
        -- symbols "On"
         optimize "On"
        -- Link Debug versions of libraries if they exist
        libdirs { ThirdParty.JOLT_LIB_R }
        links { 
            "Jolt_r.lib",
            "rttr_core" 
        }
        
        postbuildcommands {
                '{COPYFILE} "' .. ThirdParty.JOLT_PDB_R .. '" "%{cfg.targetdir}"',
                '{COPYFILE} "' .. ThirdParty.FMOD_DLL .. '" "%{cfg.targetdir}"',
                '{COPYFILE} "' .. ThirdParty.MONO_LIB .. '" "%{cfg.targetdir}"',
                '{COPYFILE} "' .. ThirdParty.RTTR_DLL .. '" "%{cfg.targetdir}"',
                '{COPYFILE} "' .. ThirdParty.GLEW_DLL .. '" "%{cfg.targetdir}"',
        }


    filter {} -- Reset filter

    
    prebuildcommands {
        "{MKDIR} \"%{cfg.targetdir}/Data\"",
        '{COPYFILE}  "%{engine_lib_path}" "%{cfg.targetdir}/Data"',
    }

    postbuildcommands {
        
        '{COPY} "%{script_dev_path}" "%{cfg.targetdir}/Data/"',
        
        -- Copy Resources & Settings (Required)
        '{COPY} "%{resource_asset_path}" "%{cfg.targetdir}/Resources"',
        '{COPYDIR} "%{dev_wks_path}/SliceEngine/thirdparty/Mono/lib" "%{cfg.targetdir}/Data/thirdparty/Mono/lib"',
        '{COPYDIR} "%{dev_wks_path}/SliceEngine/thirdparty/Mono/bin" "%{cfg.targetdir}/Data/thirdparty/Mono/bin"',
        '{COPY} "%{proj_settings_path}" "%{cfg.targetdir}/ProjectSettings"',
        
        -- Copy ThirdParty content (Required)
        '{COPY} "%{thirdparty_path}" "%{cfg.targetdir}/Data/thirdparty"',

    }