project "SliceEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("%{wks.location}/build/bin-int/%{cfg.buildcfg}/%{prj.name}")

    files { "src/main.cpp" }

    includedirs {
        "SliceEditor/src",
    }

    links { "SliceEngine" }
 
    prebuildcommands {
        '{COPYFILE} ' .. engine_lib_path .. ' %{cfg.targetdir}'
    }