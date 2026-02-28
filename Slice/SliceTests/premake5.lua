project "SliceTests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}")

    -- This is the key change: we are now compiling the engine's dependencies directly.
    files
    {
        "**.cpp",
        "**.h",
        "../SliceEngine/thirdparty/googletest/googletest/src/gtest-all.cc",
        "../SliceEngine/thirdparty/googletest/googlemock/src/gmock-all.cc",
        "../SliceEngine/thirdparty/recast/Recast/Source/**.cpp",
        "../SliceEngine/thirdparty/recast/Detour/Source/**.cpp",
        "../SliceEngine/thirdparty/recast/Detour/DetourCrowd/Source/**.cpp",
        "../SliceEngine/thirdparty/recast/Detour/DetourTileCache/Source/**.cpp"
    }

    includedirs
    {
        "../SliceEngine/src",
        "../SliceEngine/thirdparty/entt",
        "../SliceEngine/thirdparty/glew",
        "../SliceEngine/thirdparty/glfw/include",
        "../SliceEngine/thirdparty/glm",
        "../SliceEngine/thirdparty/googletest/googletest/include",
        "../SliceEngine/thirdparty/nlohmann/include",
        "../SliceEngine/thirdparty/JoltPhysics",
        "../SliceEngine/thirdparty/fmod/include",
        "../SliceEngine/thirdparty/rttr/include",
        "../SliceEngine/thirdparty/recast/Recast/include",
        "../SliceEngine/thirdparty/recast/Detour/Detour/include",
        "../SliceEngine/thirdparty/recast/Detour/DetourCrowd/include",
        "../SliceEngine/thirdparty/recast/Detour/DetourTileCache/include",
        "../SliceEngine/thirdparty/Mono/include"
    }

    libdirs
    {
        "../SliceEngine/thirdparty/glfw/lib-vc2022",
        "../SliceEngine/thirdparty/glew",
        "../SliceEngine/thirdparty/fmod/lib",
        "../SliceEngine/thirdparty/rttr/lib",
        "../SliceEngine/thirdparty/JoltPhysics/lib/Debug",
        "../SliceEngine/thirdparty/JoltPhysics/lib/Release",
        "../SliceEngine/thirdparty/Mono/lib"
    }

    defines { 
        "_CRT_SECURE_NO_WARNINGS",
        "GLEW_STATIC",
        "RTTR_DLL"
    }

    links
    {
        "SliceEngine", -- Still link the main engine library for non-recast code
        "opengl32",
        "glfw3",
        "glew32",
        "fmod_vc",
        "mono-2.0-sgen.lib",
        "MonoPosixHelper.lib"
    }

    filter { "files:../SliceEngine/thirdparty/**.cpp" }
        flags { "NoPCH" }

    filter "configurations:EditorDebug"
        defines { "DEBUG_MODE" }
        symbols "On"
        links { "rttr_core_d", "Jolt_d.lib" }

    filter "configurations:EditorRelease"
        defines { "RELEASE_MODE" }
        optimize "On"
        links { "rttr_core", "Jolt_r.lib" }

    filter "system:windows"
        systemversion "latest"
        defines { "GTEST_HAS_PTHREAD=0", "_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING" }

    filter {}