# Build Instructions

1. [Requirements](#requirements)
1. [Building Overview](#building-overview)
1. [Generated source code](#generated-source-code)
1. [Dependencies](#dependencies)
1. [Windows Build](#building-on-windows)
1. [Linux Build](#building-on-linux)
1. [Android Build](#building-on-android)
1. [MacOS build](#building-on-macos)
1. [Installed Files](#installed-files)

## Requirements

1. CMake >= 3.17.2
2. C++ >= c++17 compiler. See platform-specific sections below for supported compiler versions.
3. Python >= 3.7 (3.6 may work, 3.5 and earlier is not supported)

NOTE: Python is needed for working on generated code, and helping grab dependencies.
While it's not technically required, it's practically required for most users.

## Building Overview

The following will be enough for most people, for platform-specific instructions, see below.

```bash
git clone https://github.com/KhronosGroup/Vulkan-ValidationLayers.git
cd Vulkan-ValidationLayers

mkdir build
cd build

# Linux
python3 ../scripts/update_deps.py --dir ../external --arch x64 --config debug
cmake -G Ninja -C ../external/helper.cmake -DCMAKE_BUILD_TYPE=Debug ..

# Windows
python ..\scripts\update_deps.py --dir ..\external --arch x64 --config debug
cmake -A x64 -C ..\external\helper.cmake -DCMAKE_BUILD_TYPE=Debug ..

cmake --build . --config Debug
```

## Generated source code

This repository contains generated source code in the `layers/generated`
directory which is not intended to be modified directly.

Please see the [Generated Code documentation](./docs/generated_code.md) for more information

## Dependencies

### Display Drivers

This repository does not contain a Vulkan-capable driver. You will need to
obtain and install a Vulkan driver from your graphics hardware vendor or from
some other suitable source if you intend to run Vulkan applications.

### Building dependencies with script and Known-Good revisions

There is a Python utility script, `scripts/update_deps.py`, that you can use to
gather and build the dependent repositories mentioned above. This script uses
information stored in the `scripts/known_good.json` file to check out dependent
repository revisions that are known to be compatible with the revision of this
repository that you currently have checked out.

The script will produce a `helper.cmake` file that can be consumed by the cmake

run './scripts/update_deps.py --help' for more information

#### Updating with CMAKE_TOOLCHAIN_FILE

When cross compiling, you may want to pass arguments to the CMake build

```bash
update_deps.py ... --cmake_var CMAKE_TOOLCHAIN_FILE=/home/.../arm64.cmake
```

### Repository Dependencies

This repository attempts to resolve some of its dependencies by using
components found from the following places, in this order:

1. CMake or Environment variable overrides (e.g., -DVULKAN_HEADERS_INSTALL_DIR)
1. LunarG Vulkan SDK, located by the `VULKAN_SDK` environment variable
1. System-installed packages, mostly applicable on Linux

Dependencies that cannot be resolved by the SDK or installed packages must be
resolved with the "install directory" override and are listed below. The
"install directory" override can also be used to force the use of a specific
version of that dependency.

> Note: All test will be downloaded, built, and installed with `update_deps.py`

- [Vulkan Headers repository](https://github.com/KhronosGroup/Vulkan-Headers)
    - You must clone the headers repository and build its `install` target
- [SPIRV-Headers repository](https://github.com/KhronosGroup/SPIRV-Headers)
    - You must clone the headers repository and build its `install` target
- [SPIRV-Tools repository](https://github.com/KhronosGroup/SPIRV-Tools)
    - You must clone the headers repository and build its `install` target
- [robin-hood-hashing repository](https://github.com/martinus/robin-hood-hashing)
    - This is a header-only reimplementation of `std::unordered_map` and `std::unordered_set` which provides substantial performance improvements on all platforms.
    - You must clone this repository and build its `install` target
- [mimalloc repository](https://github.com/microsoft/mimalloc)
    - This is a reimplementation of malloc()/free() and their c++ equivalents.
    - It is currently only used for windows 64 bit builds, where it is statically linked into the layer.
    - For window 64 bit builds, you must clone this repository and build its `install` target

For running the tests:

- [Vulkan-Loader repository](https://github.com/KhronosGroup/Vulkan-Loader.git)
- [glslang repository](https://github.com/KhronosGroup/glslang)
    - You must clone the headers repository and build its `install` target
- [Google Test](https://github.com/google/googletest)

### Build and Install Directories

A common convention is to place the build directory in the top directory of
the repository with a name of `build` and place the install directory as a
child of the build directory with the name `install`. The remainder of these
instructions follow this convention, although you can use any name for these
directories and place them in any location (see option `--dir` in the
[notes](#notes)).

### Notes

- You may need to adjust some of the CMake options based on your platform. See
  the platform-specific sections later in this document.
- When using update_deps.py to change architectures (x64, Win32...)
  or build configurations (debug, release...) it is strongly recommended to
  add the '--clean-repo' parameter. This ensures compatibility among dependent
  components.
  dependent components will produce consistent build artifacts.
- The `update_deps.py` script fetches and builds the dependent repositories in
  the current directory when it is invoked. In this case, they are built in
  the `build` directory.
- The `build` directory is also being used to build this
  (Vulkan-ValidationLayers) repository. But there shouldn't be any conflicts
  inside the `build` directory between the dependent repositories and the
  build files for this repository.
- The `--dir` option for `update_deps.py` can be used to relocate the
  dependent repositories to another arbitrary directory using an absolute or
  relative path.
- The `update_deps.py` script generates a file named `helper.cmake` and places
  it in the same directory as the dependent repositories (`build` in this
  case). This file contains CMake commands to set the CMake `*_INSTALL_DIR`
  variables that are used to point to the install artifacts of the dependent
  repositories. You can use this file with the `cmake -C` option to set these
  variables when you generate your build files with CMake. This lets you avoid
  entering several `*_INSTALL_DIR` variable settings on the CMake command line.
- If using "MINGW" (Git For Windows), you may wish to run
  `winpty update_deps.py` in order to avoid buffering all of the script's
  "print" output until the end and to retain the ability to interrupt script
  execution.
- Please use `update_deps.py --help` to list additional options and read the
  internal documentation in `update_deps.py` for further information.
- You can build against different C++ standards by setting the
  `VVL_CPP_STANDARD` option at cmake generation time. Current code is written
  to compile under C++17.

## CMake

### Warnings as errors off by default!

By default `BUILD_WERROR` is `OFF`

The idiom for open source projects is to NOT enable warnings as errors.

System package managers, and language package managers have to build on multiple different platforms and compilers.

By defaulting to `ON` we cause issues for package managers since there is no standard way to disable warnings until CMake 3.24

Add `-D BUILD_WERROR=ON` to your workflow. Or use the `dev` preset shown below which will also enabling warnings as errors.

### CMakePresets.json (3.21+)

[CMakePresets.json](./CMakePresets.json) can save developer time by specifying common build flags.

```bash
# Enables tests, enable werror, etc.
cmake -S . -B build/ --preset dev
```

## Building On Windows

### Windows Development Environment Requirements

- Windows
  - Any Personal Computer version supported by Microsoft
- Microsoft [Visual Studio](https://www.visualstudio.com/)
  - Versions
    - [2022](https://www.visualstudio.com/vs/downloads/)
    - [2017-2019](https://www.visualstudio.com/vs/older-downloads/)
  - The Community Edition of each of the above versions is sufficient, as
    well as any more capable edition.
- [CMake 3.17.2](https://cmake.org/files/v3.17/cmake-3.17.2-win64-x64.zip) is the minimum CMake version supported.  [CMake 3.19.3](https://cmake.org/files/v3.19/cmake-3.19.3-win64-x64.zip) is recommended.
  - Use the installer option to add CMake to the system PATH
- Git Client Support
  - [Git for Windows](http://git-scm.com/download/win) is a popular solution
    for Windows
  - Some IDEs (e.g., [Visual Studio](https://www.visualstudio.com/),
    [GitHub Desktop](https://desktop.github.com/)) have integrated
    Git client support

### Windows Build - Microsoft Visual Studio

Run CMake to generate the Visual Studio project files.

The `-A` option is used to select either the "Win32" or "x64" architecture.

If a generator for a specific version of Visual Studio is required, you can
specify it with the `-G` switch.

```batch
# Generates Visual Studio 2022 for x64
cmake -G "Visual Studio 17 2022" -A x64 -C ..\external\helper.cmake -DCMAKE_BUILD_TYPE=Debug ..
# Generates Visual Studio 2022 for Win32
cmake -G "Visual Studio 17 2022" -A Win32 -C ..\external\helper.cmake -DCMAKE_BUILD_TYPE=Debug ..
```

Note: VS2019 and higher target "x64" by default. VS2017 and lower target "Win32" by default

Check the [official CMake documentation](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators) for further information on Visual Studio generators.

The above steps create a Windows solution file named
`Vulkan-ValidationLayers.sln` in the build directory.

At this point, you can build the solution from the command line or open the
generated solution with Visual Studio.

If you want to build the Solution From the Command Line

```batch
cmake --build . --config Debug
# Or
cmake --build . --config Release
```

## Building On Linux

### Linux Build Requirements

This repository has been built and tested on the two most recent Ubuntu LTS
versions. Currently, the oldest supported version is Ubuntu 18.04, meaning
that the minimum officially supported C++17 compiler version is GCC 7.3.0,
although earlier versions may work. It should be straightforward to adapt this
repository to other Linux distributions.

[CMake 3.17.2](https://cmake.org/files/v3.17/cmake-3.17.2-Linux-x86_64.tar.gz) is recommended.

```bash
sudo apt-get install git build-essential libx11-xcb-dev \
        libxkbcommon-dev libwayland-dev libxrandr-dev \
        libegl1-mesa-dev python3-distutils
```

### WSI Support Build Options

By default, the repository components are built with support for the
Vulkan-defined WSI display servers: Xcb, Xlib, and Wayland. It is recommended
to build the repository components with support for these display servers to
maximize their usability across Linux platforms. If it is necessary to build
these modules without support for one of the display servers, the appropriate
CMake option of the form `BUILD_WSI_xxx_SUPPORT` can be set to `OFF`.

### Linux 32-bit support

Usage of this repository's contents in 32-bit Linux environments is not
officially supported. However, since this repository is supported on 32-bit
Windows, these modules should generally work on 32-bit Linux.

Here are some notes for building 32-bit targets on a 64-bit Ubuntu "reference"
platform:

```bash
# 32-bit libs
# your PKG_CONFIG configuration may be different, depending on your distribution
sudo apt-get install gcc-multilib g++-multilib libx11-dev:i386
```

Set up your environment for building 32-bit targets:

```bash
export ASFLAGS=--32
export CFLAGS=-m32
export CXXFLAGS=-m32
export PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu
```

## Building On Android

### Android Build Requirements

Note that the minimum supported Android SDK API Level is 26, revision
level 3.

NDK r20 or greater required

- Install [Android Studio 2.3](https://developer.android.com/studio/index.html)
  or later.
- From the "Welcome to Android Studio" splash screen, add the following
  components using Configure > SDK Manager:
  - SDK Platforms > Android 8.0.0 and newer
  - SDK Tools > Android SDK Build-Tools
  - SDK Tools > Android SDK Platform-Tools
  - SDK Tools > Android SDK Tools
  - SDK Tools > NDK

#### Android Hardware Buffer support

The Validation Layers by default build and release for Android 26 (Android Oreo). While Vulkan is supported in Android 24 and 25, there is no AHardwareBuffer support. To build a version of the Validation Layers for use with Android that will not require AHB support, simply addjust the `APP_PLATFORM` in [build-android/jni/Application.mk](build-android/jni/Application.mk)

```patch
-APP_PLATFORM := android-26
+APP_PLATFORM := android-24
```

#### Add Android specifics to environment

For each of the below, you may need to specify a different build-tools
version, as Android Studio will roll it forward fairly regularly.

On Linux:

```bash
export ANDROID_SDK_HOME=$HOME/Android/sdk
export ANDROID_NDK_HOME=$HOME/Android/sdk/ndk-bundle
export PATH=$ANDROID_SDK_HOME:$PATH
export PATH=$ANDROID_NDK_HOME:$PATH
# Where X.Y.Z is the most recent version number inside the build-tools directory.
export PATH=$ANDROID_SDK_HOME/build-tools/X.Y.Z:$PATH
```

On Windows:

    set ANDROID_SDK_HOME=%LOCALAPPDATA%\Android\sdk
    set ANDROID_NDK_HOME=%LOCALAPPDATA%\Android\sdk\ndk-bundle
    set PATH=%LOCALAPPDATA%\Android\sdk\ndk-bundle;%PATH%

On OSX:

```bash
export ANDROID_SDK_HOME=$HOME/Library/Android/sdk
export ANDROID_NDK_HOME=$HOME/Library/Android/sdk/ndk-bundle
export PATH=$ANDROID_NDK_HOME:$PATH
# Where X.Y.Z is the most recent version number inside the build-tools directory.
export PATH=$ANDROID_SDK_HOME/build-tools/X.Y.Z:$PATH
```

Note: If `jarsigner` is missing from your platform, you can find it in the
Android Studio install or in your Java installation. If you do not have Java,
you can get it with something like the following:

  sudo apt-get install openjdk-8-jdk

#### Additional OSX System Requirements

Tested on OSX version 10.15

Setup Homebrew and components

- Ensure Homebrew is at the beginning of your PATH:

```
export PATH=/usr/local/bin:$PATH
```

- Add packages with the following:

      brew install python

### Android Build

There are two options for building the Android layers. Either using the SPIRV
tools provided as part of the Android NDK, or using upstream sources. To build
with SPIRV tools from the NDK, remove the build-android/third_party directory
created by running update_external_sources_android.sh, (or avoid running
update_external_sources_android.sh). Use the following script to build
everything in the repository for Android, including validation layers, tests,
demos, and APK packaging: This script does retrieve and use the upstream SPRIV
tools.

    cd build-android
    ./build_all.sh

> **NOTE:** By default, the `build_all.sh` script will build for all Android ABI variations. To **speed up the build time** if you know your target(s), set `APP_ABI` in both [build-android/jni/Application.mk](build-android/jni/Application.mk) and [build-android/jni/shaderc/Application.mk](build-android/jni/shaderc/Application.mk) to the desired [Android ABI](https://developer.android.com/ndk/guides/application_mk#app_abi)

Resulting validation layer binaries will be in build-android/libs. Test and
demo APKs can be installed on production devices with:

    ./install_all.sh [-s <serial number>]

Note that there are no equivalent scripts on Windows yet, that work needs to
be completed. The following per platform commands can be used for layer only
builds:

#### Linux and OSX

Follow the setup steps for Linux or OSX above, then from your terminal:

    cd build-android
    ./update_external_sources_android.sh --no-build
    ndk-build -j4

#### Windows

Follow the setup steps for Windows above, then from the Developer Command Prompt:

    cd build-android
    update_external_sources_android.bat
    ndk-build

## Building on MacOS

### MacOS Build Requirements

Tested on OSX version 10.15

NOTE: To force the OSX version set the environment variable [MACOSX_DEPLOYMENT_TARGET](https://cmake.org/cmake/help/latest/envvar/MACOSX_DEPLOYMENT_TARGET.html) when building VVL and it's dependencies.

[CMake 3.17.2](https://cmake.org/files/v3.17/cmake-3.17.2-Darwin-x86_64.tar.gz) is recommended.

Setup Homebrew and components


- Ensure Homebrew is at the beginning of your PATH:

```
export PATH=/usr/local/bin:$PATH
```

- Add packages with the following (may need refinement)

      brew install python python3 git

### Building with the Xcode Generator

To create and open an Xcode project:

```bash
cmake -G Xcode -C ../external/helper.cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --open .
```

Within Xcode, you can select Debug or Release builds in the Build Settings of the project.

## Installed Files

The installation depends on the target platform

For UNIX operating systems:

- *install_dir*`/lib` : The Vulkan validation layer library
- *install_dir*`/share/vulkan/explicit_layer.d` : The VkLayer_khronos_validation.json manifest

For WIN32/MINGW:

- *install_dir*`/bin` : The Vulkan validation layer library
- *install_dir*`/bin` : The VkLayer_khronos_validation.json manifest

### Software Installation

After you have built your project you can install using CMake's install functionality.

CMake Docs:
- [Software Installation Guide](https://cmake.org/cmake/help/latest/guide/user-interaction/index.html#software-installation)
- [CLI for installing a project](https://cmake.org/cmake/help/latest/manual/cmake.1.html#install-a-project)

```sh
# EX: Installs Release artifacts into `build/install` directory.
# NOTE: --config is only needed for multi-config generators (Visual Studio, Xcode, etc)
cmake --install build/ --config Release --prefix build/install
```
