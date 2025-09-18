workspace "Slice"
    configurations { "EditorDebug", "EditorRelease" }
    architecture "x64"
    startproject "SliceEditor"
    flags { "MultiProcessorCompile" }
    warnings "Extra"

engine_lib_path = "%{wks.location}/SliceEngine/SliceEngine.lib"
assets_build_path = "%{wks.location}/SliceEditor/Assets"
assets_dev_path = "%{wks.location}/SliceEditor/DevAssets"

IncludeDir = {}
IncludeDir["EnginePublic"]   = "%{wks.location}/SliceEngine/src"
IncludeDir["EnTT"]              = "%{wks.location}/SliceEngine/thirdparty/entt"

ThirdParty = {}
ThirdParty.GLEW_INC = "%{wks.location}/SliceEngine/thirdparty/glew/GL"
ThirdParty.GLEW_LIB = "%{wks.location}/SliceEngine/thirdparty/glew"
ThirdParty.GLFW_INC = "%{wks.location}/SliceEngine/thirdparty/glfw/include"
ThirdParty.GLFW_LIB = "%{wks.location}/SliceEngine/thirdparty/glfw/lib-vc2022"
ThirdParty.GLM_INC = "%{wks.location}/SliceEngine/thirdparty/glm"
ThirdParty.FMOD_INC = "%{wks.location}/SliceEngine/thirdparty/fmod/include"
ThirdParty.FMOD_LIB = "%{wks.location}/SliceEngine/thirdparty/fmod/lib"
ThirdParty.RTTR_INC = "%{wks.location}/SliceEngine/thirdparty/rttr/include"
ThirdParty.RTTR_LIB = "%{wks.location}/SliceEngine/thirdparty/rttr/lib"
ThirdParty.GLEW_DLL =  "%{wks.location}/SliceEngine/thirdparty/glew/glew32.dll"
ThirdParty.GLFW_DLL = "%{wks.location}/SliceEngine/thirdparty/glfw/lib-vc2022/glfw3.dll"
ThirdParty.FMOD_DLL = "%{wks.location}/SliceEngine/thirdparty/fmod/lib/fmod.dll"
ThirdParty.JSON_INC = "%{wks.location}/SliceEngine/thirdparty/nlohmann/include"

ThirdParty.RTTR_DLL = "%{wks.location}/SliceEngine/thirdparty/rttr/bin/rttr_core.dll"
ThirdParty.RTTR_DLL_DEBUG = "%{wks.location}/SliceEngine/thirdparty/rttr/bin/rttr_core_d.dll"
include "SliceEngine/premake5.lua"
include "SliceEditor/premake5.lua"