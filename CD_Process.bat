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
:: 1.5 BUILD ENGINE PREREQUISITES (If needed)
:: -----------------------------------------------------------------------------
set "ENGINE_LIB=Slice\SliceEngine\SliceEngine.lib"

if exist "%ENGINE_LIB%" (
    echo --- Engine Library Found, Skipping Engine Build ---
) else (
    echo --- Building Engine Prerequisites (SliceEngine.lib) ---
    pushd Slice
    call PremakeProj.bat
    if %ERRORLEVEL% neq 0 (echo ERROR: Premake Slice failed! & exit /b %ERRORLEVEL%)

    echo Building Slice Engine (EditorRelease)...
    %MSBUILD_EXE% Slice.sln /p:Configuration=EditorRelease /p:Platform=x64 /t:Build /m /v:m
    if %ERRORLEVEL% neq 0 (echo ERROR: Slice Engine build failed! & exit /b %ERRORLEVEL%)
    popd
)

:: -----------------------------------------------------------------------------
:: 2. BUILD USER-FACING VERSION (WeightOfTheSky Release)
:: -----------------------------------------------------------------------------
echo --- Building User-Facing (Release) Solution ---

set "PREMAKE_EXE=%~dp0Slice\premake\premake5.exe"
if not exist "%PREMAKE_EXE%" (
    echo ERROR: Premake not found at %PREMAKE_EXE%
    exit /b 1
)

if exist "WeightOfTheSky\" (
    pushd WeightOfTheSky
) else (
    echo ERROR: WeightOfTheSky directory not found!
    exit /b 1
)

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
if not exist ".venv" (
    echo Creating Python virtual environment...
    python -m venv .venv
)

echo Installing Python dependencies...
.venv\Scripts\python -m pip install -r WeightOfTheSkyInstaller\requirements.txt

:: Upload to GitHub
echo Uploading to GitHub...
:: Use GH_TOKEN from Jenkins environment if available
if defined GH_TOKEN (
    echo [Token Found] Uploading release...
    .venv\Scripts\python WeightOfTheSkyInstaller\upload_installer.py github v0.0.1
) else (
    echo [No Token Found] Skipping GitHub upload. Installer is ready in WeightOfTheSkyInstaller\INSTALLER\
)

echo --- CD PROCESS COMPLETE ---
exit /b 0
