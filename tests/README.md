# Validation Layers Tests

The Validation Layers use [Google Test (gtest)](https://github.com/google/googletest) for testing the
Validation Layers. This helps make sure all new changes are correct and prevent regressions.

The following is information how to setup and run the tests. There is seperate documentation for [creating tests](../docs/creating_tests.md).

## Building the tests

Make sure `-DBUILD_TESTS` was passed into the CMake command when generating the project

## Different Categories of tests

The tests are grouped into different categories. Some of the main test categories are:

- Negative testing
    - General tests that expect the Validation Layers to trigger an error
- Positive testing
    - Make sure Validation isn't accidentally triggering an error
    - Commonly created to prevent bug regressions

## Implicit Layers note

In general, the implicit validation layers may change the expected message output, which may cause some tests to fail. One solution is to disable the implicit layer during the test session using the `VK_LOADER_LAYERS_DISABLE` environment variable introduced in Vulkan Loader v.1.3.234.

Here is an example with OBS implicit layer:
```bash
export VK_LOADER_LAYERS_DISABLE=VK_LAYER_OBS_HOOK
```

## Running Test on Linux

**IMPORTANT** Make sure you have the correct `VK_LAYER_PATH` set on Linux

```bash
export VK_LAYER_PATH=/path/to/Vulkan-ValidationLayers/build/layers/
```

To run the tests

```bash
cd build

# Run all the test
./tests/vk_layer_validation_tests

# Run with certain VkPhysicalDevice
# see --help for more options
./tests/vk_layer_validation_tests --device-index 1

# Run a single test
./tests/vk_layer_validation_tests --gtest_filter=VkLayerTest.BufferExtents

# Run a multiple tests with a patter
./tests/vk_layer_validation_tests --gtest_filter=*Buffer*
```

## Running Test on Android

```bash
cd build-android

# Optional
adb uninstall com.example.VulkanLayerValidationTests

adb install -r -g --no-incremental bin/VulkanLayerValidationTests.apk

# Runs all test (And print the VUIDs)
adb shell am start -a android.intent.action.MAIN -c android-intent.category.LAUNCH -n com.example.VulkanLayerValidationTests/android.app.NativeActivity --es args --print-vu

# Run test with gtest_filter
adb shell am start -a android.intent.action.MAIN -c android-intent.category.LAUNCH -n com.example.VulkanLayerValidationTests/android.app.NativeActivity --es args --gtest_filter="*AndroidHardwareBuffer*"
```

To view to logging while running in a separate terminal run

```bash
adb logcat -c && adb logcat *:S VulkanLayerValidationTests
```

Or the files can be pulled off the device with

```bash
adb shell cat /sdcard/Android/data/com.example.VulkanLayerValidationTests/files/out.txt
adb shell cat /sdcard/Android/data/com.example.VulkanLayerValidationTests/files/err.txt
```

## Running Test on MacOS

Clone and build the [MoltenVK](https://github.com/KhronosGroup/MoltenVK) repository.

You will have to direct the loader from Vulkan-Loader to the MoltenVK ICD:

```bash
export VK_DRIVER_FILES=<path to MoltenVK repository>/Package/Latest/MoltenVK/macOS/MoltenVK_icd.json
```

## Running Tests on MockICD and Profiles layer

To allow a much higher coverage of testing the Validation Layers a test writer can use the Profile layer. [More details here](https://vulkan.lunarg.com/doc/view/1.3.204.1/windows/profiles_layer.html), but the main idea is the layer intercepts the Physical Device queries allowing testing of much more device properties. The Mock ICD is a "null driver" that is used to handle the Vulkan calls as the Validation Layers mostly only care about input "input" of the driver. If your tests relies on the "output" of the driver (such that a driver/implementation is correctly doing what it should do with valid input), then it might be worth looking into the [Vulkan CTS instead](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/vulkan_cts.md).

The Profile Layer can be found in the Vulkan SDK, otherwise, they will need to be cloned from [Vulkan Profiles](https://github.com/KhronosGroup/Vulkan-Profiles). The MockICD can be built from source in [Vulkan-Tools](https://github.com/KhronosGroup/Vulkan-Tools/tree/main/icd)

**NOTE**: While using the MockICD and Profiles layer the test will not be able to use Device Profiles API (`VK_LAYER_LUNARG_device_profile_api`) at the same time.
- If a feature is needed, it can be adjusted in the profile JSON
- Allowing both adds complexity due to the order the layers must be in, while adding little over value to test coverage

Here is an example of setting up and running the Profile layer with MockICD on a Linux environment
```bash
export VULKAN_SDK=/path/to/vulkansdk
export VVL=/path/to/Vulkan-ValidationLayers

# Add built Vulkan Validation Layers... remember it was Rule #1
export VK_LAYER_PATH=$VVL/build/layers/

# This step can be skipped if the Vulkan SDK is properly installed
# Add path for Profile Layer
export VK_LAYER_PATH=$VK_LAYER_PATH:$VULKAN_SDK/etc/vulkan/explicit_layer.d/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$VULKAN_SDK/lib/

# Set MockICD to be driver
export VK_DRIVER_FILES=/path/to/Vulkan-Tools/build/icd/VkICD_mock_icd.json
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/Vulkan-Tools/build/icd/

# Set Layers, the order here is VERY IMPORTANT otherwise the test will see the
# profile layer but the layer won't see it and have a different value
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation:VK_LAYER_KHRONOS_profiles

# Set the device profile to a valid json file
#    These are generated from https://github.com/KhronosGroup/Vulkan-Profiles
#    Also can found on https://vulkan.gpuinfo.org/ and downloaded from any device
# A set of profiles for CI are in the tests/device_profiles folder
export VK_KHRONOS_PROFILES_PROFILE_FILE=$VVL/tests/device_profiles/max_profile.json

# Expose all the parts of the profile layer
export VK_KHRONOS_PROFILES_SIMULATE_CAPABILITIES=SIMULATE_API_VERSION_BIT,SIMULATE_FEATURES_BIT,SIMULATE_PROPERTIES_BIT,SIMULATE_EXTENSIONS_BIT,SIMULATE_FORMATS_BIT,SIMULATE_QUEUE_FAMILY_PROPERTIES_BIT

# MockICD exposes VK_KHR_portability_subset but most tests are not testing for it
export VK_KHRONOS_PROFILES_EMULATE_PORTABILITY=false

# By default the Profile Layer logs warnings that mostly likely won't be useful for
# the usecase the Validation Layers tests are using it for
export VK_KHRONOS_PROFILES_DEBUG_REPORTS=DEBUG_REPORT_ERROR_BIT

# Running tests just as normal
$VVL/build/tests/vk_layer_validation_tests --gtest_filter=TestName
```

### Max Profile

`tests/device_profiles/max_profile.json` is a custom file designed to allow as much testing as possible.

It would be ideal to generate this similarly to the layer generated in [the Profile Layer repo](https://github.com/KhronosGroup/Vulkan-Profiles/blob/main/scripts/gen_profiles_layer.py), but the goal is to make sure as many tests are running in CI as possible.

This file will **never** fully cover all tests because some require certain properties/extension to be **both** enabled and disabled. The goal is to get as close to 100% coverage as possible.

### Address Sanitization (ASAN)

ASAN (Address Sanitization) has become a part of our CI process to ensure high quality code.

`-D VVL_ENABLE_ASAN=ON` will enable address sanitization in the build for GCC/Clang users. NOTE: This will only enable address sanitzation for validation layer code.

You could also set the needed compiler flags via environment variables:
```bash
export CFLAGS=-fsanitize=address
export CXXFLAGS=-fsanitize=address
export LDFLAGS=-fsanitize=address
```

- https://clang.llvm.org/docs/AddressSanitizer.html
- https://github.com/google/sanitizers/wiki/AddressSanitizer
- https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html

NOTE: `MSVC` offers thread sanitization but isn't yet part of our CI process.

### Thread Sanitization (TSAN)

TSAN (Thread Sanitization) has become a part of our CI process to detect data race bugs.

```bash
# NOTE: ThreadSanitizer generally requires all code to be compiled with -fsanitize=thread to prevent false positives.
# https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual#non-instrumented-code
export CFLAGS=-fsanitize=thread
export CXXFLAGS=-fsanitize=thread
export LDFLAGS=-fsanitize=thread
```

- https://clang.llvm.org/docs/ThreadSanitizer.html
- https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual
- https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html

NOTE: `MSVC` currently doesn't offer any form of thread sanitization.
