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
        ThirdParty.RECAST_INC,
        ThirdParty.DETOUR_INC,
        "thirdparty/imgui/include",
        "thirdparty/filewatch/FileWatch"
        
    }

    libdirs {
        ThirdParty.GLEW_LIB,
        ThirdParty.GLFW_LIB,
        ThirdParty.FMOD_LIB,
        ThirdParty.RTTR_LIB,
        ThirdParty.JOLT_LIB_D,
        ThirdParty.JOLT_LIB_R,
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
            "Jolt_d.lib"
             }
        
        linkoptions { "/IGNORE:4204", "/IGNORE:4006", "/IGNORE:4098" }
        -- includedirs
        -- {
        --     ThirdParty.RTTR_INC
        -- }
         postbuildcommands {
                '{COPYFILE} "' .. ThirdParty.JOLT_PDB_D .. '" "%{cfg.targetdir}"'
            }
    
    filter "configurations:EditorRelease"
        --defines { "RELEASE_MODE " }
        --staticruntime "off"
        
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
    -- Reset filter
    filter {}

 
    prebuildcommands {
        '{COPYFILE}  "%{engine_lib_path}" "%{cfg.targetdir}"',
        '{COPYDIR}  "%{script_dev_path}" "%{cfg.targetdir}/../SliceScript"'
    }

    postbuildcommands {
        '{COPYFILE} "%{ThirdParty.GLEW_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.GLFW_DLL}" "%{cfg.targetdir}"',    
        '{COPYDIR} "%{assets_build_path}" "%{cfg.targetdir}/Assets"',
        '{COPYDIR} "%{resource_asset_path}" "%{cfg.targetdir}/Resources"',
        '{COPYFILE} "%{wks.location}/SliceEditor/projectSettings.json" "%{cfg.targetdir}"',
        '{COPYFILE} "%{wks.location}/SliceEditor/FBX_Compile.exe" "%{cfg.targetdir}"',
        '{COPYFILE} "%{wks.location}/SliceEditor/TextureCompile.exe" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.FMOD_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.RTTR_DLL}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.RTTR_DLL_DEBUG}" "%{cfg.targetdir}"',
        '{COPYFILE} "%{ThirdParty.MONO_DLL}" "%{cfg.targetdir}"',
        '{COPYDIR} "%{wks.location}/SliceEditor/thirdparty/Mono/bin" "%{cfg.targetdir}/thirdparty/Mono/bin"',
        '{COPYFILE} "%{wks.location}/SliceEditor/imgui.ini" "%{cfg.targetdir}"'

    }

print("editor")
