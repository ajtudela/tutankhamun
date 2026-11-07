@echo off
rem Configures the workspace (static SDL2 + SDL2_mixer + Bink, Visual Studio 2022, Win32 or x64) through
rem CMakePresets.json and opens its solution. Usage: update-workspace.bat [Debug^|Release] [x86^|x64]
rem (default: Debug x86).
setlocal

set "build_type=%~1"
set "arch=%~2"
if "%build_type%"=="" set "build_type=Debug"
if "%arch%"=="" set "arch=x86"
if /i "%arch%"=="x86" (set "arch_name=") else if /i "%arch%"=="x64" (set "arch_name=-x64") else goto usage
if /i "%build_type%"=="Debug" (
  set "preset=win-msvc-dev%arch_name%-vs2022"
) else if /i "%build_type%"=="Release" (
  set "preset=win-msvc-release%arch_name%-vs2022"
) else goto usage

pushd "%~dp0"
cmake --preset %preset%
if errorlevel 1 (
  popd
  echo update-workspace.bat: configure of %preset% failed
  exit /b 1
)
set "workspace=%~dp0out\build\%preset%"
if exist "%workspace%\tutankhamun.slnx" (
  start "" "%workspace%\tutankhamun.slnx"
) else (
  start "" "%workspace%\tutankhamun.sln"
)
popd
exit /b 0

:usage
echo usage: update-workspace.bat [Debug^|Release] [x86^|x64]
exit /b 2
