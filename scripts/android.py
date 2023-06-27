#!/usr/bin/env python
# Copyright (c) 2023 Valve Corporation
# Copyright (c) 2023 LunarG, Inc.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import json
import os
import sys
import shutil
import common_ci

# TODO: Find a more robust way to get the host_tag.
def get_host_tag():
    if sys.platform.startswith('linux'):
        return 'linux-x86_64'
    elif sys.platform == 'darwin':
        return 'darwin-x86_64'
    elif sys.platform == 'win32' or sys.platform == 'cygwin':
        return 'windows-x86_64'
    sys.exit('Unsupported platform: ' + sys.platform)

# NOTE: Android this documentation is crucial for understanding the layout of the NDK.
# https://android.googlesource.com/platform/ndk/+/master/docs/BuildSystemMaintainers.md

# TODO: Use CMake from Android SDK?

# TODO: Handle both c++_shared and c++_static for CMAKE_ANDROID_STL_TYPE

# TODO: Lots of the logic is hardcoded to Android API 26. Including external files like `AndroidManifest.xml`.

# Android APKs can contain binaries for multiple ABIs (armeabi-v7a, arm64-v8a, x86, x86_64).
# CMake will need to be run multiple times to create a complete test APK that can run on any Android device.
def main():
    # Environment variables we can rely on users/environments setting.
    # https://github.com/actions/runner-images/blob/main/images/linux/Ubuntu2204-Readme.md#environment-variables-2
    if "ANDROID_NDK_HOME" not in os.environ:
        print("Cannot find ANDROID_NDK_HOME!")
        sys.exit(1)
    elif "ANDROID_SDK_ROOT" not in os.environ:
        print("Cannot find ANDROID_SDK_ROOT!")
        sys.exit(1)

    android_ndk_home = os.environ.get('ANDROID_NDK_HOME')
    android_sdk_root = os.environ.get('ANDROID_SDK_ROOT')
    android_toolchain = android_ndk_home + '/build/cmake/android.toolchain.cmake'

    print(f"ANDROID_NDK_HOME = {android_ndk_home}")
    print(f"ANDROID_SDK_ROOT = {android_sdk_root}")
    print(f'CMAKE_TOOLCHAIN_FILE = {android_toolchain}')

    # TODO: Verify aapt, ndk-build, cmake, ninja are in the path.

    # TODO: Make ABI a cli argument.
    # android_abis = ["armeabi-v7a", "arm64-v8a", "x86", "x86_64"]
    android_abis = [ "arm64-v8a" ]

    apk_dir = common_ci.RepoRelative(f'build-android/bin')
    cmake_install_dir = common_ci.RepoRelative(f'build-android/bin/libs')

    # NOTE: I'm trying to roughly match what build-android/build_all.sh currently does.
    for abi in android_abis:
        build_dir = common_ci.RepoRelative(f'build-android/obj/{abi}')
        lib_dir = f'lib/{abi}'

        # Delete CMakeCache.txt to ensure clean builds
        # NOTE: CMake 3.24 has --fresh which would be better to use in the future.
        cmake_cache = build_dir + '/CMakeCache.txt'
        if os.path.isfile(cmake_cache):
            os.remove(cmake_cache)

        cmake_cmd =  f'cmake -S . -B {build_dir} -G Ninja'

        # TODO: Allow user to change config
        cmake_cmd += f' -D CMAKE_BUILD_TYPE=Release'

        cmake_cmd += f' -D UPDATE_DEPS=ON -D UPDATE_DEPS_DIR={build_dir}'
        cmake_cmd += f' -D CMAKE_TOOLCHAIN_FILE={android_toolchain}'

        cmake_cmd += f' -D ANDROID_PLATFORM=26'
        cmake_cmd += f' -D CMAKE_ANDROID_ARCH_ABI={abi}'
        cmake_cmd += f' -D CMAKE_ANDROID_RTTI=YES'
        cmake_cmd += f' -D CMAKE_ANDROID_EXCEPTIONS=YES'
        cmake_cmd += f' -D ANDROID_USE_LEGACY_TOOLCHAIN_FILE=NO'

        cmake_cmd += f' -D CMAKE_INSTALL_LIBDIR={lib_dir}'

        cmake_cmd += f' -D BUILD_TESTS=ON'
        cmake_cmd += f' -D CMAKE_ANDROID_STL_TYPE=c++_shared'

        common_ci.RunShellCmd(cmake_cmd)

        build_cmd = f'cmake --build {build_dir}'
        common_ci.RunShellCmd(build_cmd)

        install_cmd = f'cmake --install {build_dir} --prefix {cmake_install_dir}'
        common_ci.RunShellCmd(install_cmd)

        # https://android.googlesource.com/platform/ndk/+/master/docs/BuildSystemMaintainers.md#architectures
        # "abis.json" lets us get the triple based on the abi.
        with open(android_ndk_home + "/meta/abis.json") as f:
            abis_json = json.load(f)

        host_tag = get_host_tag()

        triple = abis_json[abi]['triple']
        # "https://android.googlesource.com/platform/ndk/+/master/docs/BuildSystemMaintainers.md#sysroot"
        # The Android sysroot is installed to <NDK>/toolchains/llvm/prebuilt/<host-tag>/sysroot
        sysroot = f'{android_ndk_home}/toolchains/llvm/prebuilt/{host_tag}/sysroot'
        if not os.path.isdir(sysroot):
            print("Unable to find sysroot!")
            print('NDK = {android_ndk_home}')
            print('HOST-TAG = {host_tag}')
            sys.exit(-1)

        # https://android.googlesource.com/platform/ndk/+/master/docs/BuildSystemMaintainers.md#STL
        # "If using the shared variant, libc++_shared.so must be included in the APK. This library is installed to <NDK>/sysroot/usr/lib/<triple>."
        src_shared_stl = f'{sysroot}/usr/lib/{triple}/libc++_shared.so'
        if not os.path.isfile(src_shared_stl):
            print("Unable to find libc++_shared.so!")
            print('Triple = {triple}')
            sys.exit(-1)

        if not os.path.isfile(f'{cmake_install_dir}/{lib_dir}/libVulkanLayerValidationTests.so'):
            print("Unable to find tests!")
            sys.exit(-1)

        dst_shared_stl = f'{cmake_install_dir}/{lib_dir}/libc++_shared.so'

        shutil.copyfile(src_shared_stl, dst_shared_stl)
        print(f'Copied {src_shared_stl} to {dst_shared_stl}')

    # The following are CLI instructions I plan to automate with this script.
    # The main problem I'm having with Android is the steps after the CMake build.
    #
    # IE generating the APK and generating binaries for multiple platforms.
    # CMake only handles 1 toolchain per build. As a result we need to run CMake for each ISA Android supports.
    # Then we need to create an APK with those binaries.
    # However, we only need the APK for testing. For our Android releases we just put our `.so` files for each platform. No APK drama.
    android_manifest = common_ci.RepoRelative('build-android/AndroidManifest.xml')

    android_jar = android_sdk_root + "/platforms/android-26/android.jar"

    android_res = common_ci.RepoRelative('build-android/res')

    apk_name = 'VulkanLayerValidationTests'

    unaligned_apk = f'{apk_dir}/{apk_name}-unaligned.apk'
    aligned_apk = f'{apk_dir}/{apk_name}.apk'

    # Create APK
    common_ci.RunShellCmd(f'aapt package -f -M {android_manifest} -I {android_jar} -S {android_res} -F {unaligned_apk} {cmake_install_dir}')

    # Align APK
    common_ci.RunShellCmd(f'zipalign -f 4 {unaligned_apk} {aligned_apk}')

    # Create Key (If it doesn't already exist)
    debug_key = common_ci.RepoRelative('build-android/obj/debug.keystore')
    ks_pass = 'android'
    if not os.path.isfile(debug_key):
        dname = 'CN=Android-Debug,O=Android,C=US'
        common_ci.RunShellCmd(f'keytool -genkey -v -keystore {debug_key} -alias androiddebugkey -storepass {ks_pass} -keypass {ks_pass} -keyalg RSA -keysize 2048 -validity 10000 -dname {dname}')

    # Sign APK
    common_ci.RunShellCmd(f'apksigner sign --verbose --ks {debug_key} --ks-pass pass:{ks_pass} {aligned_apk}')

if __name__ == '__main__':
    main()
