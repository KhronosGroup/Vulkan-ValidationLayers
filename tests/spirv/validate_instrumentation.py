#!/usr/bin/env python3
#
# Copyright (c) 2024 LunarG, Inc.
#
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

# This scripts lives in testings as it is a way to run the instrumentation.exe
# against a large set of SPIR-V (example, https://github.com/LunarG/SPIRV-Database)

import os
import sys
import argparse
import tempfile
import subprocess

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='run instrumentation against a directory of SPIR-V files')
    parser.add_argument('--exe', action='store', type=str, help='path to instrumentation executable')
    parser.add_argument('--spirv-val', action='store', dest='spirv_val', type=str, help='Path to spirv-val to use')
    parser.add_argument('--shaders', action='store', required=True, type=str, help='path to directory with shaders')
    parser.add_argument('-v', '--verbose', action='store_true', help='print each shader')
    args = parser.parse_args()

    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))

    # default exe path
    exe_path = os.path.join(root_dir, 'build/tests/spirv/instrumentation')
    if args.exe:
        exe_path = args.exe
    if not os.path.isfile(exe_path):
        sys.exit("Cannot find instrumentation exe: " + exe_path)

    # default spirv-val path
    spirv_val_path = os.path.join(root_dir, 'external/SPIRV-Tools/build/tools/spirv-val')
    if args.spirv_val:
        spirv_val_path = args.spirv_val
    if not os.path.isfile(spirv_val_path):
        sys.exit("Cannot find instrumentation: " + spirv_val_path)

    if not os.path.isdir(args.shaders):
        sys.exit("Cannot find valid directory for shaders: " + args.shaders)

    # generate in temp directory so we can compare or copy later
    temp_obj = tempfile.TemporaryDirectory(prefix='vvl_spirv_instrumentation_')
    temp_file = os.path.join(temp_obj.name, "out.spv")

    for currentpath, folders, files in os.walk(args.shaders):
        for file in files:
            spirv_file = os.path.join(currentpath, file)

            # If spirv-val is failing before, it will fail later too
            exit_code = subprocess.call([spirv_val_path, spirv_file], stderr=subprocess.DEVNULL)
            if exit_code != 0:
                continue

            command = f'{exe_path} {spirv_file} -o {temp_file} --all-passes'
            if args.verbose:
                command += ' --timer'
                print(command)
            subprocess.call(command.split())
            exit_code = subprocess.call([spirv_val_path, temp_file])
            if exit_code != 0:
                print(f'spirv-val error for {spirv_file}')
