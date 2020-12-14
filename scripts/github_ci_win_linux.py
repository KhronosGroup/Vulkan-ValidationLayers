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

EXTERNAL_DIR_NAME = "external_linux"
BUILD_DIR_NAME = "build"

VERBOSE = False

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))
EXTERNAL_DIR = common_codegen.repo_relative(EXTERNAL_DIR_NAME)
VVL_BUILD_DIR = common_codegen.repo_relative(BUILD_DIR_NAME)
CONFIGURATIONS = ['release', 'debug']
DEFAULT_CONFIGURATION = CONFIGURATIONS[0]


#
#
# Check if the system is Windows
def is_windows():
    return 'windows' == platform.system().lower()

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

# Create build directory if it does not already exist
def CreateBuildDirectory(dir_path):
    if not os.path.exists(dir_path):
        os.makedirs(dir_path)

#
#
# Verify consistency of generated source code
def CheckVVLCodegenConsistency():
    gen_check_args = ['python3',
                      'scripts/generate_source.py',
                      '--verify',
                      '%s/Vulkan-Headers/registry' % EXTERNAL_DIR]
    print("XXXXX gen_check_args is ", gen_check_args)
    print("XXXXX BUILD_ROOT is ", PROJECT_ROOT)
    return subprocess.call(gen_check_args)

#
#
# Prepare the Validation Layers for testing
def BuildVVL(args):
    # Run update_deps.py for VVL repo
    update_cmd = 'python3 scripts/update_deps.py --dir %s --config %s --arch x64' % (EXTERNAL_DIR_NAME, args.configuration)
    command_output(ListArgs(update_cmd), PROJECT_ROOT)

    if CheckVVLCodegenConsistency():
        sys.exit(1)

    # Clone googletest repo into 'external' dir
    GTEST_DIR = common_codegen.repo_relative("external/googletest")
    if not os.path.exists(GTEST_DIR):
        clone_gtest_cmd = 'git clone https://github.com/google/googletest.git %s' % GTEST_DIR
        command_output(ListArgs(clone_gtest_cmd), PROJECT_ROOT)

        # Move to desired googletest tag
        gtest_checkout_cmd = 'git checkout tags/release-1.8.1'
        command_output(ListArgs(gtest_checkout_cmd), GTEST_DIR)

    CreateBuildDirectory(VVL_BUILD_DIR)

    # Run cmake for VVL
    cmake_cmd = 'cmake -C ../%s/helper.cmake -DCMAKE_BUILD_TYPE=%s -DUSE_CCACHE=ON ..' \
        % (EXTERNAL_DIR_NAME, args.configuration.capitalize())
    command_output(ListArgs(cmake_cmd), VVL_BUILD_DIR)

    # Build VVL
    os.chdir(VVL_BUILD_DIR)
    build_cmd = 'cmake --build . -- -j4'
    ret_code = subprocess.call(ListArgs(build_cmd))
    os.chdir(PROJECT_ROOT)
    if ret_code != 0:
        sys.exit(ret_code)
    return 0

#
#
# Prepare Loader for executing Layer Validation Tests
def BuildLoader(args):
    LOADER_DIR = common_codegen.repo_relative("%s/Vulkan-Loader" % EXTERNAL_DIR_NAME)
    # Clone Loader repo
    if not os.path.exists(LOADER_DIR):
        clone_loader_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Loader.git'
        command_output(ListArgs(clone_loader_cmd), EXTERNAL_DIR)

    # Run update_deps.py to get dependencies
    update_cmd = 'python3 scripts/update_deps.py --dir external'
    command_output(ListArgs(update_cmd), LOADER_DIR)

    # Create build directory
    LOADER_BUILD_DIR = common_codegen.repo_relative("%s/Vulkan-Loader/%s" % (EXTERNAL_DIR_NAME, BUILD_DIR_NAME))

    CreateBuildDirectory(LOADER_BUILD_DIR)

    # Run cmake
    cmake_cmd = 'cmake -C ../external/helper.cmake -DCMAKE_BUILD_TYPE=%s ..' % args.configuration.capitalize()
    command_output(ListArgs(cmake_cmd), LOADER_BUILD_DIR)

    # Build Loader
    os.chdir(LOADER_BUILD_DIR)
    build_cmd = 'cmake --build . -- -j4'
    ret_code = subprocess.call(ListArgs(build_cmd))
    os.chdir(PROJECT_ROOT)
    if ret_code != 0:
        sys.exit(ret_code)
    return 0

