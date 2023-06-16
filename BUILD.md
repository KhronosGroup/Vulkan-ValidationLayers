# Build Instructions

1. [Requirements](#requirements)
2. [Building Overview](#building-overview)
3. [Generated source code](#generated-source-code)
4. [Dependencies](#dependencies)
5. [Linux Build](#building-on-linux)
6. [Windows Build](#building-on-windows)
7. [MacOS build](#building-on-macos)
8. [Android Build](#building-on-android)
9. [Installed Files](#installed-files)

## Requirements

1. CMake >= 3.17.2
2. C++17 compatible toolchain
3. Git
4. Python >= 3.8

NOTE: Python is needed for working on generated code, and helping grab dependencies.
While it's not technically required, it's practically required for most users.

## Building Overview

The following will be enough for most people, for more detailed instructions, see below.

```bash
git clone https://github.com/KhronosGroup/Vulkan-ValidationLayers.git
cd Vulkan-ValidationLayers

cmake -S . -B build -D UPDATE_DEPS=ON -D BUILD_WERROR=ON -D BUILD_TESTS=ON -D CMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# CMake 3.21+
cmake -S . -B build --preset dev
cmake --build build --config Debug
```

### CMakePresets.json (3.21+)

[CMakePresets.json](./CMakePresets.json) can save developer time by specifying common build flags.

```bash
# Enables tests, enable werror, etc.
cmake -S . -B build/ --preset dev
```

### Warnings as errors off by default!

By default `BUILD_WERROR` is `OFF`. The idiom for open source projects is to NOT enable warnings as errors.

System/language package managers have to build on multiple different platforms and compilers.

By defaulting to `ON` we cause issues for package managers since there is no standard way to disable warnings until CMake 3.24

Add `-D BUILD_WERROR=ON` to your workflow. Or use the `dev` preset shown below which will also enabling warnings as errors.

## Generated source code

This repository contains generated source code in the `layers/vulkan/generated` directory which is not intended to be modified directly.

Please see the [Generated Code documentation](./docs/generated_code.md) for more information

## Dependencies

Currently this repo has a custom process for grabbing C/C++ dependencies.

Keep in mind this repo predates tools like `vcpkg`, `conan`, etc. Our process is most similar to `vcpkg`.

By specifying `-D UPDATE_DEPS=ON` when configuring CMake we grab dependencies listed in [known_good.json](scripts/known_good.json).

All we are doing is streamlining `building`/`installing` the `known good` dependencies and helping CMake `find` the dependencies.

This is done via a combination of `Python` and `CMake` scripting.

Misc Useful Information:

- By default `UPDATE_DEPS` is `OFF`. The intent is to be friendly by default to system/language package managers.
- You can run `update_deps.py` manually but it isn't recommended for most users.

### How to test new dependency versions

Typically most developers alter `known_good.json` with the commit/branch they are testing.

Alternatively you can modify `CMAKE_PREFIX_PATH` as follows.

```sh
# Delete the CMakeCache.txt which will cache find_* results
rm build -rf/
cmake -S . -B build/ ... -D CMAKE_PREFIX_PATH=~/foobar/my_custom_glslang_install/ ...
```

## Building On Linux

### Linux Build Requirements

This repository is regularly built and tested on the two most recent Ubuntu LTS versions.

```bash
sudo apt-get install git build-essential python3 cmake

# Linux WSI system libraries
sudo apt-get install libwayland-dev xorg-dev
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

## Building On Windows

### Windows Development Environment Requirements

- Windows 10+
- Visual Studio

Note: Anything less than `Visual Studio 2019` is not guaranteed to compile/work.

### Visual Studio Generator

Run CMake to generate [Visual Studio project files](https://cmake.org/cmake/help/latest/guide/user-interaction/index.html#command-line-g-option).

```bash
# NOTE: By default CMake picks the latest version of Visual Studio as the default generator.
cmake -S . -B build --preset dev

# Open the Visual Studio solution
cmake --open build
```

See the [CMake documentation](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#visual-studio-generators) for further information on Visual Studio generators.

NOTE: Windows developers don't have to develop in Visual Studio. Visual Studio just helps streamlining the needed C++ toolchain requirements (compilers, linker, etc).

## Building on MacOS

### MacOS Development Environment Requirements

- Xcode

NOTE: MacOS developers don't have to develop in Xcode. Xcode just helps streamlining the needed C++ toolchain requirements (compilers, linker, etc). Similar to Visual Studio on Windows.

### Xcode Generator

To create and open an Xcode project:

```bash
# Create the Xcode project
cmake -S . -B build -G Xcode --preset dev

# Open the Xcode project
cmake --open build
```

See the [CMake documentation](https://cmake.org/cmake/help/latest/generator/Xcode.html) for further information on the Xcode generator.

## Building On Android

### Android Build Requirements

- Download [Android Studio](https://developer.android.com/studio)
- Install (https://developer.android.com/studio/install)
- From the `Welcome to Android Studio` splash screen, add the following components using the SDK Manager:
  - SDK Platforms > Android 8.0 and newer (API Level 26 or higher)
  - SDK Tools > Android SDK Build-Tools
  - SDK Tools > Android SDK Platform-Tools
  - SDK Tools > Android SDK Tools
  - SDK Tools > NDK

#### Add Android specifics to environment

For each of the below, you may need to specify a different build-tools version, as Android Studio will roll it forward fairly regularly.

NOTE: The following commands are streamlined for Linux but easily transferable to other platforms.
The main intent is setting 2 environment variables and ensuring the NDK and build tools are in the `PATH`.

```sh
# Set environment variables
# https://github.com/actions/runner-images/blob/main/images/linux/Ubuntu2204-Readme.md#environment-variables-2
export ANDROID_SDK_ROOT=$HOME/Android/Sdk
export ANDROID_NDK_HOME=$ANDROID_SDK_ROOT/ndk/X.Y.Z

# Modify path
export PATH=$ANDROID_NDK_HOME:$PATH
export PATH=$ANDROID_SDK_ROOT/build-tools/X.Y.Z:$PATH

# Verify SDK build-tools is set correctly
which aapt

# Verify NDK path is set correctly
which ndk-build

# Check apksigner
apksigner --help
```

Note: If `apksigner` gives a `java: not found` error you do not have Java in your path.

```bash
# A common way to install on the system
sudo apt install default-jre
```

### Android Build

There are two options for building the Android layers. Either using the SPIRV
tools provided as part of the Android NDK, or using upstream sources. To build
with SPIRV tools from the NDK, remove the build-android/third_party directory
created by running update_external_sources_android.sh, (or avoid running
update_external_sources_android.sh). Use the following script to build
everything in the repository for Android, including validation layers, tests,
demos, and APK packaging: This script does retrieve and use the upstream SPRIV
tools.

```bash
# NOTE: If you haven't already you will need to generate a key with keytool before running build_all.sh
keytool -genkey -v -keystore ~/.android/debug.keystore -alias androiddebugkey -storepass android -keypass android -keyalg RSA -keysize 2048 -validity 10000 -dname 'CN=Android Debug,O=Android,C=US'

cd build-android
./build_all.sh
```

> **NOTE:** To allow manual installation of Android layer libraries on development devices, `build_all.sh` will use the static version of the c++ library (libc++_static.so). For testing purposes and general usage including installation via APK the c++ shared library should be used (libc++_shared.so). See comments in [build_all.sh](build-android/build_all.sh) for details.

> **NOTE:** By default, the `build_all.sh` script will build for all Android ABI variations. To **speed up the build time** if you know your target(s), set `APP_ABI` in both [build-android/jni/Application.mk](build-android/jni/Application.mk) and [build-android/jni/shaderc/Application.mk](build-android/jni/shaderc/Application.mk) to the desired [Android ABI](https://developer.android.com/ndk/guides/application_mk#app_abi)

Resulting validation layer binaries will be in `build-android/libs`. Test and demo APKs can be installed on production devices with:

```sh
./install_all.sh [-s <serial number>]
```

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

## CMake Installed Files

The installation depends on the target platform

For UNIX operating systems:

- *install_dir*`/lib` : The Vulkan validation layer library
- *install_dir*`/share/vulkan/explicit_layer.d` : The VkLayer_khronos_validation.json manifest

`NOTE`: Android doesn't use json manifests for Vulkan layers.

For WIN32:

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
