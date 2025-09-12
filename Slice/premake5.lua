workspace "Slice"
    configurations { "EditorDebug", "EditorRelease" }
    architecture "x64"
    startproject "SliceEditor"
    flags { "MultiProcessorCompile" }
    warnings "Extra"

engine_lib_path = "%{wks.location}/SliceEngine/SliceEngine.lib"

include "SliceEngine/premake5.lua"
include "SliceEditor/premake5.lua"