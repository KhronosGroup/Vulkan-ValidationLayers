@echo off
REM Update source for glslang, spirv-tools, and shaderc

REM
REM Copyright 2016 The Android Open Source Project
REM Copyright (C) 2015 Valve Corporation
REM Copyright 2018 LunarG, Inc.
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM      http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.
REM

setlocal EnableDelayedExpansion
set errorCode=0
set ANDROID_BUILD_DIR=%~dp0
set BUILD_DIR=%ANDROID_BUILD_DIR%
set BASE_DIR=%BUILD_DIR%\third_party
set SHADERC_DIR=%BASE_DIR%\shaderc

for %%X in (where.exe) do (set FOUND=%%~$PATH:X)
if not defined FOUND (
   echo Dependency check failed:
   echo   where.exe not found
   echo   This script requires Windows Vista or later, which includes where.exe.
   set errorCode=1
)

where /q git.exe
if %ERRORLEVEL% equ 1 (
   echo Dependency check failed:
   echo   git.exe not found
   echo   Git for Windows can be downloaded here:  https://git-scm.com/download/win
   echo   Install and ensure git.exe makes it into your PATH
   set errorCode=1
)

where /q ndk-build.cmd
if %ERRORLEVEL% equ 1 (
   echo Dependency check failed:
   echo   ndk-build.cmd not found
   echo   Android NDK can be downloaded here:  http://developer.android.com/ndk/guides/setup.html
   echo   Install and ensure ndk-build.cmd makes it into your PATH
   set errorCode=1
)

:main

if %errorCode% neq 0 (goto:error)

echo Creating and/or updating glslang, spirv-tools, spirv-headers, shaderc, vulkan-headers, vulkan-tools in %BASE_DIR%

set build-shaderc=1

REM Pull down or update external dependencies
echo Update external dependencies based on the %ANDROID_BUILD_DIR%/known_good.json file
py -3 ../scripts/update_deps.py --no-build --dir %BASE_DIR% --known_good_dir %BUILD_DIR%


if %build-shaderc% equ 1 (
   call:build_shaderc
   if %errorCode% neq 0 (goto:error)
)

echo.
echo Exiting
goto:finish

:error
echo.
echo Halting due to error
goto:finish

:finish
if not "%cd%\" == "%BUILD_DIR%" ( cd %BUILD_DIR% )
endlocal
REM This needs a fix to return error, something like exit %errorCode%
REM Right now it is returning 0
goto:eof



REM // ======== Functions ======== //

:build_shaderc
   echo.
   echo Building %SHADERC_DIR%
   cd %SHADERC_DIR%\android_test
   echo Building shaderc with Android NDK
   call ndk-build NDK_APPLICATION_MK=../../../jni/shaderc/Application.mk THIRD_PARTY_PATH=../third_party -j 4
   REM Check for existence of one lib, even though we should check for all results
   if not exist %SHADERC_DIR%\android_test\obj\local\x86\libshaderc.a (
      echo.
      echo shaderc build failed!
      set errorCode=1
   )
goto:eof
