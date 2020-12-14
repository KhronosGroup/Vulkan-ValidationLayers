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

EXTERNAL_DIR_NAME = "external_linux"
BUILD_DIR_NAME = "build"
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))
EXTERNAL_DIR = common_ci.repo_relative(EXTERNAL_DIR_NAME)
VVL_BUILD_DIR = common_ci.repo_relative(BUILD_DIR_NAME)
CONFIGURATIONS = ['release', 'debug']
DEFAULT_CONFIGURATION = CONFIGURATIONS[0]

#
# Check if the system is Windows
def is_windows():
    return 'windows' == platform.system().lower()

#
# Create build directory if it does not already exist
def CreateBuildDirectory(dir_path):
    if not os.path.exists(dir_path):
        os.makedirs(dir_path)

#
# Verify consistency of generated source code
def CheckVVLCodegenConsistency():
    print("Check Generated Source Code Consistency")
    gen_check_cmd = 'python3 scripts/generate_source.py --verify %s/Vulkan-Headers/registry' % EXTERNAL_DIR
    ret_code = common_ci.RunShellCmd(gen_check_cmd, PROJECT_ROOT)
    return ret_code

#
# Prepare the Validation Layers for testing
def BuildVVL(args):

    print("Log CMake version")
    cmake_ver_cmd = 'cmake --version'
    common_ci.RunShellCmd(cmake_ver_cmd, PROJECT_ROOT)

    print("Run update_deps.py for VVL Repository")
    update_cmd = 'python3 scripts/update_deps.py --dir %s --config %s --arch x64' % (EXTERNAL_DIR_NAME, args.configuration)
    ret_code = common_ci.RunShellCmd(update_cmd, PROJECT_ROOT)

    if ret_code == 0:
        GTEST_DIR = common_ci.repo_relative("external/googletest")
        if not os.path.exists(GTEST_DIR):
            print("Clone Testing Framework Source Code")
            clone_gtest_cmd = 'git clone https://github.com/google/googletest.git %s' % GTEST_DIR
            ret_code = common_ci.RunShellCmd(clone_gtest_cmd, PROJECT_ROOT)
            
            if ret_code == 0:
                print("Get Specified Testing Source")
                gtest_checkout_cmd = 'git checkout tags/release-1.8.1'
                common_ci.RunShellCmd(gtest_checkout_cmd, GTEST_DIR)

    if ret_code == 0:
        CreateBuildDirectory(VVL_BUILD_DIR)
        print("Run CMake for Validation Layers")
        cmake_cmd = 'cmake -C ../%s/helper.cmake -DCMAKE_BUILD_TYPE=%s -DUSE_CCACHE=ON ..' \
            % (EXTERNAL_DIR_NAME, args.configuration.capitalize())
        ret_code = common_ci.RunShellCmd(cmake_cmd, VVL_BUILD_DIR)

    if ret_code == 0:
        print("Build Validation Layers and Tests")
        os.chdir(VVL_BUILD_DIR)
        build_cmd = 'cmake --build . -- -j%s' % os.cpu_count()
        ret_code = common_ci.RunShellCmd(build_cmd, VVL_BUILD_DIR)

    return ret_code

#
# Prepare Loader for executing Layer Validation Tests
def BuildLoader(args):
    LOADER_DIR = common_ci.repo_relative("%s/Vulkan-Loader" % EXTERNAL_DIR_NAME)
    ret_code = 0
    # Clone Loader repo
    if not os.path.exists(LOADER_DIR):
        print("Clone Loader Source Code")
        clone_loader_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Loader.git'
        ret_code = common_ci.RunShellCmd(clone_loader_cmd, EXTERNAL_DIR)

    if ret_code == 0:
        print("Run update_deps.py for Loader Repository")
        update_cmd = 'python3 scripts/update_deps.py --dir external'
        ret_code = common_ci.RunShellCmd(update_cmd, LOADER_DIR)
       
    if ret_code == 0:
        print("Run CMake for Loader")
        LOADER_BUILD_DIR = common_ci.repo_relative("%s/Vulkan-Loader/%s" % (EXTERNAL_DIR_NAME, BUILD_DIR_NAME))
        CreateBuildDirectory(LOADER_BUILD_DIR)
        cmake_cmd = 'cmake -C ../external/helper.cmake -DCMAKE_BUILD_TYPE=%s ..' % args.configuration.capitalize()
        ret_code = common_ci.RunShellCmd(cmake_cmd, LOADER_BUILD_DIR)
       
    if ret_code == 0:
        print("Build Loader")
        build_cmd = 'cmake --build . -- -j%s' % os.cpu_count()
        ret_code = common_ci.RunShellCmd(build_cmd, LOADER_BUILD_DIR)

    return ret_code

