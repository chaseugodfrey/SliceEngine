project "WeightOfTheSky"
    kind "WindowedApp"           
    language "C++"
    cppdialect "C++20"            

    targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/build")
    objdir ("%{wks.location}/build/bin-int/%{cfg.buildcfg}")

    files {
        "src/**.h",
        "src/**.cpp",
        "src/pch.h",
        "src/Engine.h"
    }

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    includedirs {
        "src",                         
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
        "thirdparty/filewatch/FileWatch"
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
        dev_wks_path .. "/build", -- Where SliceEngine.lib lives
        ThirdParty.GLFW_LIB,
        ThirdParty.GLEW_LIB,
        ThirdParty.FMOD_LIB,
        ThirdParty.MONO_LIB,
        ThirdParty.JOLT_LIB_D,
        ThirdParty.JOLT_LIB_R,
    }

    filter "configurations:RELEASE"
        --defines { "DEBUG", "SLICE_DEBUG" }
        runtime "Release"
        symbols "On"
        
        -- Link Debug versions of libraries if they exist
        libdirs { ThirdParty.JOLT_LIB_D }
        links { 
            "Jolt",
            "rttr_core" 
        } 


    filter {} -- Reset filter

    
    prebuildcommands {
        '{COPYFILE}  "%{engine_lib_path}" "%{cfg.targetdir}/Data/"',
        '{COPYFILE}  "%{script_dev_path}" "%{cfg.targetdir}/Data/"',
    }

    postbuildcommands {
        '{COPYDIR} "%{resource_asset_path}" "%{cfg.targetdir}/Resources"',
        '{COPYDIR} "%{proj_settings_path}" "%{cfg.targetdir}/ProjectSettings"',

    }