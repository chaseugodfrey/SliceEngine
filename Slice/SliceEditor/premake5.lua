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
        "thirdparty/imgui/include"
    }

    externalincludedirs {

    }

    libdirs {
        ThirdParty.GLEW_LIB,
        ThirdParty.GLFW_LIB,
        ThirdParty.FMOD_LIB
    }

    links { "SliceEngine" }

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    -- Disable PCH for external files
    filter "files:thirdparty/**.cpp"
    flags { "NoPCH" }

    filter "files:thirdparty/**.c"
        flags { "NoPCH" }

    -- Reset filter
    filter {}

 
    prebuildcommands {
        '{COPYFILE} ' .. engine_lib_path .. ' %{cfg.targetdir}'
    }

    postbuildcommands {
        '{COPYFILE} "%{ThirdParty.GLEW_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.GLFW_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.FMOD_DLL}" "%{cfg.targetdir}"',
    }

print("editor")
