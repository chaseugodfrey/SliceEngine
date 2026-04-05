@echo off
setlocal enabledelayedexpansion

:: -----------------------------------------------------------------------------
:: itch.io Deployment Script (Robust Butler Discovery)
:: -----------------------------------------------------------------------------

set "INSTALLER_PATH=INSTALLER\WeightOfTheSky_Setup.exe"
set "ITCH_TARGET=abusivebanana/weight-of-the-sky:windows"

echo "--- STARTING ITCH.IO DEPLOYMENT ---"

if not exist "!INSTALLER_PATH!" (
    echo "ERROR: Installer not found at !INSTALLER_PATH!"
    dir INSTALLER
    exit /b 1
)

:: --- ROBUST BUTLER DISCOVERY ---
set "BUTLER_EXE="

:: 1. Check if it's already in the PATH
where butler >nul 2>nul
if !ERRORLEVEL! equ 0 (
    set "BUTLER_EXE=butler"
    echo "Butler found in system PATH."
)

:: 2. Check common DigiPen/Jenkins locations if not found
if "!BUTLER_EXE!"=="" (
    for %%P in (
        "C:\Tools\butler\butler.exe"
        "C:\butler\butler.exe"
        "%USERPROFILE%\AppData\Local\itch\bin\butler.exe"
        "D:\Tools\butler\butler.exe"
    ) do (
        if exist %%P (
            set "BUTLER_EXE=%%P"
            echo "Butler found at: %%P"
        )
    )
)

if "!BUTLER_EXE!"=="" (
    echo "ERROR: Butler CLI (butler.exe) not found anywhere."
    echo "Please ensure Butler is installed on the Jenkins Agent."
    exit /b 1
)

:: Note: ITCH_IO_TOKEN must be set in the environment by Jenkins
if not defined ITCH_IO_TOKEN (
    echo "ERROR: ITCH_IO_TOKEN environment variable is not defined."
    exit /b 1
)

echo "Pushing !INSTALLER_PATH! to !ITCH_TARGET!..."
!BUTLER_EXE! push "!INSTALLER_PATH!" "!ITCH_TARGET!" --userversion "!FINAL_TAG!"

if !ERRORLEVEL! equ 0 (
    echo "--- ITCH.IO DEPLOYMENT SUCCESSFUL ---"
    exit /b 0
) else (
    echo "ERROR: Butler push failed with exit code !ERRORLEVEL!"
    exit /b 1
)
