@echo off
setlocal enabledelayedexpansion

:: -----------------------------------------------------------------------------
:: itch.io Deployment Script (Butler)
:: -----------------------------------------------------------------------------

set "INSTALLER_PATH=.\INSTALLER\WeightOfTheSky_Setup.exe"
set "ITCH_TARGET=abusivebanana/weight-of-the-sky:windows"

echo "--- STARTING ITCH.IO DEPLOYMENT ---"

if not exist "!INSTALLER_PATH!" (
    echo "ERROR: Installer not found at !INSTALLER_PATH!"
    exit /b 1
)

:: Check if Butler is installed
where butler >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo "Butler CLI not found in PATH."
    echo "Attempting to run from common Jenkins location..."
    :: Some labs put butler in a specific tools folder
    set "BUTLER_EXE=C:\Tools\butler\butler.exe"
) else (
    set "BUTLER_EXE=butler"
)

:: Note: ITCH_IO_TOKEN must be set in the environment by Jenkins
if not defined ITCH_IO_TOKEN (
    echo "ERROR: ITCH_IO_TOKEN environment variable is not defined."
    exit /b 1
)

echo "Pushing !INSTALLER_PATH! to !ITCH_TARGET!..."
"!BUTLER_EXE!" push "!INSTALLER_PATH!" "!ITCH_TARGET!" --userversion "!FINAL_TAG!"

if %ERRORLEVEL% equ 0 (
    echo "--- ITCH.IO DEPLOYMENT SUCCESSFUL ---"
    exit /b 0
) else (
    echo "ERROR: Butler push failed with exit code %ERRORLEVEL%"
    exit /b 1
)
