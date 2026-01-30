workspace "WeightOfTheSky"
    configurations { "Release"}
    architecture "x64"
    startproject "WeightOfTheSky"
    flags { "MultiProcessorCompile" }
    warnings "Extra"

dev_wks_path = "%{wks.location}/../Slice"
engine_lib_path = "%{dev_wks_path}/SliceEngine/SliceEngine.lib"
script_dev_path = "%{dev_wks_path}/SliceScript/SliceScript.dll"
resource_asset_path = "%{dev_wks_path}/SliceEditor/Resources"
proj_settings_path = "%{dev_wks_path}/SliceEditor/ProjectSettings"
thirdparty_path = "%{dev_wks_path}/SliceEngine/thirdparty"



IncludeDir = {}
IncludeDir["EnginePublic"]   = "%{dev_wks_path}/SliceEngine/src"
IncludeDir["EnTT"]           = "%{dev_wks_path}/SliceEngine/thirdparty/entt"

ThirdParty = {}
ThirdParty.GLEW_INC = "%{dev_wks_path}/SliceEngine/thirdparty/glew"
ThirdParty.GLEW_LIB = "%{dev_wks_path}/SliceEngine/thirdparty/glew"
ThirdParty.GLEW_DLL =  "%{dev_wks_path}/SliceEngine/thirdparty/glew/glew32.dll"

ThirdParty.GLFW_INC = "%{dev_wks_path}/SliceEngine/thirdparty/glfw/include"
ThirdParty.GLFW_LIB = "%{dev_wks_path}/SliceEngine/thirdparty/glfw/lib-vc2022"
ThirdParty.GLFW_DLL = "%{dev_wks_path}/SliceEngine/thirdparty/glfw/lib-vc2022/glfw3.dll"

ThirdParty.GLM_INC = "%{dev_wks_path}/SliceEngine/thirdparty/glm"

ThirdParty.FMOD_INC = "%{dev_wks_path}/SliceEngine/thirdparty/fmod/include"
ThirdParty.FMOD_LIB = "%{dev_wks_path}/SliceEngine/thirdparty/fmod/lib"
ThirdParty.FMOD_DLL = "%{dev_wks_path}/SliceEngine/thirdparty/fmod/lib/fmod.dll"

ThirdParty.JSON_INC = "%{dev_wks_path}/SliceEngine/thirdparty/nlohmann/include"

ThirdParty.JOLT_INC = "%{dev_wks_path}/SliceEngine/thirdparty/JoltPhysics"
ThirdParty.JOLT_LIB_D = "%{dev_wks_path}/SliceEngine/thirdparty/JoltPhysics/lib/Debug"
ThirdParty.JOLT_LIB_R = "%{dev_wks_path}/SliceEngine/thirdparty/JoltPhysics/lib/Release"
ThirdParty.JOLT_PDB_D = "%{dev_wks_path}/SliceEngine/thirdparty/JoltPhysics/lib/Debug"
ThirdParty.JOLT_PDB_R = "%{dev_wks_path}/SliceEngine/thirdparty/JoltPhysics/lib/Release"

ThirdParty.RTTR_INC = "%{dev_wks_path}/SliceEngine/thirdparty/rttr/include"
ThirdParty.RTTR_LIB = "%{dev_wks_path}/SliceEngine/thirdparty/rttr/lib"
ThirdParty.RTTR_DLL = "%{dev_wks_path}/SliceEngine/thirdparty/rttr/bin/rttr_core.dll"
ThirdParty.RTTR_DLL_DEBUG = "%{dev_wks_path}/SliceEngine/thirdparty/rttr/bin/rttr_core_d.dll"

ThirdParty.MONO_INC = "%{dev_wks_path}/SliceEngine/thirdparty/Mono/include"
ThirdParty.MONO_LIB = "%{dev_wks_path}/SliceEngine/thirdparty/Mono/lib"
ThirdParty.MONO_MONO = "%{dev_wks_path}/SliceEngine/thirdparty/Mono/mono"
ThirdParty.MONO_DLL = "%{dev_wks_path}/SliceEngine/thirdparty/Mono/lib/*.dll"
ThirdParty.MONO_BIN = "%{dev_wks_path}/SliceEngine/thirdparty/Mono/bin"

ThirdParty.RECAST_INC = "%{dev_wks_path}/SliceEngine/thirdparty/recast/Recast/include"
ThirdParty.DETOUR_INC = "%{dev_wks_path}/SliceEngine/thirdparty/recast/Detour/Detour/include"
ThirdParty.DETOUR_CROWD_INC = "%{dev_wks_path}/SliceEngine/thirdparty/recast/Detour/DetourCrowd/include"
ThirdParty.DETOUR_TILE_INC = "%{dev_wks_path}/SliceEngine/thirdparty/recast/Detour/DetourTileCache/include"

include "WeightOfTheSky/src/premake5.lua"