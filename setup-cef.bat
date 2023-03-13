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
  set CEFVER=cef_binary_98.2.1+g29d6e22+chromium-98.0.4758.109_windows32_minimal
)
set CEFHOST=https://cef-builds.spotifycdn.com

echo %CEFVER%

@REM @REM -----------------
@REM @REM Cleanup directory
@REM @REM -----------------
@REM cd "%BASEDIR%"
@REM cmake -E rm -rf include lib rlib D32 R32
@REM cmake -E make_directory include lib rlib D32 R32
@REM cmake -E make_directory cef-cache

@REM @REM -------------------------
@REM @REM Download CEF distribution
@REM @REM -------------------------
@REM IF NOT EXIST "cef-cache\%CEFVER%" (
@REM 	cd cef-cache
@REM 	powershell -C "(New-Object Net.WebClient).DownloadFile('%CEFHOST%/%CEFVER%.tar.bz2', '%CEFVER%.tar.bz2')"
@REM 	cmake -E tar -xjf %CEFVER%.tar.bz2%
@REM )

@REM @echo =========== Current CEF cache folder ==================
@REM @dir "%BASEDIR%\cef-cache"
@REM @echo =======================================================

@REM @REM ------------------
@REM @REM Build CEF binaries
@REM @REM ------------------
@REM cd "%BASEDIR%\cef-cache\%CEFVER%"
@REM cmake -B build -D USE_ATL=Off -DUSE_SANDBOX=Off -A Win32 .
@REM cmake --build build
@REM cmake --build build --config Release

@REM @REM ------------------
@REM @REM Install CEF assets
@REM @REM ------------------
@REM cd "%BASEDIR%"
@REM cmake -E copy_directory "cef-cache\%CEFVER%\include" include
@REM cmake -E copy_directory "cef-cache\%CEFVER%\build\libcef_dll_wrapper\Release" rlib
@REM cmake -E copy_directory "cef-cache\%CEFVER%\build\libcef_dll_wrapper\Debug" lib
@REM cmake -E copy "cef-cache\%CEFVER%\Release\libcef.lib" rlib
@REM cmake -E copy "cef-cache\%CEFVER%\Release\libcef.lib" lib

@REM cmake -E copy_directory "cef-cache\%CEFVER%\Release" D32
@REM cmake -E copy_directory "cef-cache\%CEFVER%\Release" R32
@REM cmake -E copy_directory "cef-cache\%CEFVER%\Resources" D32
@REM cmake -E copy_directory "cef-cache\%CEFVER%\Resources" R32
