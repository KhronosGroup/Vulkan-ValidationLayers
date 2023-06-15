#!/bin/bash

# Copyright 2017 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if [ -z "${ANDROID_SDK_ROOT}" ];
then echo "Please set ANDROID_SDK_ROOT, exiting"; exit 1;
else echo "ANDROID_SDK_ROOT is ${ANDROID_SDK_ROOT}";
fi

if [ -z "${ANDROID_NDK_HOME}" ];
then echo "Please set ANDROID_NDK_HOME, exiting"; exit 1;
else echo "ANDROID_NDK_HOME is ${ANDROID_NDK_HOME}";
fi

if [[ $(uname) == "Linux" ]]; then
    cores=$(nproc) || echo 4
elif [[ $(uname) == "Darwin" ]]; then
    cores=$(sysctl -n hw.ncpu) || echo 4
fi

function findtool() {
    if [[ ! $(type -t $1) ]]; then
        echo Command $1 not found, see ../BUILD.md;
        exit 1;
    fi
}

# Check for dependencies
findtool aapt
findtool zipalign
findtool apksigner

set -ev

LAYER_BUILD_DIR=$PWD
echo LAYER_BUILD_DIR="${LAYER_BUILD_DIR}"

#
# Android builds and c++ libraries:
#
#   https://developer.android.com/ndk/guides/cpp-support recommends using
#   c++_shared for applications that use more than one shared library.
#   If multiple libraries using c++_static are loaded several copies of
#   the globals will be present in the C++ runtime. This also happens
#   if the same library is dlopen/dlclosed several times, as when running
#   the Layer Validation Tests. Some of the c++ runtime globals are
#   thread_local, so each copy consumes a TLS key. There are only 128 TLS
#   keys allowed on android, and the unit tests can hit this because of
#   repeatedly loading and unloading VVL.
#
#   The drawback to using c++shared is that the layer library can no longer
#   be installed manually, but must be installed in an APK. It is still
#   common practice to load layer libraries manually, so this script will
#   build the layer library using c++static by default.
#
# To build the layer libaries for running the layer validation tests on Android, c++_shared needs to be used for APP_STL. To do this set
# ANDROID_STL_TYPE to 'SHARED' in your environment
if [ -z "${ANDROID_STL_TYPE}" ]; then
    export ANDROID_STL_TYPE=STATIC
fi
echo "Building Layer using ${ANDROID_STL_TYPE}";

function create_APK() {
    aapt package -f -M AndroidManifest.xml -I "$ANDROID_SDK_ROOT/platforms/android-26/android.jar" -S res -F bin/$1-unaligned.apk bin/libs
    # If zipalign was run after signing, it won't be a valid signature
    zipalign -f 4 bin/$1-unaligned.apk bin/$1.apk
    # jarsigner used to be used until it was removed from the Android SDK
    apksigner sign --verbose --ks ~/.android/debug.keystore --ks-pass pass:android bin/$1.apk
}

#
# build layers
#
./update_external_sources_android.sh --no-build
ndk-build -j $cores

#
# build VulkanLayerValidationTests APK
#
mkdir -p bin/libs/lib
cp -r $LAYER_BUILD_DIR/libs/* $LAYER_BUILD_DIR/bin/libs/lib/
create_APK VulkanLayerValidationTests

echo Builds succeeded
exit 0
