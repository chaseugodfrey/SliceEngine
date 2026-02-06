@echo off
setlocal enabledelayedexpansion

:: Find MSBuild path using vswhere
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set "VS_PATH=%%i"
)

if not exist "!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe" (
    echo Error: MSBuild 2022 not found. Please ensure Visual Studio 2022 is installed.
    pause
    exit /b 1
)

set "MSBUILD_EXE="!VS_PATH!\MSBuild\Current\Bin\MSBuild.exe""

:: 1. Run Premake for the Slice solution
echo --- Generating Slice Solution ---
pushd ..\Slice
call premake\premake5.exe vs2022
if %errorlevel% neq 0 (echo Premake Slice failed & pause & exit /b %errorlevel%)

:: 2. Clean and Build Slice (EditorRelease)
echo --- Cleaning and Building Slice (EditorRelease) ---
%MSBUILD_EXE% Slice.sln /t:Clean /p:Configuration=EditorRelease /p:Platform=x64
%MSBUILD_EXE% Slice.sln /t:Build /p:Configuration=EditorRelease /p:Platform=x64
if %errorlevel% neq 0 (echo Build Slice failed & pause & exit /b %errorlevel%)
popd

:: 3. Run Premake for WeightOfTheSky
echo --- Generating WeightOfTheSky Solution ---
call premake\premake5.exe vs2022
if %errorlevel% neq 0 (echo Premake WeightOfTheSky failed & pause & exit /b %errorlevel%)

:: 4. Clean and Build WeightOfTheSky (Release)
echo --- Cleaning and Building WeightOfTheSky (Release) ---
%MSBUILD_EXE% WeightOfTheSky.sln /t:Clean /p:Configuration=Release /p:Platform=x64
%MSBUILD_EXE% WeightOfTheSky.sln /t:Build /p:Configuration=Release /p:Platform=x64
if %errorlevel% neq 0 (echo Build WeightOfTheSky failed & pause & exit /b %errorlevel%)


echo --- Workflow Complete ---
pause