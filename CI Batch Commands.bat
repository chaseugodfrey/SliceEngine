@echo off
setlocal enabledelayedexpansion

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 1. ENVIRONMENT SETUP
:: -------------------------------------------------------------------------------------------------------------------------------------------

:: Find MSBuild path using vswhere
:: vswhere searches computer for installed VS instances
:: find latest instlled VS -> allow any product type -> return an installatino that contains MSbuild component -> output only the installation path
:: store instalation path in VS_PATH variable, overall avoid hardcoding a single VS path
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set "VS_PATH=%%i"
)

:: check whether MSbuild exists at the path found by vswhere, if missing return exit code 1, failing build
if not exist "!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe" (
    echo ERROR: MSBuild 2022 not found. Please ensure Visual Studio 2022 is installed.
    exit /b 1
)

:: store the MSbuild.exe path in variable MSBUILD_EXE then print it to jenkins console
set "MSBUILD_EXE="!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe""
echo Using MSBuild from: %MSBUILD_EXE%

:: -------------------------------------------------------------------------------------------------------------------------------------------
:: 2. BUILD SLICE ENGINE & EDITOR
:: -------------------------------------------------------------------------------------------------------------------------------------------

echo --- Building Slice Solution ---
pushd Slice
:: run batch file that invokes premake
call PremakeProj.bat
:: check whether premakeproj.bat failed, exit code 0 means success and exit code 1 means failure
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
:: generate project files for WeightOfTheSky
call premake\premake5.exe vs2022
if %ERRORLEVEL% neq 0 (echo ERROR: Premake WeightOfTheSky failed! & exit /b %ERRORLEVEL%)

:: build the standalone Release version
echo Building WeightOfTheSky App (Release)...
:: %MSBUILD_EXE% -> runs msbuild path we got eaarlier with vswhere, slice.sln -> sln file to build, 
:: choose configuration, choose platform (64-bti target), run rebuild to clean old ouput and clean everything from scratch
:: /v:m -> set verbosity to minimal, reduce noise in jenkins log (i don't want so many warnings), only useful info
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

:: pass 3 arguments into :stabilitycheck
:: smoketest.log is a path to store stdout/stderr logs from launched executable to capture whatever it printed to console
:StabilityCheck
set "TARGET_DIR=%~1"
set "TARGET_EXE=%~2"
set "TARGET_NAME=%~3"
set "LOG_FILE=%WORKSPACE%\%TARGET_NAME%_smoke_test.log"

:: enter target directory, check if it exsits, if not exit
echo Starting Stability Check for %TARGET_NAME%...
pushd "%TARGET_DIR%"
if not exist "%TARGET_EXE%" (
    echo ERROR: %TARGET_EXE% not found in %TARGET_DIR%!
    popd
    exit /b 1
)

:: use a temporary batch to launch and redirect output
:: we use 'start /b' to run in background, but wrap in 'cmd /c' to allow redirection
:: start /b launches new process in bg but doesnt open a visible window
:: cmd /c runs command in cmd shell then exits to send standard output to log file alongside standard errors
echo Launching %TARGET_EXE% and capturing logs to %LOG_FILE%...
start /b "" cmd /c "%TARGET_EXE% > "%LOG_FILE%" 2>&1"

:: ping for stability check for 10 seconds
echo Waiting 10 seconds for stability...
ping -n 11 127.0.0.1 > nul

:: check if process is still running
:: /FI "IMAGENAME" is to filter by process name and find the target executable, it's case insensitive
:: if process found, exit code 0, if not exit code 1 for failure
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
