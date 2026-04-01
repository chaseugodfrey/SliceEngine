@echo off
setlocal enabledelayedexpansion

echo "--- STARTING INSTALLER BUILD PROCESS ---"

:: 1. SET PATHS
set "GAME_BUILD_DIR=..\WeightOfTheSky\build\bin\Release"
set "STAGING_DIR=.\GAMEDIRECTORY"
set "OUTPUT_DIR=.\INSTALLER"
set "ISCC_EXE=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"

:: 2. PREPARE STAGING AREA
echo "Cleaning staging directory: !STAGING_DIR!"
if exist "!STAGING_DIR!" (
    del /q /s "!STAGING_DIR!\*" > nul 2>&1
    for /d %%p in ("!STAGING_DIR!\*") do rd /s /q "%%p"
)
if not exist "!STAGING_DIR!" (
    mkdir "!STAGING_DIR!"
)

echo "Copying game files from !GAME_BUILD_DIR!..."
xcopy /S /E /I /Y "!GAME_BUILD_DIR!\*" "!STAGING_DIR!\"
if !ERRORLEVEL! neq 0 (echo "ERROR: Failed to stage game files!" & exit /b 1)

:: 3. COMPILE INSTALLER
if not exist "!ISCC_EXE!" (
    echo "ERROR: Inno Setup Compiler (ISCC.exe) not found at: !ISCC_EXE!"
    exit /b 1
)

echo "Compiling installer with Inno Setup..."
"!ISCC_EXE!" "InstallScript.iss"
if !ERRORLEVEL! neq 0 (echo "ERROR: Inno Setup compilation failed!" & exit /b 1)

echo "--- INSTALLER BUILD COMPLETE ---"
exit /b 0
