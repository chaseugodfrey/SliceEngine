/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ScriptEditor.cpp

 author:      Lee Yong Yee

 email:       l.yongyee@digipen.edu

 brief:      Script Editor, handles the scripting through the editor

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include "pch.h"
#include "ScriptEditor.h"
#include "../src/Scripting/ScriptSystem.h"

namespace SliceEditor
{

	void InitFileWatcher()
	{
		/*AssemblyFileWatcher = std::make_unique<filewatch::FileWatch<std::string>>(
			"../SliceScript/Source",
			[](const std::string& path, const filewatch::Event change_type)
			{
				std::cout << "Change detected in: " << path << std::endl;

				if (change_type == filewatch::Event::modified)
				{
					SliceEditor::OnAssemblyFileSystemEvent(path, change_type);
				}
			}
		);*/
	}

	void OnAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
	{
		/*if (SliceEngine::gScriptSystem == nullptr)
		{
			return;
		}

		if (SliceEngine::gScriptSystem->AssemblyReloadPending == false && change_type == filewatch::Event::modified)
		{
			SliceEngine::gScriptSystem->AssemblyReloadPending = true;

			AssemblyFileWatcher.reset();

			SliceEngine::gScriptSystem->ReloadAssembly();
		}*/
	}


	void RebuildSliceScriptDLL()
	{
//		// --- Step 1: locate paths relative to current directory ---
//		std::filesystem::path cwd = std::filesystem::current_path();
//
//		// Go up one folder and into SliceScript
//		std::filesystem::path csprojPath = cwd / ".." / "SliceScript" / "SliceScript.csproj";
//		csprojPath = std::filesystem::canonical(csprojPath); // resolves .. to absolute path
//
//		// Path to MSBuild (adjust if you have a different VS version)
//		std::filesystem::path msbuildPath = R"(C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe)";
//
//		// --- Step 2: verify paths exist ---
//		if (!std::filesystem::exists(csprojPath))
//		{
//			SLICE_LOG_ERROR("SliceScript.csproj not found at {}", csprojPath.string());
//			std::cout << csprojPath.string() << std::endl;
//			return;
//		}
//		SLICE_LOG("Found SliceScript.csproj at {}", csprojPath.string());
//
//		if (!std::filesystem::exists(msbuildPath))
//		{
//			SLICE_LOG_ERROR("MSBuild.exe not found at {}", msbuildPath.string());
//			std::cout << msbuildPath.string() << std::endl;
//			return;
//		}
//		SLICE_LOG("Found MSBuild.exe at {}", msbuildPath.string());
//
//		// --- Step 3: build the MSBuild command ---
//		std::stringstream command;
//		command << "\"" << msbuildPath.string() << "\""
//			<< " \"" << csprojPath.string() << "\""
//			<< " /t:Rebuild"
//#ifdef DEBUG
//			<< " /p:Configuration=EditorDebug"
//#else
//			<<" /p:Configuration=EditorRelease"
//#endif // EDITORDEBUG
//
//			<< " /p:Platform=x64";
//
//		// Wrap the whole command in extra quotes for system()
//		std::string finalCommand = "\"" + command.str() + "\"";
//
//		int result = system(finalCommand.c_str());
//
//		if (result != 0)
//		{
//			SLICE_LOG_ERROR("Failed to build SliceScript.dll via MSBuild");
//			return;
//		}
//
//		SLICE_LOG("SliceScript.dll built successfully. Reloading assembly...");
//
//		// --- Step 5: reload the script assembly ---
//		SliceEngine::gScriptSystem->ReloadAssembly();
//		SLICE_LOG("Scripts reloaded successfully!");
		
	}

}