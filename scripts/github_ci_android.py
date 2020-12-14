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

# Script to determine if source code in Pull Request is properly formatted.

import os
import argparse
import shutil
import subprocess
import sys
import platform

import common_codegen

from subprocess import check_output
from datetime import date
from argparse import RawDescriptionHelpFormatter

os.system("")

VERBOSE = False
SUPPORTED_ABIS = [ 'arm64-v8a', 'armeabi-v7a']
DEFAULT_ABI = SUPPORTED_ABIS[0]
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))

# Split command lines into a list of args for python subprocess
def ListArgs(command_string):
    return command_string.split(" ")

# Runs a command in a directory and returns its standard output stream.
#    Captures the standard error stream and prints it if error.
#    Raises a RuntimeError if the command fails to launch or otherwise fails.
def command_output(cmd, directory):
    if VERBOSE:
        print('In {d}: {cmd}'.format(d=directory, cmd=cmd))
    p = subprocess.Popen(
        cmd, cwd=directory, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    #if p.returncode != 0:
        #print('*** Error ***\nstderr contents:\n{}'.format(stderr))
        #if not fail_ok:
        #    raise RuntimeError('Failed to run {} in {}'.format(cmd, directory))
    if VERBOSE:
        print(stdout)
    return p.returncode, stdout

#
# Fetch Android components, build Android VVL
def BuildAndroid(target_abi):

    #wget http://dl.google.com/android/repository/android-ndk-${{ matrix.config.ndk }}-linux-${{ matrix.config.arch }}.zip
    wget_cmd = 'wget http://dl.google.com/android/repository/android-ndk-r21d-linux-x86_64.zip'
    ret_code, out_text = command_output(ListArgs(wget_cmd), PROJECT_ROOT)
    print("wget retcode = ", ret_code)

    if ret_code == 0:
        #unzip -u -q android-ndk-${ANDROID_NDK}-linux-${ARCH}.zip
        unzip_cmd = 'unzip -u -q android-ndk-r21d-linux-x86_64.zip'
        ret_code, out_text = command_output(ListArgs(unzip_cmd), PROJECT_ROOT)
        # Add NDK to paths
        os.environ['ANDROID_NDK_HOME'] = common_codegen.repo_relative('android-ndk-r21d')
        os.environ['PATH'] = os.environ.get('ANDROID_NDK_HOME') + ':' + os.environ.get('PATH')
        print("unzip retcode = ", ret_code)

    if ret_code == 0:
        # Run Android update external sources script
        update_sources_cmd = './update_external_sources_android.sh --abi %s --no-build' % target_abi
        ret_code, out_text = command_output(ListArgs(update_sources_cmd), common_codegen.repo_relative("build-android"))
        print("Preparing Android Sources\n", out_text.decode())
        print("update retcode = ", ret_code)

    if ret_code == 0:
        # Build Android VVL and tests
        ndk_build_cmd = 'ndk-build APP_ABI=%s -j4' % target_abi
        ret_code, out_text = command_output(ListArgs(ndk_build_cmd), common_codegen.repo_relative("build-android"))
        print("Building Android Layers and Tests", out_text.decode())
        print("build retcode = ", ret_code)

    if ret_code != 0:
        sys.exit(ret_code)
    return 0

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

    failed = BuildAndroid(args.target_abi)

    if failed:
        exit(1)
    else:
        exit(0)

if __name__ == '__main__':
  main()
