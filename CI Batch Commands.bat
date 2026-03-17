::---------------------------------------------------------------------------------------------------------------------------------------------

:: build project in debug and release configurations plus run the game.exe + run launch stability check and asset validation
cd Slice
call PremakeProj.bat
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" Slice.sln /p:Configuration=EditorDebug /p:Platform=x64 /t:Rebuild /m
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" Slice.sln /p:Configuration=EditorRelease /p:Platform=x64 /t:Rebuild /m

::---------------------------------------------------------------------------------------------------------------------------------------------

:: Run the launch stability check on the debug build of the editor
@echo off
set "EDITOR_DIR=Slice\build\bin\EditorDebug\SliceEditor"
set "EDITOR_EXE=SliceEditor.exe"

echo Starting Launch Stability Check...
pushd %EDITOR_DIR%

:: launch "sliceEngine"
start "" "%EDITOR_EXE%"

:: ping localhost 11 times to simulate 10 sec delay, then nul to not display output
echo Waiting 10 seconds for stability...
ping -n 11 127.0.0.1 > nul

:: 0 means editor is still running and not crashing and 1 is vice versa
:: kill the editor task with taskkill
:: return only processes matching the name
:: pipe into SliceEditor.exe (case-insensitive) and if found, return 0, else return 1
tasklist /FI "IMAGENAME eq %EDITOR_EXE%" | find /I "%EDITOR_EXE%" > nul
if %ERRORLEVEL% equ 0 (
     echo SUCCESS: %EDITOR_EXE% is stable. Closing now.
     taskkill /f /im %EDITOR_EXE%
     popd
     exit /b 0
) else (
     echo ERROR: %EDITOR_EXE% crashed or failed to start!
     popd
     exit /b 1
)

::---------------------------------------------------------------------------------------------------------------------------------------------

:: Run the asset validation script after building the editor
@echo off
:: allow variables within loops to be updated and accessed correctly dynamically
setlocal enabledelayedexpansion
set "ASSET_DIR=Slice\SliceEditor\Assets"
set "VALID_STATUS=0"

:: print out the current step
echo Running Asset Validation...

:: 1. Check for Missing Meta Files
echo Checking for missing meta files...
:: for every file within asset dir (loop recursively)
:: check if the file extension is not in the ignore list, then check if the corresponding .meta file exists
for /r "%ASSET_DIR%" %%f in (*) do (
    set "filename=%%~nxf"
    set "ext=%%~xf"

    :: Define extensions to ignore (add more with spaces if needed)
    set "skip=0"
    if /i "!ext!"==".meta"     set "skip=1"
    if /i "!ext!"==".txt"      set "skip=1"
    if /i "!ext!"==".resource" set "skip=1"
    if /i "!ext!"==".ini"      set "skip=1"
    if /i "!ext!"==".temp"     set "skip=1"

    :: if the file is not in the ignore list, check for the corresponding .meta file
    if !skip! equ 0 (
        if not exist "%%f.meta" (
            echo ERROR: Missing meta file for asset: "%%f"
            set "VALID_STATUS=1"
        )
    )
)

:: 2. Check for Orphaned Meta Files
echo Checking for orphaned meta files...
for /r "%ASSET_DIR%" %%f in (*.meta) do (
    set "metafile=%%f"
    :: strip the .meta suffix to find the expected asset name
    set "assetfile=!metafile:~0,-5!"

    :: check if the corresponding asset file exists
    if not exist "!assetfile!" (
        echo ERROR: Orphaned meta file found: "%%~nxf"
        echo Expected Asset: "!assetfile!"
        set "VALID_STATUS=1"
    )
)

:: final validation result
if %VALID_STATUS% equ 0 (
    echo SUCCESS: Asset validation passed!
    exit /b 0
) else (
    echo FAILURE: Asset validation found errors.
    exit /b 1
)

::---------------------------------------------------------------------------------------------------------------------------------------------