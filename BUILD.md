# Build Instructions

Instructions for building this repository on Linux, Windows, Android, and MacOS.

## Index

1. [Contributing](#contributing-to-the-repository)
2. [Repository Set-Up](#repository-set-up)
3. [Windows Build](#building-on-windows)
4. [Linux Build](#building-on-linux)
5. [Android Build](#building-on-android)
6. [MacOS build](#building-on-macos)

## Contributing to the Repository

If you intend to contribute, the preferred work flow is for you to develop
your contribution in a fork of this repository in your GitHub account and
then submit a pull request.
Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file in this repository for more details.

## Repository Set-Up

### Display Drivers

This repository does not contain a Vulkan-capable driver.
Before proceeding, it is strongly recommended that you obtain a Vulkan driver from your
graphics hardware vendor and install it properly.

### Download the Repository

To create your local git repository:

    git clone https://github.com/KhronosGroup/Vulkan-ValidationLayers

## Building On Windows

### Windows Build Requirements

Windows 7+ with the following software packages:

- Microsoft Visual Studio 2013 Update 4 Professional, VS2015 (any version), or VS2017 (any version).
- [glslang](https://github.com/KhronosGroup/glslang)
  - Ensure that the 'update_glslang_sources.py' script has been run. Follow the build instructions in the
    glslang [README.md](https://github.com/KhronosGroup/glslang/blob/master/README.md) file, noting the location of the chosen install directory.
- [CMake](http://www.cmake.org/download/)
  - Tell the installer to "Add CMake to the system PATH" environment variable.
- [Python 3](https://www.python.org/downloads)
  - Select to install the optional sub-package to add Python to the system PATH
    environment variable.
  - Ensure the `pip` module is installed (it should be by default)
  - Python3.3 or later is necessary for the Windows py.exe launcher that is used to select python3
  rather than python2 if both are installed
- [Git](http://git-scm.com/download/win)
  - Tell the installer to allow it to be used for "Developer Prompt" as well as "Git Bash".
  - Tell the installer to treat line endings "as is" (i.e. both DOS and Unix-style line endings).
  - Install both the 32-bit and 64-bit versions, as the 64-bit installer does not install the
    32-bit libraries and tools.
  - Tell the installer to treat line endings "as is" (i.e. both DOS and Unix-style line endings).
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers.git)
  - Vulkan headers should be built with the "install" target. Generally, you will want to use the
    `CMAKE_INSTALL_PREFIX` and remember where you set this to.
- Vulkan Loader Library
  - Building the Layer Validation Tests requires linking to the Vulkan Loader Library (vulkan-1.dll).
    Locating the library for this repo can be done in two different ways:
      -  The Vulkan SDK can be installed. In this case, cmake should be able to locate the loader repo through the VulkanSDK
         environment variable
      -  The library can be built from the [Vulkan-Loader](https://github.com/KhronosGroup/Vulkan-Loader.git) repository. This can be done by setting 'CMAKE_INSTALL_PREFIX' and building 'install' target from within the Vulkan-Loader repo, noting the install location.
         In this case, the following option should be used on the cmake command line:

             cmake -DVULKAN_LOADER_INSTALL_DIR=c:\absolute_path_to\Vulkan-Loader\install

- [googletest](https://github.com/google/googletest.git)
  - This is an optional component, but required for building the validation layer tests. To install,

        cd <the root directory of your copy of the Vulkan-ValidationLayers repository, "external" will be a subdirectory>
        git clone https://github.com/google/googletest.git external/googletest

  - The gtest libraries will be built as part of the main project cmake/build process

### Windows Build - Microsoft Visual Studio

1. Open a Developer Command Prompt for VS201x
2. Change directory to `Vulkan-ValidationLayers` -- the root of the cloned git repository
3. Note the location of a local glslang repository. This will be passed to cmake as a command line variable.
4. Note the location of a local Vulkan-Headers install location. This will be passed to cmake as a command line variable.
5. Create a `build` directory, change into that directory, and run cmake

For example, for VS2017 (generators for other versions are [specified here](#cmake-visual-studio-generators)):

    cmake -DVULKAN_HEADERS_INSTALL_DIR=c:\absolute_path_to\Vulkan-Headers\install -DVULKAN_LOADER_INSTALL_DIR=c:\absolute_path_to\VULKAN_LOADER\install -DGLSLANG_INSTALL_DIR=c:\absolute_path_to\glslang\install -G "Visual Studio 15 2017 Win64" ..

VULKAN_LOADER_INSTALL_DIR, VULKAN_HEADERS_INSTALL_DIR, and GLSLANG_INSTALL_DIR each require absolute paths.
This will create a Windows solution file named `Vulkan-ValidationLayers.sln` in the build directory.

Launch Visual Studio and open the "Vulkan-ValidationLayers.sln" solution file in the build folder.
You may select "Debug" or "Release" from the Solution Configurations drop-down list.
Start a build by selecting the Build->Build Solution menu item.

### Windows Tests and Demos

After making any changes to the repository, you should perform some quick sanity tests,
including the run_all_tests Powershell script. In addition, running sample applications such as
the [cube demo](https://www.github.com/KhronosGroup/Vulkan-Tools.git) with validation enabled is
advised.

To run the validation test script, open a Powershell Console,
change to the build/tests directory, and run:

For Release builds:

    .\run_all_tests.ps1

For Debug builds:

    .\run_all_tests.ps1 -Debug

This script will run the following tests:

- `vk_layer_validation_tests`:
  Test Vulkan validation layers
- `vkvalidatelayerdoc`:
  Tests that validation database is up-to-date and is synchronized with the validation source code

### Windows Notes

#### CMake Visual Studio Generators

The above example used Visual Studio 2017, and specified its generator as "Visual Studio 15 2017 Win64".
The chosen generator should match your Visual Studio version. Appropriate Visual Studio generators include:

| Build Platform               | 64-bit Generator              | 32-bit Generator        |
|------------------------------|-------------------------------|-------------------------|
| Microsoft Visual Studio 2013 | "Visual Studio 12 2013 Win64" | "Visual Studio 12 2013" |
| Microsoft Visual Studio 2015 | "Visual Studio 14 2015 Win64" | "Visual Studio 14 2015" |
| Microsoft Visual Studio 2017 | "Visual Studio 15 2017 Win64" | "Visual Studio 15 2017" |

## Building On Linux

### Linux Build Requirements

This repository has been built and tested on the two most recent Ubuntu LTS versions.
Currently, the oldest supported version is Ubuntu 14.04, meaning that the minimum supported
compiler versions are GCC 4.8.2 and Clang 3.4, although earlier versions may work.
It should be straightforward to adapt this repository to other Linux distributions.

**Required Package List:**

    sudo apt-get install git cmake build-essential libx11-xcb-dev libxkbcommon-dev libmirclient-dev libwayland-dev libxrandr-dev

- [Vulkan-Loader](https://github.com/Khronosgroup/Vulkan-Loader.git)
  - Building the Layer Validation Tests requires linking to the Vulkan Loader Library (libvulkan-1.so). This can be done by setting 'CMAKE_INSTALL_PREFIX' and building 'install' target from within the Vulkan-Loader repo, noting the install location.
      - The VULKAN_LOADER_INSTALL_DIR environment variable should be used on the cmake command line to specify a vulkan loader library, like so:

          `cmake -DVULKAN_LOADER_INSTALL_DIR=c:\absolute_path_to\Vulkan-Loader\install ....`

- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers.git)
  - This repo should be built and installed to the directory of your choosing. This can be done by setting `CMAKE_INSTALL_PREFIX`
    and building the "install" target from within the Vulkan-Headers repository.

- [glslang](https://github.com/KhronosGroup/glslang)
  - Ensure that the 'update_glslang_sources.py' script has been run. Follow the build instructions in the
    glslang [README.md](https://github.com/KhronosGroup/glslang/blob/master/README.md) file, noting the location of the chosen install directory.

- [googletest](https://github.com/google/googletest.git)
  - This is an optional component, but required for building the validation layer tests. To install,

        cd <the root directory of your copy of the Vulkan-ValidationLayers repository, "external" will be a subdirectory>
        git clone https://github.com/google/googletest.git external/googletest

  - The gtest libraries will be built as part of the main project cmake/build process

### Linux Build

Example debug build

See **Validation Layer Dependencies** (below) for more information and other options:

1. In a Linux terminal, `cd Vulkan-ValidationLayers` -- the root of the cloned git repository
2. Create a `build` directory, change into that directory, and run cmake using absolute paths for the repo locations:

        mkdir build
        cd build
        cmake -DVULKAN_HEADERS_INSTALL_DIR=/absolute_path_to/Vulkan-Headers/install -DVULKAN_LOADER_INSTALL_DIR=/absolute_path_to/Vulkan-Loader/install -DGLSLANG_INSTALL_DIR=/absolute_path_to_/glslang/location_of/install -DCMAKE_BUILD_TYPE=Debug ..

3. Run `make -j8` to begin the build

If your build system supports ccache, you can enable that via CMake option `-DUSE_CCACHE=On`

#### Using the new layers

    export VK_LAYER_PATH=<path to your repository root>/build/layers

You can run the `cube` or `vulkaninfo` applications from the Vulkan-Tools repository to see which driver, loader and layers are being used.

### WSI Support Build Options

By default, the Validation Layers are built with support for all 4 Vulkan-defined WSI display servers: Xcb, Xlib, Wayland, and Mir.
It is recommended to build the repository components with support for these display servers to maximize their usability across Linux platforms.
If it is necessary to build these modules without support for one of the display servers, the appropriate CMake option of the form `BUILD_WSI_xxx_SUPPORT` can be set to `OFF`.
See the top-level CMakeLists.txt file for more info.

### Linux Install to System Directories

Installing the files resulting from your build to the systems directories is optional since environment variables can usually be used instead to locate the binaries.
There are also risks with interfering with binaries installed by packages.
If you are certain that you would like to install your binaries to system directories, you can proceed with these instructions.

Assuming that you have built the code as described above and the current directory is still `build`, you can execute:

    sudo make install

This command installs files to:

- `/usr/local/lib`:  Vulkan layers shared objects
- `/usr/local/share/vulkan/explicit_layer.d`:  Layer JSON files

You can further customize the installation location by setting additional CMake variables to override their defaults.
For example, if you would like to install to `/tmp/build` instead of `/usr/local`, on your CMake command line specify:

    -DCMAKE_INSTALL_PREFIX=/tmp/build
    -DDEST_DIR=/tmp/build

Then run `make install` as before. The install step places the files in `/tmp/build`.

Using the `CMAKE_INSTALL_PREFIX` to customize the install location also modifies
the loader search paths to include searching for layers in the specified install location.
In this example, setting `CMAKE_INSTALL_PREFIX` to `/tmp/build` causes the loader to search
`/tmp/build/etc/vulkan/explicit_layer.d` and `/tmp/build/share/vulkan/explicit_layer.d`
for the layer JSON files.
The loader also searches the "standard" system locations of `/etc/vulkan/explicit_layer.d` and
`/usr/share/vulkan/explicit_layer.d` after searching the two locations under `/tmp/build`.

You can further customize the installation directories by using the CMake variables
`CMAKE_INSTALL_SYSCONFDIR` to rename the `etc` directory and `CMAKE_INSTALL_DATADIR`
to rename the `share` directory.

See the CMake documentation for more details on using these variables
to further customize your installation.

Also see the `LoaderAndLayerInterface` document in the [Vulkan-Loader](https://github.com/KhronosGroup/Vulkan-Loader)
repository for more information about loader operation.

### Linux Uninstall

To uninstall the files from the system directories, you can execute:

    sudo make uninstall

### Linux Tests

After making any changes to the repository, you should perform the included sanity tests by running
the run_all_tests shell script.

To run the **validation test script**, in a terminal change to the build/tests directory and run:

    VK_LAYER_PATH=../layers ./run_all_tests.sh

This script will run the following tests:

- `vk_layer_validation_tests`: Test Vulkan validation layers
- `vkvalidatelayerdoc`: Tests that validation database is in up-to-date and in synchronization with
  the validation source code

Further testing and sanity checking can be achieved by running the cube and vulkaninfo applications
in the [Vulkan-Tools](https://github.com/KhronosGroup/Vulkan-Tools) repository.


### Linux Notes

#### Linux 32-bit support

Usage of the contents of this repository in 32-bit Linux environments is not officially supported.
However, since this repository is supported on 32-bit Windows,
these modules should generally work on 32-bit Linux.

Here are some notes for building 32-bit targets on a 64-bit Ubuntu "reference" platform:

If not already installed, install the following 32-bit development libraries:

`gcc-multilib g++-multilib libx11-dev:i386`

This list may vary depending on your distribution and which windowing systems you are building for.

Set up your environment for building 32-bit targets:

    export ASFLAGS=--32
    export CFLAGS=-m32
    export CXXFLAGS=-m32
    export PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu

Again, your PKG_CONFIG configuration may be different, depending on your distribution.

Finally, rebuild the repository using `cmake` and `make`, as explained above.

## Building On Android

Install the required tools for Linux and Windows covered above, then add the following.

### Android Build Requirements

- Install [Android Studio 2.3](https://developer.android.com/studio/index.html) or later.
- From the "Welcome to Android Studio" splash screen, add the following components using
  Configure > SDK Manager:
  - SDK Platforms > Android 6.0 and newer
  - SDK Tools > Android SDK Build-Tools
  - SDK Tools > Android SDK Platform-Tools
  - SDK Tools > Android SDK Tools
  - SDK Tools > NDK

#### Add Android specifics to environment

For each of the below, you may need to specify a different build-tools version, as Android Studio will roll it forward fairly regularly.

On Linux:

    export ANDROID_SDK_HOME=$HOME/Android/sdk
    export ANDROID_NDK_HOME=$HOME/Android/sdk/ndk-bundle
    export PATH=$ANDROID_SDK_HOME:$PATH
    export PATH=$ANDROID_NDK_HOME:$PATH
    export PATH=$ANDROID_SDK_HOME/build-tools/23.0.3:$PATH

On Windows:

    set ANDROID_SDK_HOME=%LOCALAPPDATA%\Android\sdk
    set ANDROID_NDK_HOME=%LOCALAPPDATA%\Android\sdk\ndk-bundle
    set PATH=%LOCALAPPDATA%\Android\sdk\ndk-bundle;%PATH%

On OSX:

    export ANDROID_SDK_HOME=$HOME/Library/Android/sdk
    export ANDROID_NDK_HOME=$HOME/Library/Android/sdk/ndk-bundle
    export PATH=$ANDROID_NDK_PATH:$PATH
    export PATH=$ANDROID_SDK_HOME/build-tools/23.0.3:$PATH

Note: If `jarsigner` is missing from your platform, you can find it in the
Android Studio install or in your Java installation.
If you do not have Java, you can get it with something like the following:

  sudo apt-get install openjdk-8-jdk

#### Additional OSX System Requirements

Tested on OSX version 10.13.3

Setup Homebrew and components

- Follow instructions on [brew.sh](http://brew.sh) to get Homebrew installed.

      /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

- Ensure Homebrew is at the beginning of your PATH:

      export PATH=/usr/local/bin:$PATH

- Add packages with the following:

      brew install cmake python

### Android Build

There are two options for building the Android layers.
Either using the SPIRV tools provided as part of the Android NDK, or using upstream sources.
To build with SPIRV tools from the NDK, remove the build-android/third_party directory created by
running update_external_sources_android.sh, (or avoid running update_external_sources_android.sh).
Use the following script to build everything in the repository for Android, including validation
layers, tests, demos, and APK packaging: This script does retrieve and use the upstream SPRIV tools.

    cd build-android
    ./build_all.sh

Resulting validation layer binaries will be in build-android/libs.
Test and demo APKs can be installed on production devices with:

    ./install_all.sh [-s <serial number>]

Note that there are no equivalent scripts on Windows yet, that work needs to be completed.
The following per platform commands can be used for layer only builds:

#### Linux and OSX

Follow the setup steps for Linux or OSX above, then from your terminal:

    cd build-android
    ./update_external_sources_android.sh --no-build
    ./android-generate.sh
    ndk-build -j4

#### Windows

Follow the setup steps for Windows above, then from Developer Command Prompt for VS2013:

    cd build-android
    update_external_sources_android.bat
    android-generate.bat
    ndk-build

### Android Tests and Demos

After making any changes to the repository you should perform some quick sanity tests,
including the layer validation tests and the cube and smoke demos with validation enabled.

#### Run Layer Validation Tests

Use the following steps to build, install, and run the layer validation tests for Android:

    cd build-android
    ./build_all.sh
    adb install -r bin/VulkanLayerValidationTests.apk
    adb shell am start com.example.VulkanLayerValidationTests/android.app.NativeActivity

Alternatively, you can use the test_APK script to install and run the layer validation tests:

    test_APK.sh -s <serial number> -p <plaform name> -f <gtest_filter>

## Building on MacOS

### MacOS Build Requirements

Tested on OSX version 10.12.6

Setup Homebrew and components

- Follow instructions on [brew.sh](http://brew.sh) to get Homebrew installed.

      /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

- Ensure Homebrew is at the beginning of your PATH:

      export PATH=/usr/local/bin:$PATH

- Add packages with the following (may need refinement)

      brew install cmake python python3 git

### Clone the Repository

Clone the Vulkan-ValidationLayers repository:

    git clone https://github.com/KhronosGroup/Vulkan-ValidationLayers.git

#### Out-of-Tree External Libraries

Also clone the following repos:

- [glslang](https://github.com/KhronosGroup/glslang)
  - Ensure that the 'update_glslang_sources.py' script has been run, and the repository successfully built.
- [Vulkan-Loader](https://github.com/KhronosGroup/Vulkan-Loader.git)
  - Building the Layer Validation Tests requires linking to the Vulkan Loader Library (libvulkan.1.dylib).
    Locating the library for this repo can be done in two different ways:
      -  The Vulkan SDK can be installed. In this case, cmake should be able to locate the loader repo through the VulkanSDK
         environment variable
      -  The library can be built from the [Vulkan-Loader](https://github.com/KhronosGroup/Vulkan-Loader.git) repository. This can be done by setting 'CMAKE_INSTALL_PREFIX' and building 'install' target from within the Vulkan-Loader repo, noting the location.
             cmake -DVULKAN_LOADER_INSTALL_DIR=/absolute_path_to_/Vulkan-Loader/install ....
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers.git)
  - This repo should be built and installed to the directory of your choosing. This can be done by setting `CMAKE_INSTALL_PREFIX`
    and building the "install" target from within the Vulkan-Headers repository.
- [googletest](https://github.com/google/googletest.git)
  - This is an optional component, but required for building the validation layer tests. To install,

        cd <the root directory of your copy of the Vulkan-ValidationLayers repository, "external" will be a subdirectory>
        git clone https://github.com/google/googletest.git external/googletest

  - The gtest libraries will be built as part of the main project cmake/build process


### MacOS build

#### CMake Generators

This repository uses CMake to generate build or project files that are
then used to build the repository.
The CMake generators explicitly supported in this repository are:

- Unix Makefiles
- Xcode

#### Building with the Unix Makefiles Generator

This generator is the default generator, so all that is needed for a debug
build is:

        mkdir build
        cd build
        cmake -DVULKAN_HEADERS_INSTALL_DIR=/absolute_path_to/Vulkan-Headers/install -DVULKAN_LOADER_INSTALL_DIR=/absolute_path_to/Vulkan-Loader/install -DGLSLANG_INSTALL_DIR=/absolute_path_to_/glslang/build/install -DCMAKE_BUILD_TYPE=Debug ..
        make

To speed up the build on a multi-core machine, use the `-j` option for `make`
to specify the number of cores to use for the build.
For example:

    make -j4

#### Building with the Xcode Generator

To create and open an Xcode project:

        mkdir build-xcode
        cd build-xcode
        cmake -DVULKAN_LOADER_INSTALL_DIR=/absolute_path_to/Vulkan-Loader/install -DGLSLANG_INSTALL_DIR=/absolute_path_to_/glslang/build/install -GXcode ..
        open VULKAN.xcodeproj

Within Xcode, you can select Debug or Release builds in the Build Settings of the project.
You can also select individual schemes for working with specific applications like `cube`.

#### Using the new layers

    export VK_LAYER_PATH=<path to your repository root>/build/layers

You can run the `vulkaninfo` applications from the Vulkan-Tools repository to see which driver, loader and layers are being used.

### MacOS Tests

After making any changes to the repository, you should perform the included sanity tests by running
the run_all_tests shell script.

These test require a manual path to an ICD to run properly on MacOS.

You can use:

- MoltenVK ICD
- Mock ICD

#### Using MoltenVK ICD

Clone and build the [MoltenVK](https://github.com/KhronosGroup/MoltenVK) repository.

You will have to direct the loader from Vulkan-Loader to the MoltenVK ICD:

    export VK_ICD_FILENAMES=<path to MoltenVK repository>/Package/Latest/MoltenVK/macOS/MoltenVK_icd.json

#### Using Mock ICD

Clone and build the [Vulkan-Tools](https://github.com/KhronosGroup/Vulkan-Tools) repository.

You will have to direct the loader from Vulkan-Loader to the Mock ICD:

    export VK_ICD_FILENAMES=<path to Vulkan-Tools repository>/build/icd/VkICD_mock_icd.json

#### Running the Tests

To run the **validation test script**, in a terminal change to the build/tests directory and run:

    VK_LAYER_PATH=../layers ./run_all_tests.sh

This script will run the following tests:

- `vk_layer_validation_tests`: Test Vulkan validation layers
- `vkvalidatelayerdoc`: Tests that validation database is in up-to-date and in synchronization with
  the validation source code

Further testing and sanity checking can be achieved by running the cube and vulkaninfo applications
in the [Vulkan-Tools](https://github.com/KhronosGroup/Vulkan-Tools) repository.

Note that MoltenVK is still adding Vulkan features and some tests may fail.

## Ninja Builds - All Platforms

The [Qt Creator IDE](https://qt.io/download-open-source/#section-2) can open a root CMakeList.txt
as a project directly, and it provides tools within Creator to configure and generate Vulkan SDK
build files for one to many targets concurrently.
Alternatively, when invoking CMake, use the `-G "Codeblocks - Ninja"` option to generate Ninja build
files to be used as project files for QtCreator

- Follow the steps defined elsewhere for the OS as shown in **Validation Layer Dependencies** below
- Open, configure, and build the glslang CMakeList.txt files. Note that building the glslang
  project will provide access to spirv-tools and spirv-headers
- Then do the same with the Vulkan-ValidationLayers CMakeList.txt file
- In order to debug with QtCreator, a
  [Microsoft WDK: eg WDK 10](http://go.microsoft.com/fwlink/p/?LinkId=526733) is required.

Note that installing the WDK breaks the MSVC vcvarsall.bat build scripts provided by MSVC,
requiring that the LIB, INCLUDE, and PATHenv variables be set to the WDK paths by some other means

## Validation Layer Dependencies

The glslang repository is required to build and run Validation Layer components.
Instructions to install an instance of the glslang repository follow here.

1) clone the repository:

    `git clone https://github.com/KhronosGroup/glslang.git`

2) Execute the glslang python script to pull in the SPIRV-tools componenets:

    `python update_glslang_sources.py`

3) Configure the glslang source tree with CMake and build it with your IDE of choice

After installing and building glslang, the location will be used to build the Vulkan-ValidationLayers repo:

1) Pass in the location of your glslang repository to cmake using absolute paths. From your build directory run:
    1) on Windows

        `cmake -DGLSLANG_INSTALL_DIR=c:/absolute_path_to/glslang/location_of/install -G "Visual Studio 15 Win64" ..`
    1) or Linux

        `cmake -DGLSLANG_INSTALL_DIR=/absolute_path_to/glslang/location_of/install -DCMAKE_BUILD_TYPE=Debug ..`

2) If building on Windows with MSVC, set `DISABLE_BUILDTGT_DIR_DECORATION` to _On_.
 If building on Windows, but without MSVC set `DISABLE_BUILD_PATH_DECORATION` to _On_

## Optional software packages

- [Cygwin for windows](https://www.cygwin.com/)
  - Cygwin provides some Linux-like tools, which can be valuable for working with the repository,
    such as the BASH shell and git packages
  - With appropriate adjustments, it is possible to use other shells and environments as well

- [Ninja on all platforms](https://github.com/ninja-build/ninja/releases)
- [The Ninja-build project](https://ninja-build.org)
- [Ninja Users Manual](https://ninja-build.org/manual.html)

- [QtCreator as IDE for CMake builds on all platforms](https://qt.io/download-open-source/#section-2)
