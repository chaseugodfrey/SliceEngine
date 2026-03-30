project "SliceTests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}")

    files
    {
        "**.cpp",
        "**.h"
    }

    includedirs
    {
        "../SliceEngine/src",
        ThirdParty.GLEW_INC,
        ThirdParty.GLFW_INC,
        ThirdParty.FMOD_INC,
        ThirdParty.JSON_INC,
        ThirdParty.RTTR_INC,
        IncludeDir.EnTT,
        ThirdParty.GLM_INC,
        ThirdParty.JOLT_INC,
        ThirdParty.MONO_INC,
        "../SliceEngine/thirdparty/catch2"
    }

    libdirs
    {
        ThirdParty.GLEW_LIB,
        ThirdParty.GLFW_LIB,
        ThirdParty.FMOD_LIB,
        ThirdParty.RTTR_LIB,
        ThirdParty.JOLT_LIB_D,
        ThirdParty.JOLT_LIB_R,
        ThirdParty.MONO_LIB
    }

    defines { 
        "_CRT_SECURE_NO_WARNINGS",
        "GLEW_STATIC",
        "RTTR_DLL",
        "GLFW_INCLUDE_NONE"
    }

    links
    {
        "SliceEngine",
        "glew32",
        "opengl32",
        "glfw3",
        "fmod_vc",
        "mono-2.0-sgen.lib",
        "MonoPosixHelper.lib"
    }

    postbuildcommands {
        '{COPYFILE} "%{ThirdParty.GLEW_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.GLFW_DLL}" "%{cfg.targetdir}"',    
        '{COPYFILE} "%{ThirdParty.FMOD_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.RTTR_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.RTTR_DLL_DEBUG}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.MONO_DLL}" "%{cfg.targetdir}"',
        -- run the tests immediately after build, no need to build and then run separately
        'pushd "%{cfg.targetdir}" && "SliceTests.exe" && popd'
    }

    filter "configurations:EditorDebug"
        defines { "DEBUG_MODE" }
        symbols "On"
        links { "rttr_core_d", "Jolt_d.lib" }
        linkoptions { "/IGNORE:4204", "/IGNORE:4006", "/IGNORE:4098" }
        postbuildcommands {
            "(robocopy \"" .. ThirdParty.JOLT_LIB_D .. "\" \"%{cfg.targetdir}\" Jolt.pdb) ^& IF %ERRORLEVEL% LEQ 1 exit 0"
        }

    filter "configurations:EditorRelease"
        defines { "RELEASE_MODE" }
        optimize "On"
        links { "rttr_core", "Jolt_r.lib" }
        postbuildcommands {
            "(robocopy \"" .. ThirdParty.JOLT_LIB_R .. "\" \"%{cfg.targetdir}\" Jolt.pdb) ^& IF %ERRORLEVEL% LEQ 1 exit 0"
        }

    filter "system:windows"
        systemversion "latest"

    filter {}
