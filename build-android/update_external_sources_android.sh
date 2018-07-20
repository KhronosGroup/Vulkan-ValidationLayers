#!/bin/bash
# Update source for glslang, spirv-tools, shaderc

# Copyright 2016 The Android Open Source Project
# Copyright (C) 2015 Valve Corporation
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

set -e

ANDROIDBUILDDIR=$PWD
BUILDDIR=$ANDROIDBUILDDIR
BASEDIR=$BUILDDIR/third_party

if [[ $(uname) == "Linux" ]]; then
    cores="$(nproc || echo 4)"
elif [[ $(uname) == "Darwin" ]]; then
    cores=$(sysctl -n hw.ncpu)
fi

#
# Parse parameters
#

function printUsage {
   echo "Supported parameters are:"
   echo "    --abi <abi> (optional)"
   echo "    --no-build (optional)"
   echo
   echo "i.e. ${0##*/} --abi arm64-v8a \\"
   exit 1
}

while [[ $# -gt 0 ]]
do
    case $1 in
        --abi)
            abi="$2"
            shift 2
            ;;
        --no-build)
            nobuild=1
            shift 1
            ;;
        *)
            # unknown option
            echo Unknown option: $1
            echo
            printUsage
            exit 1
            ;;
    esac
done

echo abi=$abi
if [[ -z $abi ]]
then
    echo No abi provided, so building for all supported abis.
fi




echo no-build=$nobuild
if [[ $nobuild ]]
then
    echo Skipping build.
fi

function build_shaderc () {
   echo "Building $BASEDIR/shaderc"
   cd $BASEDIR/shaderc/android_test
   if [[ $abi ]]; then
      ndk-build NDK_APPLICATION_MK=../../../jni/shaderc/Application.mk THIRD_PARTY_PATH=../third_party APP_ABI=$abi -j $cores;
   else
      ndk-build NDK_APPLICATION_MK=../../../jni/shaderc/Application.mk THIRD_PARTY_PATH=../third_party -j $cores;
   fi
}

# Pull down or update external dependencies
echo "Update external dependencies based on the $ANDROIDBUILDDIR/known_good.json file"
python3 ../scripts/update_deps.py --no-build --dir $BASEDIR --known_good_dir $BUILDDIR

if [[ -z $nobuild ]]
then
build_shaderc
fi

echo ""
echo "${0##*/} finished."

