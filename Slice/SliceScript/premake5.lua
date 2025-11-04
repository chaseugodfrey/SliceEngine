project "SliceScript"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"
    -- targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
    -- objdir ("%{wks.location}/build/bin-int/%{cfg.buildcfg}/%{prj.name}")
    targetdir ("")

    files
    {
        "Source/**.cs",
        "Properties/**.cs"
    }
    
    --postbuildcommands -- copies dll files to Editor's bin (the exe)
    --{
    --   "{COPYDIR} %[bin/" .. outputdir .. "/%{prj.name}/**.dll] %[CarmicahScriptCore/]"
    --}


    filter "configurations:EditorDebug"
        optimize "Off"
        symbols "Default"
    
    filter "configurations:EditorRelease"
        optimize "Off"
        symbols "Default"
       