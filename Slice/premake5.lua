workspace "Slice"
    configurations { "EditorDebug", "EditorRelease"}
    architecture "x64"
    startproject "SliceEditor"
    flags { "MultiProcessorCompile" }
    warnings "Extra"
    buildoptions { "/bigobj" }

engine_lib_path = "%{wks.location}/SliceEngine/SliceEngine.lib"
script_dev_path = "%{wks.location}/SliceScript"
assets_build_path = "%{wks.location}/SliceEditor/Assets"
resource_asset_path = "%{wks.location}/SliceEditor/Resources"
assets_dev_path = "%{wks.location}/SliceEditor/DevAssets"

IncludeDir = {}
IncludeDir["EnginePublic"]   = "%{wks.location}/SliceEngine/src"
IncludeDir["EnTT"]              = "%{wks.location}/SliceEngine/thirdparty/entt"

ThirdParty = {}
ThirdParty.GLEW_INC = "%{wks.location}/SliceEngine/thirdparty/glew"
ThirdParty.GLEW_LIB = "%{wks.location}/SliceEngine/thirdparty/glew"
ThirdParty.GLEW_DLL =  "%{wks.location}/SliceEngine/thirdparty/glew/glew32.dll"

ThirdParty.GLFW_INC = "%{wks.location}/SliceEngine/thirdparty/glfw/include"
ThirdParty.GLFW_LIB = "%{wks.location}/SliceEngine/thirdparty/glfw/lib-vc2022"
ThirdParty.GLFW_DLL = "%{wks.location}/SliceEngine/thirdparty/glfw/lib-vc2022/glfw3.dll"

ThirdParty.GLM_INC = "%{wks.location}/SliceEngine/thirdparty/glm"

ThirdParty.FMOD_INC = "%{wks.location}/SliceEngine/thirdparty/fmod/include"
ThirdParty.FMOD_LIB = "%{wks.location}/SliceEngine/thirdparty/fmod/lib"
ThirdParty.FMOD_DLL = "%{wks.location}/SliceEngine/thirdparty/fmod/lib/fmod.dll"

ThirdParty.JSON_INC = "%{wks.location}/SliceEngine/thirdparty/nlohmann/include"

ThirdParty.JOLT_INC = "%{wks.location}/SliceEngine/thirdparty/JoltPhysics"
ThirdParty.JOLT_LIB_D = "%{wks.location}/SliceEngine/thirdparty/JoltPhysics/lib/Debug"
ThirdParty.JOLT_LIB_R = "%{wks.location}/SliceEngine/thirdparty/JoltPhysics/lib/Release"
ThirdParty.JOLT_PDB_D = "%{wks.location}/SliceEngine/thirdparty/JoltPhysics/lib/Debug"
ThirdParty.JOLT_PDB_R = "%{wks.location}/SliceEngine/thirdparty/JoltPhysics/lib/Release"

ThirdParty.RTTR_INC = "%{wks.location}/SliceEngine/thirdparty/rttr/include"
ThirdParty.RTTR_LIB = "%{wks.location}/SliceEngine/thirdparty/rttr/lib"
ThirdParty.RTTR_DLL = "%{wks.location}/SliceEngine/thirdparty/rttr/bin/rttr_core.dll"
ThirdParty.RTTR_DLL_DEBUG = "%{wks.location}/SliceEngine/thirdparty/rttr/bin/rttr_core_d.dll"

ThirdParty.MONO_INC = "%{wks.location}/SliceEngine/thirdparty/Mono/include"
ThirdParty.MONO_LIB = "%{wks.location}/SliceEngine/thirdparty/Mono/lib"
ThirdParty.MONO_MONO = "%{wks.location}/SliceEngine/thirdparty/Mono/mono"
ThirdParty.MONO_DLL = "%{wks.location}/SliceEngine/thirdparty/Mono/lib/*.dll"
ThirdParty.MONO_BIN = "%{wks.location}/SliceEngine/thirdparty/Mono/bin"


include "SliceEngine/premake5.lua"
include "SliceScript/premake5.lua"
include "SliceEditor/premake5.lua"
include "SliceTests/premake5.lua"