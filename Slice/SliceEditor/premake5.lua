project "SliceEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("%{wks.location}/build/bin-int/%{cfg.buildcfg}/%{prj.name}")

    files { "src/**" }

    includedirs {
        "src",
        IncludeDir.EnginePublic,
        ThirdParty.GLEW_INC,
        ThirdParty.GLFW_INC,
        ThirdParty.FMOD_INC,
        ThirdParty.RTTR_INC,
        "thirdparty/imgui/include"
    }

    externalincludedirs {

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
        "fmod_vc"
     }

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    -- Disable PCH for external files
    filter "files:thirdparty/**"
        flags { "NoPCH" }

    filter "configurations:EditorDebug"
        -- Link the debug versions of our libraries
        links { "rttr_core_d" }
        defines { "DEBUG_MODE" }
        symbols "On"

    filter "configurations:EditorRelease"
        -- Link the release versions of our libraries
        links { "rttr_core" }
        defines { "RELEASE_MODE" }
        optimize "On"
    -- Reset filter
    filter {}

 
    prebuildcommands {
        '{COPYFILE}  "%{engine_lib_path}" "%{cfg.targetdir}"'
    }

    postbuildcommands {
        '{COPYFILE} "%{ThirdParty.GLEW_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.GLFW_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.FMOD_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.RTTR_DLL}" "%{cfg.targetdir}"'
    }

print("editor")
