@echo off
setlocal enabledelayedexpansion

echo "--- 1. ENVIRONMENT SETUP ---"
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set "VS_PATH=%%i"
)

if not exist "!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe" (
    echo "ERROR: MSBuild 2022 not found."
    exit /b 1
)

set "MSBUILD_EXE="!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe""

echo "--- 1.2 DYNAMIC VERSIONING ---"
:: Default version if parsing fails
set "APP_VERSION=1.0.0"
:: Parse the first "Version X.X.X" found in version.txt
for /f "tokens=3" %%v in ('findstr /i "Version" version.txt') do (
    set "APP_VERSION=%%v"
    goto :found_version
)
:found_version
:: If running in Jenkins, append the build number for uniqueness
if defined BUILD_NUMBER (
    set "FINAL_TAG=v!APP_VERSION!-b!BUILD_NUMBER!"
) else (
    set "FINAL_TAG=v!APP_VERSION!-local"
)
echo "Final Build Tag: !FINAL_TAG!"

echo "--- 1.5 BUILD ENGINE PREREQUISITES ---"
set "ENGINE_LIB=Slice\SliceEngine\SliceEngine.lib"

if exist "%ENGINE_LIB%" (
    echo "Engine Library Found, skipping engine build..."
) else (
    echo "Engine Library NOT Found, building prerequisites..."
    pushd Slice
    call PremakeProj.bat
    if !ERRORLEVEL! neq 0 (echo "ERROR: Premake Slice failed!" & exit /b 1)
    
    echo "Building Slice Engine (EditorRelease)..."
    %MSBUILD_EXE% Slice.sln /p:Configuration=EditorRelease /p:Platform=x64 /t:Build /m /v:m
    if !ERRORLEVEL! neq 0 (echo "ERROR: Slice Engine build failed!" & exit /b 1)
    popd
)

echo "--- 2. BUILD USER-FACING VERSION ---"
set "PREMAKE_EXE=%~dp0Slice\premake\premake5.exe"
if not exist "!PREMAKE_EXE!" (
    echo "ERROR: Premake not found!"
    exit /b 1
)

pushd WeightOfTheSky
echo "Running Premake..."
"!PREMAKE_EXE!" vs2022
if !ERRORLEVEL! neq 0 (echo "ERROR: Premake generation failed!" & popd & exit /b 1)

echo "Building WeightOfTheSky App (Release)..."
%MSBUILD_EXE% WeightOfTheSky.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild /m /v:m
if !ERRORLEVEL! neq 0 (echo "ERROR: MSBuild failed!" & popd & exit /b 1)
popd

echo "--- 3. PACKAGE INSTALLER ---"
pushd WeightOfTheSkyInstaller
call build_installer.bat
if !ERRORLEVEL! neq 0 (echo "ERROR: Installer packaging failed!" & exit /b 1)
popd

echo "--- 4. DEPLOY / UPLOAD ---"
if not exist ".venv" (
    python -m venv .venv
)
.venv\Scripts\python -m pip install -r WeightOfTheSkyInstaller\requirements.txt

:: GitHub Deployment
:: Map your Jenkins secret (gh_tokensecret) to the standard GH_TOKEN
if defined gh_tokensecret set "GH_TOKEN=!gh_tokensecret!"

if defined GH_TOKEN (
    echo "[Token Found] Uploading release to GitHub as !FINAL_TAG!..."
    .venv\Scripts\python WeightOfTheSkyInstaller\upload_installer.py github !FINAL_TAG!
) else (
    echo "[No Token Found] Skipping GitHub upload. Ensure gh_tokensecret is set in Jenkins."
)


:: Bonus Rubric Placeholder: itch.io deployment could go here
if defined ITCH_IO_TOKEN (
    echo "Bonus: Deploying to itch.io..."
    :: call WeightOfTheSkyInstaller\deploy_itch.bat !FINAL_TAG!
)

echo "--- CD PROCESS COMPLETE ---"
exit /b 0
