@echo off
rem Configures the workspace (static SDL2 + SDL2_mixer + Bink, Visual Studio 2022, Win32) through
rem CMakePresets.json and opens its solution. Usage: update-workspace.bat [Debug^|Release]  (default: Debug).
setlocal

set "build_type=%~1"
if "%build_type%"=="" set "build_type=Debug"
if /i "%build_type%"=="Debug" (
  set "preset=win-msvc-dev-vs2022"
) else if /i "%build_type%"=="Release" (
  set "preset=win-msvc-release-vs2022"
) else (
  echo usage: update-workspace.bat [Debug^|Release]
  exit /b 2
)

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
