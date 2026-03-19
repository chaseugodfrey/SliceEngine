@echo off
setlocal enabledelayedexpansion

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 1. ENVIRONMENT SETUP
:: -------------------------------------------------------------------------------------------------------------------------------------------

:: Find MSBuild path using vswhere
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set "VS_PATH=%%i"
)

if not exist "!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe" (
    echo ERROR: MSBuild 2022 not found. Please ensure Visual Studio 2022 is installed.
    exit /b 1
)

set "MSBUILD_EXE="!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe""
echo Using MSBuild from: %MSBUILD_EXE%

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 2. BUILD SLICE ENGINE & EDITOR
:: -------------------------------------------------------------------------------------------------------------------------------------------

echo --- Building Slice Solution ---
:: go into slice directory, call premake.batch file, if it fails to do so, give 1 value and 0 for success, then exit
pushd Slice
call PremakeProj.bat
if %ERRORLEVEL% neq 0 (echo ERROR: Premake Slice failed! & exit /b %ERRORLEVEL%)

%MSBUILD_EXE% Slice.sln /p:Configuration=EditorDebug /p:Platform=x64 /t:Rebuild /m
if %ERRORLEVEL% neq 0 (echo ERROR: Slice EditorDebug build failed! & exit /b %ERRORLEVEL%)

%MSBUILD_EXE% Slice.sln /p:Configuration=EditorRelease /p:Platform=x64 /t:Rebuild /m
if %ERRORLEVEL% neq 0 (echo ERROR: Slice EditorRelease build failed! & exit /b %ERRORLEVEL%)
popd

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 3. BUILD WEIGHT OF THE SKY (GAME APP)
:: -------------------------------------------------------------------------------------------------------------------------------------------

echo --- Building WeightOfTheSky Solution ---
pushd WeightOfTheSky
:: generate project files for WeightOfTheSky
call premake\premake5.exe vs2022
if %ERRORLEVEL% neq 0 (echo ERROR: Premake WeightOfTheSky failed! & exit /b %ERRORLEVEL%)

:: build the standalone release version
%MSBUILD_EXE% WeightOfTheSky.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m
if %ERRORLEVEL% neq 0 (echo ERROR: WeightOfTheSky build failed! & exit /b %ERRORLEVEL%)
popd

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 4. STABILITY CHECKS (Launch validation test)
:: -------------------------------------------------------------------------------------------------------------------------------------------

:: check Editor stability
set "EDITOR_DIR=Slice\build\bin\EditorDebug\SliceEditor"
set "EDITOR_EXE=SliceEditor.exe"
call :StabilityCheck "%EDITOR_DIR%" "%EDITOR_EXE%" "Slice Editor"
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

:: check WeightOfTheSky.exe
set "GAME_DIR=WeightOfTheSky\build\bin\Release"
set "GAME_EXE=WeightOfTheSky.exe"
call :StabilityCheck "%GAME_DIR%" "%GAME_EXE%" "WeightOfTheSky App"
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 5. ASSET VALIDATION
:: -------------------------------------------------------------------------------------------------------------------------------------------

set "ASSET_DIR=Slice\SliceEditor\Assets"
set "VALID_STATUS=0"

echo Running Asset Validation...

echo Checking for missing meta files...
:: loop recursively through the dir and check every file
:: check if file extension is within the ignore list, if it is, skip then check if corresponding meta file exists
for /r "%ASSET_DIR%" %%f in (*) do (
    set "filename=%%~nxf"
    set "ext=%%~xf"
    set "skip=0"
    if /i "!ext!"==".meta"     set "skip=1"
    if /i "!ext!"==".txt"      set "skip=1"
    if /i "!ext!"==".resource" set "skip=1"
    if /i "!ext!"==".ini"      set "skip=1"
    if /i "!ext!"==".temp"     set "skip=1"

    :: if 
    if !skip! equ 0 (
        if not exist "%%f.meta" (
            echo ERROR: Missing meta file for asset: "%%f"
            set "VALID_STATUS=1"
        )
    )
)

echo Checking for orphaned meta files...
:: loop recursively through the dir and check every file
:: strip .meta suffix to find expected parent asset, if not found, flag as orphaned
for /r "%ASSET_DIR%" %%f in (*.meta) do (
    set "metafile=%%f"
    set "assetfile=!metafile:~0,-5!"
    if not exist "!assetfile!" (
        echo ERROR: Orphaned meta file found: "%%~nxf"
        echo Expected Asset: "!assetfile!"
        set "VALID_STATUS=1"
    )
)

:: final validation status, as long as it's 0 overall, checks passed
if %VALID_STATUS% equ 0 (
    echo SUCCESS: Asset validation passed!
) else (
    echo FAILURE: Asset validation found errors.
    exit /b 1
)

echo --- CI PROCESS COMPLETE: SUCCESS ---
exit /b 0

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: SUBROUTINES
:: -------------------------------------------------------------------------------------------------------------------------------------------

:StabilityCheck
set "TARGET_DIR=%~1"
set "TARGET_EXE=%~2"
set "TARGET_NAME=%~3"

echo Starting Stability Check for %TARGET_NAME%...
:: find target dir and check if it exists
pushd "%TARGET_DIR%"
if not exist "%TARGET_EXE%" (
    echo ERROR: %TARGET_EXE% not found in %TARGET_DIR%!
    popd
    exit /b 1
)

:: launch the target
start "" "%TARGET_EXE%"

:: ping localhost 11 times, lowk hardcoding but this is what i got from stackoverflow
echo Waiting 10 seconds for stability...
ping -n 11 127.0.0.1 > nul

:: 0 means editor is still running and not crashing, 1 means it failed to start or wait for 10 secs
:: /fi is to return only processes that match the name 
:: find I/ means pipe into sliceeditor (case insenstitive) and if not found, return 1
tasklist /FI "IMAGENAME eq %TARGET_EXE%" | find /I "%TARGET_EXE%" > nul
if %ERRORLEVEL% equ 0 (
     echo SUCCESS: %TARGET_NAME% is stable. Closing now.
     taskkill /f /im %TARGET_EXE%
     popd
     exit /b 0
) else (
     echo ERROR: %TARGET_NAME% crashed or failed to start!
     popd
     exit /b 1
)
