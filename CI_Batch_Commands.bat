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
pushd Slice
call PremakeProj.bat
if %ERRORLEVEL% neq 0 (echo ERROR: Premake Slice failed! & exit /b %ERRORLEVEL%)

echo Building Slice Editor (Debug)...
%MSBUILD_EXE% Slice.sln /p:Configuration=EditorDebug /p:Platform=x64 /t:Rebuild /m /v:m
if %ERRORLEVEL% neq 0 (echo ERROR: Slice EditorDebug build failed! & exit /b %ERRORLEVEL%)

echo Building Slice Editor (Release)...
%MSBUILD_EXE% Slice.sln /p:Configuration=EditorRelease /p:Platform=x64 /t:Rebuild /m /v:m
if %ERRORLEVEL% neq 0 (echo ERROR: Slice EditorRelease build failed! & exit /b %ERRORLEVEL%)
popd

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 3. BUILD WEIGHT OF THE SKY (GAME APP)
:: -------------------------------------------------------------------------------------------------------------------------------------------

echo --- Building WeightOfTheSky Solution ---
pushd WeightOfTheSky
:: Generate project files for WeightOfTheSky
call premake\premake5.exe vs2022
if %ERRORLEVEL% neq 0 (echo ERROR: Premake WeightOfTheSky failed! & exit /b %ERRORLEVEL%)

:: Build the standalone Release version
echo Building WeightOfTheSky App (Release)...
%MSBUILD_EXE% WeightOfTheSky.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m /v:m
if %ERRORLEVEL% neq 0 (echo ERROR: WeightOfTheSky build failed! & exit /b %ERRORLEVEL%)
popd

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 4. STABILITY CHECKS (SMOKE TESTS)
:: -------------------------------------------------------------------------------------------------------------------------------------------

:: Check Slice Editor
set "EDITOR_DIR=Slice\build\bin\EditorDebug\SliceEditor"
set "EDITOR_EXE=SliceEditor.exe"
call :StabilityCheck "%EDITOR_DIR%" "%EDITOR_EXE%" "Slice Editor"
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

:: Check WeightOfTheSky App
set "GAME_DIR=WeightOfTheSky\build\bin\Release"
set "GAME_EXE=WeightOfTheSky.exe"
call :StabilityCheck "%GAME_DIR%" "%GAME_EXE%" "WeightOfTheSky App"
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 5. ASSET VALIDATION
:: -------------------------------------------------------------------------------------------------------------------------------------------

set "ASSET_DIR=Slice\SliceEditor\Assets"
set "VALID_STATUS=0"

echo --- Running Asset Validation ---

echo Checking for missing meta files...
for /r "%ASSET_DIR%" %%f in (*) do (
    set "filename=%%~nxf"
    set "ext=%%~xf"
    set "skip=0"
    if /i "!ext!"==".meta"     set "skip=1"
    if /i "!ext!"==".txt"      set "skip=1"
    if /i "!ext!"==".resource" set "skip=1"
    if /i "!ext!"==".ini"      set "skip=1"
    if /i "!ext!"==".temp"     set "skip=1"

    if !skip! equ 0 (
        if not exist "%%f.meta" (
            echo ERROR: Missing meta file for asset: "%%f"
            set "VALID_STATUS=1"
        )
    )
)

echo Checking for orphaned meta files...
for /r "%ASSET_DIR%" %%f in (*.meta) do (
    set "metafile=%%f"
    set "assetfile=!metafile:~0,-5!"
    if not exist "!assetfile!" (
        echo ERROR: Orphaned meta file found: "%%~nxf"
        echo Expected Asset: "!assetfile!"
        set "VALID_STATUS=1"
    )
)

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
set "LOG_FILE=%WORKSPACE%\%TARGET_NAME%_smoke_test.log"

echo Starting Stability Check for %TARGET_NAME%...
pushd "%TARGET_DIR%"
if not exist "%TARGET_EXE%" (
    echo ERROR: %TARGET_EXE% not found in %TARGET_DIR%!
    popd
    exit /b 1
)

:: Use a temporary batch to launch and redirect output
:: We use 'start /b' to run in background, but wrap in 'cmd /c' to allow redirection
echo Launching %TARGET_EXE% and capturing logs to %LOG_FILE%...
start /b "" cmd /c "%TARGET_EXE% > "%LOG_FILE%" 2>&1"

echo Waiting 10 seconds for stability...
ping -n 11 127.0.0.1 > nul

tasklist /FI "IMAGENAME eq %TARGET_EXE%" | find /I "%TARGET_EXE%" > nul
if %ERRORLEVEL% equ 0 (
     echo SUCCESS: %TARGET_NAME% is stable. Closing now.
     taskkill /f /im %TARGET_EXE% > nul 2>&1
     popd
     exit /b 0
) else (
     echo ERROR: %TARGET_NAME% crashed or failed to start!
     echo --- BEGIN APPLICATION LOG ---
     if exist "%LOG_FILE%" (
         type "%LOG_FILE%"
     ) else (
         echo No log file was generated. The application likely failed to even initialize its entry point.
     )
     echo --- END APPLICATION LOG ---
     popd
     exit /b 1
)
