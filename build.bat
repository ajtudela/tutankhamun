@echo off
rem Builds tutankhamun through CMakePresets.json: configures and builds one preset, installs the game folder
rem (tutankhamun.exe and config\ beside it) into out\install\<preset> and prints both paths.
rem Usage: build.bat [release^|debug] [vs^|ninja]  (default: release vs). vs = Visual Studio 2022, ninja = Ninja in a
rem vcvars32 environment (found through vswhere). TUTANKHAMUN_JOBS sets the parallel build jobs.
setlocal

set "build_type=%~1"
set "generator=%~2"
if "%build_type%"=="" set "build_type=release"
if "%generator%"=="" set "generator=vs"

if /i "%build_type%"=="release" (set "preset_flow=release") else if /i "%build_type%"=="debug" (set "preset_flow=dev") else goto usage

if /i "%build_type%"=="release" (set "config=Release") else (set "config=Debug")

if /i "%generator%"=="vs" (
  set "preset=win-msvc-%preset_flow%-vs2022"
  set "binary=%~dp0out\build\win-msvc-%preset_flow%-vs2022\bin\%config%\tutankhamun.exe"
) else if /i "%generator%"=="ninja" (
  set "preset=win-msvc-%preset_flow%-ninja"
  set "binary=%~dp0out\build\win-msvc-%preset_flow%-ninja\bin\tutankhamun.exe"
) else goto usage

if /i "%generator%"=="ninja" call :vcvars32 || exit /b 1

set "parallel="
if not "%TUTANKHAMUN_JOBS%"=="" set "parallel=--parallel %TUTANKHAMUN_JOBS%"

set "deploy=%~dp0out\install\%preset%"

pushd "%~dp0"
cmake --preset %preset% || goto failed
cmake --build --preset %preset% %parallel% || goto failed
if exist "%deploy%" rmdir /s /q "%deploy%"
cmake --install "out\build\%preset%" --config %config% --prefix "%deploy%" || goto failed
popd

if not exist "%binary%" (
  echo build.bat: the build finished but %binary% is missing
  exit /b 1
)
if not exist "%deploy%\tutankhamun.exe" (
  echo build.bat: the install finished but %deploy%\tutankhamun.exe is missing
  exit /b 1
)
if not exist "%deploy%\config\" (
  echo build.bat: the install finished but %deploy%\config is missing
  exit /b 1
)
echo.
echo Built: %binary%
echo Game folder: %deploy%  (tutankhamun.exe and config\, copy both)
exit /b 0

:failed
popd
echo build.bat: preset %preset% failed
exit /b 1

:vcvars32
if "%VSCMD_ARG_TGT_ARCH%"=="x86" exit /b 0
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" (
  echo build.bat: vswhere.exe not found; install Visual Studio 2022 with "Desktop development with C++"
  exit /b 1
)
set "vs_path="
for /f "usebackq delims=" %%i in (`"%vswhere%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "vs_path=%%i"
if "%vs_path%"=="" (
  echo build.bat: no Visual Studio with the C++ tools found
  exit /b 1
)
call "%vs_path%\VC\Auxiliary\Build\vcvars32.bat" >nul || exit /b 1
exit /b 0

:usage
echo usage: build.bat [release^|debug] [vs^|ninja]
echo   release   the game (static SDL2 + mixer + Bink), Release (default)
echo   debug     the same in Debug
exit /b 2
