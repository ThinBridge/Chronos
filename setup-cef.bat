@echo "+=====================================================+"
@echo "| This script setups the following CEF directories.   |"
@echo "| Run this script to start developing Chronos.        |"
@echo "|                                                     |"
@echo "|  Chronos/                                           |"
@echo "|  +- cef-cache    ... Cache & build dir              |"
@echo "|  +- lib          ... Static lib (Debug)             |"
@echo "|  +- rlib         ... Static lib (Release)           |"
@echo "|  +- include      ... Header file                    |"
@echo "|  +- D32                                             |"
@echo "|  +- R32                                             |"
@echo "+=====================================================+"

set BASEDIR=%~dp0
IF NOT DEFINED CEFVER (
  echo Use the default CEF version.
  echo To build with a newer CEF version, set CEFVER explicitly.
  set CEFVER=cef_binary_135.0.17+gcbc1c5b+chromium-135.0.7049.52_windows32_minimal
)
set CEFHOST=https://cef-builds.spotifycdn.com

setlocal

if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
  call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
  call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
)

@REM -----------------
@REM Cleanup directory
@REM -----------------
cd "%BASEDIR%"
cmake -E rm -rf include lib rlib D32 R32
cmake -E make_directory include lib rlib D32 R32
cmake -E make_directory cef-cache

@REM -------------------------
@REM Download CEF distribution
@REM -------------------------
IF NOT EXIST "cef-cache\%CEFVER%" (
	cd cef-cache
	powershell -C "(New-Object Net.WebClient).DownloadFile('%CEFHOST%/%CEFVER%.tar.bz2', '%CEFVER%.tar.bz2')"
	cmake -E tar -xjf %CEFVER%.tar.bz2%
)

@echo =========== Current CEF cache folder ==================
@dir "%BASEDIR%\cef-cache"
@echo =======================================================

@REM ------------------
@REM Build CEF binaries
@REM ------------------
cd "%BASEDIR%\cef-cache\%CEFVER%"
cmake -B build -D USE_ATL=Off -DUSE_SANDBOX=Off -A Win32 .
cmake --build build
cmake --build build --config Release

@REM ------------------
@REM Install CEF assets
@REM ------------------
cd "%BASEDIR%"
cmake -E copy_directory "cef-cache\%CEFVER%\include" include
cmake -E copy_directory "cef-cache\%CEFVER%\build\libcef_dll_wrapper\Release" rlib
cmake -E copy_directory "cef-cache\%CEFVER%\build\libcef_dll_wrapper\Debug" lib
cmake -E copy "cef-cache\%CEFVER%\Release\libcef.lib" rlib
cmake -E copy "cef-cache\%CEFVER%\Release\libcef.lib" lib

cmake -E copy_directory "cef-cache\%CEFVER%\Release" D32
cmake -E copy_directory "cef-cache\%CEFVER%\Release" R32
cmake -E copy_directory "cef-cache\%CEFVER%\Resources" D32
cmake -E copy_directory "cef-cache\%CEFVER%\Resources" R32

endlocal
