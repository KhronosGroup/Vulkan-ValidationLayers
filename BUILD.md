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

    git clone https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers

## Building On Windows

### Windows Build Requirements

Windows 7+ with the following software packages:

- Microsoft Visual Studio 2013 Update 4 Professional, VS2015 (any version), or VS2017 (any version).
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
- Notes for using [Cygwin](https://www.cygwin.com)
  - First, in a Cygwin shell:
    - `./update_external_sources.sh --no-build`
  - Then, in a Visual Studio Developer Command Prompt:
    - Ensure python3.x and CMake in are in the path
    - Run `update_external_sources.bat --no-sync`
    - Run build_windows_targets.bat cmake

### Windows Build - Microsoft Visual Studio

1. Open a Developer Command Prompt for VS201x
2. Change directory to `Vulkan-LoaderAndValidationLayers` -- the root of the cloned git repository
3. Run `update_external_sources.bat` -- this will download and build external components
4. Create a `build` directory, change into that directory, and run cmake

For example, for VS2017 (generators for other versions are [specified here](#cmake-visual-studio-generators)):

    cmake -G "Visual Studio 15 2017 Win64" ..

This will create a Windows solution file named `VULKAN.sln` in the build directory.

Launch Visual Studio and open the "VULKAN.sln" solution file in the build folder.
You may select "Debug" or "Release" from the Solution Configurations drop-down list.
Start a build by selecting the Build->Build Solution menu item.
This solution copies the loader it built to each program's build directory
to ensure that the program uses the loader built from this solution.

#### The Update External Sources Batch File

Employing [optional parameters](#update-external-sources-optional-parameters)
to the **update_external_sources.bat** script can streamline repository set-up.

### Windows Tests and Demos

After making any changes to the repository, you should perform some quick sanity tests,
including the run_all_tests Powershell script and the cube demo with validation enabled.

To run the validation test script, open a Powershell Console,
change to the build/tests directory, and run:

For Release builds:

    .\run_all_tests.ps1

For Debug builds:

    .\run_all_tests.ps1 -Debug

This script will run the following tests:

- `vk_loader_validation_tests`:
  Vulkan loader handle wrapping, allocation callback, and loader/layer interface tests
- `vk_layer_validation_tests`:
  Test Vulkan validation layers
- `vkvalidatelayerdoc`:
  Tests that validation database is up-to-date and is synchronized with the validation source code

To run the Cube demo with validation in a Debug build configuration:

- In the MSVC solution explorer, right-click on the `cube` project and select
 `Set As Startup Project`
- Right click on cube again, select properties->Debugging->Command Arguments, change to
 `--validate`, and save
- From the main menu, select Debug->Start Debugging, or from the toolbar click
 `Local Windows Debugger`

Other demos that can be found in the build/demos directory are:

- `vulkaninfo`: Report GPU properties
- `smoketest`: A "smoke" test using more complex Vulkan rendering

### Windows Notes

#### CMake Visual Studio Generators

The above example used Visual Studio 2017, and specified its generator as "Visual Studio 15 2017 Win64".
The chosen generator should match your Visual Studio version. Appropriate Visual Studio generators include:

| Build Platform               | 64-bit Generator              | 32-bit Generator        |
|------------------------------|-------------------------------|-------------------------|
| Microsoft Visual Studio 2013 | "Visual Studio 12 2013 Win64" | "Visual Studio 12 2013" |
| Microsoft Visual Studio 2015 | "Visual Studio 14 2015 Win64" | "Visual Studio 14 2015" |
| Microsoft Visual Studio 2017 | "Visual Studio 15 2017 Win64" | "Visual Studio 15 2017" |

#### The Vulkan Loader Library

Vulkan programs must be able to find and use the vulkan-1.dll library.
While several of the test and demo projects in the Windows solution set this up automatically, doing so manually may be necessary for custom projects or solutions.
Make sure the library is either installed in the C:\Windows\System32 folder, or that the PATH environment variable includes the folder where the library resides.

To run Vulkan programs you must tell the Vulkan Loader where to find the libraries.
This is described in a `LoaderAndLayerInterface` document in the `loader` folder in this repository.
This describes both how ICDs and layers should be properly packaged, and how developers can point to ICDs and layers within their builds.

## Building On Linux

### Linux Build Requirements

This repository has been built and tested on the two most recent Ubuntu LTS versions.
Currently, the oldest supported version is Ubuntu 14.04, meaning that the minimum supported compiler versions are GCC 4.8.2 and Clang 3.4, although earlier versions may work.
It should be straightforward to adapt this repository to other Linux distributions.

**Required Package List:**

    sudo apt-get install git cmake build-essential libx11-xcb-dev libxkbcommon-dev libmirclient-dev libwayland-dev libxrandr-dev

### Linux Build

Example debug build (Note that the update\_external\_sources script used below builds external tools into predefined locations.
See **Loader and Validation Layer Dependencies** for more information and other options):

1. In a Linux terminal, `cd Vulkan-LoaderAndValidationLayers` -- the root of the
 cloned git repository
2. Execute `./update_external_sources.sh` -- this will download and build external components
3. Create a `build` directory, change into that directory, and run cmake:

        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Debug ..

4. Run `make -j8` to begin the build

If your build system supports ccache, you can enable that via CMake option `-DUSE_CCACHE=On`

#### The Update External Sources script

Employing [optional parameters](#update-external-sources-optional-parameters)
to the **update_external_sources.sh** script can streamline repository set-up.

#### Using the new loader and layers

    export LD_LIBRARY_PATH=<path to your repository root>/build/loader
    export VK_LAYER_PATH=<path to your repository root>/build/layers

You can run the `vulkaninfo` application to see which driver, loader and layers are being used.

The `LoaderAndLayerInterface` document in the `loader` folder in this repository
is a specification that describes both how ICDs and layers should be properly packaged,
and how developers can point to ICDs and layers within their builds.

### WSI Support Build Options

By default, the Vulkan Loader and Validation Layers are built with support for all 4 Vulkan-defined WSI display servers: Xcb, Xlib, Wayland, and Mir.
It is recommended to build the repository components with support for these display servers to maximize their usability across Linux platforms.
If it is necessary to build these modules without support for one of the display servers, the appropriate CMake option of the form `BUILD_WSI_xxx_SUPPORT` can be set to `OFF`.
See the top-level CMakeLists.txt file for more info.

### Linux Install to System Directories

Installing the files resulting from your build to the systems directories is optional since environment variables can usually be used instead to locate the binaries.
There are also risks with interfering with binaries installed by packages.
If you are certain that you would like to install your binaries to system directories, you can proceed with these instructions.

Assuming that you've built the code as described above and the current directory is still `build`, you can execute:

    sudo make install

This command installs files to:

- `/usr/local/include/vulkan`:  Vulkan include files
- `/usr/local/lib`:  Vulkan loader and layers shared objects
- `/usr/local/bin`:  vulkaninfo application
- `/usr/local/etc/vulkan/explicit_layer.d`:  Layer JSON files

You may need to run `ldconfig` in order to refresh the system loader search cache on some Linux systems.

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

Also see the `LoaderAndLayerInterface` document in the `loader` folder in this
repository for more information about loader operation.

Note that some executables in this repository (e.g., `cube`) use the "rpath" linker directive to
load the Vulkan loader from the build directory, `build` in this example.
This means that even after installing the loader to the system directories, these executables still
use the loader from the build directory.

### Linux Uninstall

To uninstall the files from the system directories, you can execute:

    sudo make uninstall

### Linux Tests and Demos

After making any changes to the repository, you should perform some quick sanity tests, including
the run_all_tests shell script and the cube demo with validation enabled.

To run the **validation test script**, in a terminal change to the build/tests directory and run:

    VK_LAYER_PATH=../layers ./run_all_tests.sh

This script will run the following tests:

- `vk_loader_validation_tests`: Tests Vulkan Loader handle wrapping
- `vk_layer_validation_tests`: Test Vulkan validation layers
- `vkvalidatelayerdoc`: Tests that validation database is in up-to-date and in synchronization with
  the validation source code

To run the **Cube demo** with validation, in a terminal change to the `build/demos`
directory and run:

    VK_LAYER_PATH=../layers ./cube --validate

Other demos that can be found in the `build/demos` directory are:

- `vulkaninfo`: report GPU properties
- `smoketest`: A "smoke" test using more complex Vulkan rendering

You can select which WSI subsystem is used to build the demos using a CMake option
called DEMOS_WSI_SELECTION.
Supported options are XCB (default), XLIB, WAYLAND, and MIR.
Note that you must build using the corresponding BUILD_WSI_*_SUPPORT enabled at the
base repository level (all SUPPORT options are ON by default).
For instance, creating a build that will use Xlib to build the demos,
your CMake command line might look like:

    cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DDEMOS_WSI_SELECTION=XLIB

### Linux Notes

#### Linux 32-bit support

Usage of this repository's contents in 32-bit Linux environments is not officially supported.
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

If the libraries in the `external` directory have already been built for 64-bit targets,
delete or "clean" this directory and rebuild it with the above settings using the
`update_external_sources` shell script.
This is required because the libraries in `external` must be built for 32-bit in order
to be usable by the rest of the components in the repository.

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

#### Run Cube and Smoke with Validation

Use the following steps to build, install, and run Cube and Smoke for Android:

    cd build-android
    ./build_all.sh
    adb install -r ../demos/android/cube/bin/cube.apk
    adb shell am start com.example.Cube/android.app.NativeActivity

To build, install, and run Cube with validation layers,
first build layers using steps above, then run:

    cd build-android
    ./build_all.sh
    adb install -r ../demos/android/cube-with-layers/bin/cube-with-layers.apk

##### Run without validation enabled

    adb shell am start com.example.CubeWithLayers/android.app.NativeActivity

##### Run with validation enabled

    adb shell am start -a android.intent.action.MAIN -c android-intent.category.LAUNCH -n com.example.CubeWithLayers/android.app.NativeActivity --es args "--validate"

To build, install, and run the Smoke demo for Android, run the following, and any prompts that come back from the script:

    ./update_external_sources.sh --glslang
    cd demos/smoke/android
    export ANDROID_SDK_HOME=<path to Android/Sdk>
    export ANDROID_NDK_HOME=<path to Android/Sdk/ndk-bundle>
    ./build-and-install
    adb shell am start -a android.intent.action.MAIN -c android-intent.category.LAUNCH -n com.example.Smoke/android.app.NativeActivity --es args "--validate"

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

Clone the Vulkan-LoaderAndValidationLayers repository:

    git clone https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers.git

### Get the External Libraries

Change to the cloned directory (`cd Vulkan-LoaderAndValidationLayers`) and run the script:

    ./update_external_sources.sh

This script downloads and builds the `glslang` and `MoltenVK` repositories.

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
        cmake -DCMAKE_BUILD_TYPE=Debug ..
        make

To speed up the build on a multi-core machine, use the `-j` option for `make`
to specify the number of cores to use for the build.
For example:

    make -j4

You can now run the demo applications from the command line:

    open demos/cube.app
    open demos/cubepp.app
    open demos/smoketest.app
    open demos/vulkaninfo.app

Or you can locate them from `Finder` and launch them from there.

##### The Install Target and RPATH

The applications you just built are "bundled applications", but the executables
are using the `RPATH` mechanism to locate runtime dependencies that are still
in your build tree.

To see this, run this command from your `build` directory:

    otool -l demos/cube.app/Contents/MacOS/cube

and note that the `cube` executable contains loader commands:

- `LC_LOAD_DYLIB` to load `libvulkan.1.dylib` via an `@rpath`
- `LC_RPATH` that contains an absolute path to the build location of the Vulkan loader

This makes the bundled application "non-transportable", meaning that it won't run
unless the Vulkan loader is on that specific absolute path.
This is useful for debugging the loader or other components built in this repository,
but not if you want to move the application to another machine or remove your build tree.

To address this problem, run:

    make install

This step "cleans up" the `RPATH` to remove any external references
and performs other bundle fix-ups.
After running `make install`, re-run the `otool` command again and note:

- `LC_LOAD_DYLIB` is now `@executable_path/../MacOS/libvulkan.1.dylib`
- `LC_RPATH` is no longer present

The "bundle fix-up" operation also puts a copy of the Vulkan loader into the bundle,
making the bundle completely self-contained and self-referencing.

Note that the "install" target has a very different meaning compared to the Linux
"make install" target.
The Linux "install" copies the targets to system directories.
In MacOS, "install" means fixing up application bundles.
In both cases, the "install" target operations clean up the `RPATH`.

##### The Non-bundled vulkaninfo Application

There is also a non-bundled version of the `vulkaninfo` application that you can
run from the command line:

    demos/vulkaninfo

If you run this before you run "make install", vulkaninfo's RPATH is already set
to point to the Vulkan loader in the build tree, so it has no trouble finding it.
But the loader will not find the MoltenVK driver and you'll see a message about an
incompatible driver.  To remedy this:

    VK_ICD_FILENAMES=../external/MoltenVK/Package/Latest/MoltenVK/macOS/MoltenVK_icd.json demos/vulkaninfo

If you run `vulkaninfo` after doing a "make install", the `RPATH` in the `vulkaninfo` application
got removed and the OS needs extra help to locate the Vulkan loader:

    DYLD_LIBRARY_PATH=loader VK_ICD_FILENAMES=../external/MoltenVK/Package/Latest/MoltenVK/macOS/MoltenVK_icd.json demos/vulkaninfo

#### Building with the Xcode Generator

To create and open an Xcode project:

        mkdir build-xcode
        cd build-xcode
        cmake -GXcode ..
        open VULKAN.xcodeproj

Within Xcode, you can select Debug or Release builds in the project's Build Settings.
You can also select individual schemes for working with specific applications like `cube`.

## Ninja Builds - All Platforms

The [Qt Creator IDE](https://qt.io/download-open-source/#section-2) can open a root CMakeList.txt
as a project directly, and it provides tools within Creator to configure and generate Vulkan SDK
build files for one to many targets concurrently.
Alternatively, when invoking CMake, use the `-G "Codeblocks - Ninja"` option to generate Ninja build
files to be used as project files for QtCreator

- Follow the steps defined elsewhere for the OS using the update\_external\_sources script or as
  shown in **Loader and Validation Layer Dependencies** below
- Open, configure, and build the glslang CMakeList.txt files. Note that building the glslang
  project will provide access to spirv-tools and spirv-headers
- Then do the same with the Vulkan-LoaderAndValidationLayers CMakeList.txt file
- In order to debug with QtCreator, a
  [Microsoft WDK: eg WDK 10](http://go.microsoft.com/fwlink/p/?LinkId=526733) is required.

Note that installing the WDK breaks the MSVC vcvarsall.bat build scripts provided by MSVC,
requiring that the LIB, INCLUDE, and PATHenv variables be set to the WDK paths by some other means

## Update External Sources Optional Parameters

This script will default to building 64-bit _and_ 32-bit versions of debug _and_ release
configurations, which can take a substantial amount of time.
However, it supports the following options to select a particular build configuration which can
reduce the time needed for repository set-up:

| Command Line Option  |  Function                                    |
|----------------------|----------------------------------------------|
|   --32               | Build 32-bit targets only                    |
|   --64               | Build 64-bit targets only                    |
|   --release          | Perform release builds only                  |
|   --debug            | Perform debug builds only                    |
|   --no-build         | Sync without building targets                |
|   --no-sync          | Skip repository sync step                    |

For example, to target a Windows 64-bit debug development configuration, invoke the batch file as follows:

`update_external_sources.bat --64 --debug`

Similarly, invoking the same configuration for Linux would be:

`update_external_sources.sh --64 --debug`

## Loader and Validation Layer Dependencies

The glslang repository is required to build and run Loader and Validation Layer components.
It is not a git sub-module of Vulkan-LoaderAndValidationLayers but Vulkan-LoaderAndValidationLayers
is linked to a specific revision of glslang.
This can be automatically cloned and built to predefined locations with the
`update_external_sources` scripts.
If a custom configuration is required, do the following steps:

1) clone the repository:

    `git clone https://github.com/KhronosGroup/glslang.git`

2) checkout the correct version of the tree based on the contents of the
glslang\_revision file at the root of the Vulkan-LoaderAndValidationLayers tree
(do the same anytime that Vulkan-LoaderAndValidationLayers is updated from remote)

    - On Windows

    ```script
        git checkout < [path to Vulkan-LoaderAndValidationLayers]\glslang_revision [in glslang repo]
    ```

    - Non Windows

    ```script
        git checkout `cat [path to Vulkan-LoaderAndValidationLayers]\glslang_revision` [in glslang repo]
    ```

3) Configure the glslang source tree with CMake and build it with your IDE of choice

4) Enable the `CUSTOM_GLSLANG_BIN_PATH` and `CUSTOM_SPIRV_TOOLS_BIN_PATH` options in the Vulkan-LoaderAndValidationLayers
   CMake configuration and point the `GLSLANG_BINARY_PATH`  and `SPIRV_TOOLS_BINARY_PATH` variables to the correct location

5) If building on Windows with MSVC, set `DISABLE_BUILDTGT_DIR_DECORATION` to _On_.
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
