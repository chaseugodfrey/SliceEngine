@echo off
setlocal enabledelayedexpansion

:: -----------------------------------------------------------------------------
:: 1. ENVIRONMENT SETUP
:: -----------------------------------------------------------------------------
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set "VS_PATH=%%i"
)

if not exist "!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe" (
    echo ERROR: MSBuild 2022 not found.
    exit /b 1
)

set "MSBUILD_EXE="!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe""

:: -----------------------------------------------------------------------------
:: 2. BUILD USER-FACING VERSION (WeightOfTheSky Release)
:: -----------------------------------------------------------------------------
echo --- Building User-Facing (Release) Solution ---

:: Find the premake executable (it is in the Slice directory)
set "PREMAKE_EXE=%CD%\Slice\premake\premake5.exe"
if not exist "%PREMAKE_EXE%" (
    echo ERROR: Premake not found at %PREMAKE_EXE%
    exit /b 1
)

pushd WeightOfTheSky
if %ERRORLEVEL% neq 0 (echo ERROR: Could not enter WeightOfTheSky directory! & exit /b 1)

echo Running Premake...
"%PREMAKE_EXE%" vs2022
if %ERRORLEVEL% neq 0 (echo ERROR: Premake generation failed! & popd & exit /b %ERRORLEVEL%)

echo Building WeightOfTheSky App (Release)...
%MSBUILD_EXE% WeightOfTheSky.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m /v:m
if %ERRORLEVEL% neq 0 (echo ERROR: MSBuild failed! & popd & exit /b %ERRORLEVEL%)
popd

:: -----------------------------------------------------------------------------
:: 3. PACKAGE INSTALLER
:: -----------------------------------------------------------------------------
echo --- Packaging Installer ---
pushd WeightOfTheSkyInstaller
call build_installer.bat
if %ERRORLEVEL% neq 0 (echo ERROR: Installer packaging failed! & exit /b %ERRORLEVEL%)
popd

:: -----------------------------------------------------------------------------
:: 4. DEPLOY / UPLOAD
:: -----------------------------------------------------------------------------
echo --- Deploying Artifacts ---
:: Create virtual environment for Python if it doesn't exist
if not exist ".venv" (
    echo Creating Python virtual environment...
    python -m venv .venv
)

:: Install/Update dependencies
echo Installing Python dependencies...
.venv\Scripts\python -m pip install -r WeightOfTheSkyInstaller\requirements.txt

:: Upload to GitHub (Default tag v0.0.1, Jenkins should override this as needed)
echo Uploading to GitHub...
.venv\Scripts\python WeightOfTheSkyInstaller\upload_installer.py github v0.0.1

:: (Optional) Upload to Dropbox if token is present
if defined DROPBOX_ACCESS_TOKEN (
    echo Uploading to Dropbox...
    .venv\Scripts\python WeightOfTheSkyInstaller\upload_installer.py dropbox
)

echo --- CD PROCESS COMPLETE ---
exit /b 0