#
#
# Prepare Mock ICD for use with Layer Validation Tests
def BuildMockICD(args):
    # Clone Vulkan-Tools repo
    if not os.path.exists(common_codegen.repo_relative("%s/Vulkan-Tools" % EXTERNAL_DIR_NAME)):
        clone_tools_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Tools.git'
        command_output(ListArgs(clone_tools_cmd), EXTERNAL_DIR)

    ICD_BUILD_DIR = common_codegen.repo_relative("%s/Vulkan-Tools/%s" % (EXTERNAL_DIR_NAME,BUILD_DIR_NAME))

    CreateBuildDirectory(ICD_BUILD_DIR)

    # Run cmake
    cmake_args = ['cmake',
                  '-DCMAKE_BUILD_TYPE=%s' % args.configuration.capitalize(),
                  '-DBUILD_CUBE=NO',
                  '-DBUILD_VULKANINFO=NO',
                  '-DINSTALL_ICD=OFF',
                  '-DVULKAN_HEADERS_INSTALL_DIR=%s/Vulkan-Headers/%s/install' % (EXTERNAL_DIR, BUILD_DIR_NAME),
                  '..']
    cmake_cmd = \
        'cmake -DCMAKE_BUILD_TYPE=%s -DBUILD_CUBE=NO -DBUILD_VULKANINFO=NO -DINSTALL_ICD=OFF -DVULKAN_HEADERS_INSTALL_DIR=%s/Vulkan-Headers/%s/install ..' \
        % (args.configuration.capitalize(), EXTERNAL_DIR, BUILD_DIR_NAME)
    command_output(ListArgs(cmake_cmd), ICD_BUILD_DIR)
    
    VVL_REG_DIR = "%s/Vulkan-Headers/registry" % EXTERNAL_DIR
    VT_SCRIPTS_DIR = "%s/Vulkan-Tools/scripts" % EXTERNAL_DIR

    VT_ICD_DIR = "%s/Vulkan-Tools/icd/generated" % EXTERNAL_DIR
    LVL_GEN_SCRIPT = common_codegen.repo_relative("scripts/lvl_genvk.py")
    typemap_cmd = 'python3 %s -registry %s/vk.xml vk_typemap_helper.h' % (LVL_GEN_SCRIPT, VVL_REG_DIR)
    command_output(ListArgs(typemap_cmd), VT_ICD_DIR)

    KVT_GEN_SCRIPT = "%s/Vulkan-Tools/scripts/kvt_genvk.py" % EXTERNAL_DIR
    icd_cpp_cmd = 'python3 %s -registry %s/vk.xml mock_icd.cpp' % (KVT_GEN_SCRIPT, VVL_REG_DIR)
    command_output(ListArgs(icd_cpp_cmd), VT_ICD_DIR)

    icd_h_cmd = 'python3 %s -registry %s/vk.xml mock_icd.h' % (KVT_GEN_SCRIPT, VVL_REG_DIR)
    command_output(ListArgs(icd_h_cmd), VT_ICD_DIR)

    # Build ICD
    os.chdir(ICD_BUILD_DIR)
    build_cmd = 'cmake --build . --target VkICD_mock_icd -- -j4'
    ret_code = subprocess.call(ListArgs(build_cmd))
    os.chdir(PROJECT_ROOT)

    if ret_code != 0:
        sys.exit(ret_code)

    # Copy json file into dir with ICD executable
    src_filename = common_codegen.repo_relative("%s/Vulkan-Tools/icd/linux/VkICD_mock_icd.json" % EXTERNAL_DIR_NAME)
    dst_filename = common_codegen.repo_relative("%s/Vulkan-Tools/%s/icd/VkICD_mock_icd.json" % (EXTERNAL_DIR_NAME, BUILD_DIR_NAME))
    shutil.copyfile(src_filename, dst_filename)
    return 0

#
#
# Run the Layer Validation Tests
def RunVVLTests(args):
    os.chdir(PROJECT_ROOT)
    lvt_cmd = '%s/tests/vk_layer_validation_tests' % BUILD_DIR_NAME
    lvt_env = dict(os.environ)
    lvt_env['LD_LIBRARY_PATH'] = '%s/Vulkan-Loader/%s/loader' % (EXTERNAL_DIR, BUILD_DIR_NAME)
    lvt_env['VK_LAYER_PATH'] = '%s/%s/layers' % (PROJECT_ROOT, BUILD_DIR_NAME)
    lvt_env['VK_ICD_FILENAMES'] = '%s/Vulkan-Tools/%s/icd/VkICD_mock_icd.json' % (EXTERNAL_DIR, BUILD_DIR_NAME)
    ret_code = subprocess.call(ListArgs(lvt_cmd), env=lvt_env)
    if ret_code != 0:
        sys.exit(ret_code)

#
#
# Module Entrypoint
def main():
    parser = argparse.ArgumentParser(description='''Usage: python3 ./scripts/github_ci_win_linux.py
    - Reqires python3
    - Run script in repo root
    ''', formatter_class=RawDescriptionHelpFormatter)
    parser.add_argument(
        '-c', '--config', dest='configuration',
        metavar='CONFIG', action='store',
        choices=CONFIGURATIONS, default=DEFAULT_CONFIGURATION,
        help='Build target configuration. Can be one of: {0}'.format(
            ', '.join(CONFIGURATIONS)))
    args = parser.parse_args()

    if sys.version_info[0] != 3:
        print("This script requires Python 3. Run script with [-h] option for more details.")
        exit()

    failed = BuildVVL(args)
    if not failed:
        failed = BuildLoader(args)
    if not failed:
        failed = BuildMockICD(args)
    if not failed:
        failed = RunVVLTests(args)
    
    if failed:
        exit(1)
    else:
        exit(0)

if __name__ == '__main__':
  main()
