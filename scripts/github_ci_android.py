#!/usr/bin/env python3
# Copyright (c) 2020 Valve Corporation
# Copyright (c) 2020 LunarG, Inc.

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
#
# Author: Mark Lobodzinski <mark@lunarg.com>

import os
import argparse
import shutil
import subprocess
import sys
import platform

import common_ci
from argparse import RawDescriptionHelpFormatter

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))
SUPPORTED_ABIS = [ 'arm64-v8a', 'armeabi-v7a']
DEFAULT_ABI = SUPPORTED_ABIS[0]

#
# Fetch Android components, build Android VVL
def BuildAndroid(target_abi):
    print("Fetching NDK\n")
    wget_cmd = 'wget http://dl.google.com/android/repository/android-ndk-r21d-linux-x86_64.zip'
    ret_code = common_ci.RunShellCmd(wget_cmd, PROJECT_ROOT)

    if ret_code == 0:
        print("Extracting NDK components\n")
        unzip_cmd = 'unzip -u -q android-ndk-r21d-linux-x86_64.zip'
        ret_code = common_ci.RunShellCmd(unzip_cmd, PROJECT_ROOT)
        # Add NDK to path
        os.environ['ANDROID_NDK_HOME'] = common_ci.repo_relative('android-ndk-r21d')
        os.environ['PATH'] = os.environ.get('ANDROID_NDK_HOME') + ':' + os.environ.get('PATH')

    if ret_code == 0:
        print("Preparing Android Dependencies\n")
        update_sources_cmd = './update_external_sources_android.sh --abi %s --no-build' % target_abi
        ret_code = common_ci.RunShellCmd(update_sources_cmd, common_ci.repo_relative("build-android"))

    if ret_code == 0:
        print("Building Android Layers and Tests\n")
        ndk_build_cmd = 'ndk-build APP_ABI=%s -j%s' % (target_abi, os.cpu_count())
        ret_code = common_ci.RunShellCmd(ndk_build_cmd, common_ci.repo_relative("build-android"))

    return ret_code

#
# Module Entrypoint
def main():
    parser = argparse.ArgumentParser(description='''Usage: python3 ./scripts/github_ci_android.py
    - Reqires python3
    - Run script in repo root
    ''', formatter_class=RawDescriptionHelpFormatter)
    parser.add_argument(
        '-a', '--abi', dest='target_abi',
        metavar='ABI', action='store',
        choices=SUPPORTED_ABIS, default=DEFAULT_ABI,
        help='Build target ABI. Can be one of: {0}'.format(
            ', '.join(SUPPORTED_ABIS)))
    args = parser.parse_args()

    if sys.version_info[0] != 3:
        print("This script requires Python 3. Run script with [-h] option for more details.")
        exit()

    ret_code = BuildAndroid(args.target_abi)
    if ret_code != 0:
        sys.exit(ret_code)
    else:
        sys.exit(0)

if __name__ == '__main__':
  main()
