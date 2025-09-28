project "SliceScript"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"
    targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("%{wks.location}/build/bin-int/%{cfg.buildcfg}/%{prj.name}")

    files
    {
        "Source/**.cs",
        "Properties/**.cs"
    }
    
    --postbuildcommands -- copies dll files to Editor's bin (the exe)
    --{
    --   "{COPYDIR} %[bin/" .. outputdir .. "/%{prj.name}/**.dll] %[CarmicahScriptCore/]"
    --}


    filter "configurations:Debug"
        optimize "Off"
        symbols "Default"
    
    filter "configurations:Release"
        optimize "Off"
        symbols "Default"
    
    filter "configurations:Dist"
        optimize "Full"
        symbols "Off"
   