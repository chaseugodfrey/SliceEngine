#ifndef SCRIPT_EDITOR_H
#define SCRIPT_EDITOR_H


namespace SliceEditor
{

	inline std::unique_ptr<filewatch::FileWatch<std::string>> AssemblyFileWatcher;

	void InitFileWatcher();

	void OnAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type);

	void RebuildSliceScriptDLL();
}

#endif