#
# Prepare Mock ICD for use with Layer Validation Tests
def BuildMockICD(args):
    ret_code = 0
    if not os.path.exists(common_ci.repo_relative("%s/Vulkan-Tools" % EXTERNAL_DIR_NAME)):
        print("Clone Vulkan-Tools Repository")
        clone_tools_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Tools.git'
        ret_code = common_ci.RunShellCmd(clone_tools_cmd, EXTERNAL_DIR)

    if ret_code == 0:
        print("Run CMake for ICD")
        ICD_BUILD_DIR = common_ci.repo_relative("%s/Vulkan-Tools/%s" % (EXTERNAL_DIR_NAME,BUILD_DIR_NAME))
        CreateBuildDirectory(ICD_BUILD_DIR)
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
        ret_code = common_ci.RunShellCmd(cmake_cmd, ICD_BUILD_DIR)

    if ret_code == 0:
        VVL_REG_DIR = "%s/Vulkan-Headers/registry" % EXTERNAL_DIR
        VT_SCRIPTS_DIR = "%s/Vulkan-Tools/scripts" % EXTERNAL_DIR

        print ("Geneating ICD Source Code")
        VT_ICD_DIR = "%s/Vulkan-Tools/icd/generated" % EXTERNAL_DIR
        LVL_GEN_SCRIPT = common_ci.repo_relative("scripts/lvl_genvk.py")
        typemap_cmd = 'python3 %s -registry %s/vk.xml vk_typemap_helper.h' % (LVL_GEN_SCRIPT, VVL_REG_DIR)
        typemap_ret_code = common_ci.RunShellCmd(typemap_cmd, VT_ICD_DIR)

        KVT_GEN_SCRIPT = "%s/Vulkan-Tools/scripts/kvt_genvk.py" % EXTERNAL_DIR
        icd_cpp_cmd = 'python3 %s -registry %s/vk.xml mock_icd.cpp' % (KVT_GEN_SCRIPT, VVL_REG_DIR)
        icd_cpp_ret_code = common_ci.RunShellCmd(icd_cpp_cmd, VT_ICD_DIR)

        icd_h_cmd = 'python3 %s -registry %s/vk.xml mock_icd.h' % (KVT_GEN_SCRIPT, VVL_REG_DIR)
        icd_h_ret_code = common_ci.RunShellCmd(icd_h_cmd, VT_ICD_DIR)

        if typemap_ret_code != 0  or icd_cpp_ret_code != 0 or icd_h_ret_code != 0:
            ret_code = 1

    if ret_code == 0:
        print("Build Mock ICD")
        build_cmd = 'cmake --build . --target VkICD_mock_icd -- -j%s' % os.cpu_count()
        ret_code = common_ci.RunShellCmd(build_cmd, ICD_BUILD_DIR)

        # Copy json file into dir with ICD executable
        src_filename = common_ci.repo_relative("%s/Vulkan-Tools/icd/linux/VkICD_mock_icd.json" % EXTERNAL_DIR_NAME)
        dst_filename = common_ci.repo_relative("%s/Vulkan-Tools/%s/icd/VkICD_mock_icd.json" % (EXTERNAL_DIR_NAME, BUILD_DIR_NAME))
        shutil.copyfile(src_filename, dst_filename)

    return ret_code

#
# Run the Layer Validation Tests
def RunVVLTests(args):
    print("Run Vulkan-ValidationLayer Tests using Mock ICD")
    os.chdir(PROJECT_ROOT)
    lvt_cmd = '%s/tests/vk_layer_validation_tests' % BUILD_DIR_NAME
    lvt_env = dict(os.environ)
    lvt_env['LD_LIBRARY_PATH'] = '%s/Vulkan-Loader/%s/loader' % (EXTERNAL_DIR, BUILD_DIR_NAME)
    lvt_env['VK_LAYER_PATH'] = '%s/%s/layers' % (PROJECT_ROOT, BUILD_DIR_NAME)
    lvt_env['VK_ICD_FILENAMES'] = '%s/Vulkan-Tools/%s/icd/VkICD_mock_icd.json' % (EXTERNAL_DIR, BUILD_DIR_NAME)
    ret_code = subprocess.call(lvt_cmd.split(" "), env=lvt_env)

    return ret_code

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
        sys_exit(0)

    ret_code = BuildVVL(args)
    if ret_code == 0:
        ret_code = BuildLoader(args)
    if ret_code == 0:
        ret_code = BuildMockICD(args)
    if ret_code == 0:
        ret_code = RunVVLTests(args)

    # Run codgen checks regardless of previous failures
    inconsistent_source = CheckVVLCodegenConsistency()

    if ret_code != 0 or inconsistent_source != 0:
        if ret_code != 0:
            sys.exit(ret_code)
        else:
            sys.exit(inconsistent_source)
    else:
        exit(0)

if __name__ == '__main__':
  main()
