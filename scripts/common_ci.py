#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2017, 2019-2022 The Khronos Group Inc.
# Copyright (c) 2015-2017, 2019-2022 Valve Corporation
# Copyright (c) 2015-2017, 2019-2022 LunarG, Inc.
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
#
# Author: Mark Lobodzinski <mark@lunarg.com>

import os
import sys
import subprocess
import platform
import shutil
import argparse

import utils.utils as utils

if sys.version_info[0] != 3:
    print("This script requires Python 3. Run script with [-h] option for more details.")
    sys_exit(0)

# helper to define paths relative to the repo root
def RepoRelative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))

# TODO: Pass this in as arg, may be useful for running locally
EXTERNAL_DIR_NAME = "external"
BUILD_DIR_NAME = "build"
EXTERNAL_DIR = RepoRelative(EXTERNAL_DIR_NAME)
VVL_BUILD_DIR = RepoRelative(BUILD_DIR_NAME)
CONFIGURATIONS = ['release', 'debug']
DEFAULT_CONFIGURATION = CONFIGURATIONS[0]
ARCHS = [ 'x64', 'Win32' ]
DEFAULT_ARCH = ARCHS[0]

# Runs a command in a directory and returns its return code.
# Directory is project root by default, or a relative path from project root
def RunShellCmd(command, start_dir = PROJECT_ROOT, env=None, verbose=False):
    if start_dir != PROJECT_ROOT:
        start_dir = RepoRelative(start_dir)
    cmd_list = command.split(" ")
    if verbose or ('VVL_CI_VERBOSE' in os.environ and os.environ['VVL_CI_VERBOSE'] != '0'):
        print(f'CICMD({cmd_list}, env={env})')
    subprocess.check_call(cmd_list, cwd=start_dir, env=env)

#
# Check if the system is Windows
def IsWindows(): return 'windows' == platform.system().lower()

#
# Verify consistency of generated source code
def CheckVVLCodegenConsistency():
    print("Check Generated Source Code Consistency")
    gen_check_cmd = 'python3 scripts/generate_source.py --verify %s/Vulkan-Headers/registry %s/SPIRV-Headers/include/spirv/unified1/' % (EXTERNAL_DIR, EXTERNAL_DIR)
    RunShellCmd(gen_check_cmd)

#
# Prepare the Validation Layers for testing
def BuildVVL(args, build_tests=False):

    print("Log CMake version")
    cmake_ver_cmd = 'cmake --version'
    RunShellCmd(cmake_ver_cmd)

    utils.make_dirs(VVL_BUILD_DIR)
    print("Run CMake for Validation Layers")
    cmake_cmd = f'cmake -DUPDATE_DEPS=ON -DCMAKE_BUILD_TYPE={args.configuration.capitalize()} {args.cmake} ..'
    if IsWindows(): cmake_cmd = cmake_cmd + f' -A {args.arch}'
    if build_tests: cmake_cmd = cmake_cmd + ' -DBUILD_TESTS=ON'
    RunShellCmd(cmake_cmd, VVL_BUILD_DIR)

    print("Build Validation Layers and Tests")
    build_cmd = f'cmake --build . --config {args.configuration}'
    if not IsWindows(): build_cmd = build_cmd + f' -- -j{os.cpu_count()}'
    RunShellCmd(build_cmd, VVL_BUILD_DIR)

    print('Run vk_validation_stats.py')
    utils.make_dirs(os.path.join(VVL_BUILD_DIR, 'layers', args.configuration.capitalize()))
    RunShellCmd(f'python3 ../scripts/vk_validation_stats.py ../{EXTERNAL_DIR_NAME}/Vulkan-Headers/registry/validusage.json -text layers/{args.configuration.capitalize()}/vuid_coverage_database.txt', VVL_BUILD_DIR)

#
# Prepare Loader for executing Layer Validation Tests
def BuildLoader(args):
    LOADER_DIR = RepoRelative("%s/Vulkan-Loader" % EXTERNAL_DIR_NAME)
    # Clone Loader repo
    if not os.path.exists(LOADER_DIR):
        print("Clone Loader Source Code")
        clone_loader_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Loader.git'
        RunShellCmd(clone_loader_cmd, EXTERNAL_DIR)

    print("Run update_deps.py for Loader Repository")
    update_cmd = 'python3 scripts/update_deps.py --dir external'
    RunShellCmd(update_cmd, LOADER_DIR)

    print("Run CMake for Loader")
    LOADER_BUILD_DIR = RepoRelative("%s/Vulkan-Loader/%s" % (EXTERNAL_DIR_NAME, BUILD_DIR_NAME))
    utils.make_dirs(LOADER_BUILD_DIR)
    cmake_cmd = f'cmake -C ../external/helper.cmake -DCMAKE_BUILD_TYPE={args.configuration.capitalize()} {args.cmake} ..'
    if IsWindows(): cmake_cmd = cmake_cmd + f' -A {args.arch}'
    RunShellCmd(cmake_cmd, LOADER_BUILD_DIR)

    print("Build Loader")
    build_cmd = f'cmake --build . --config {args.configuration}'
    if not IsWindows(): build_cmd = build_cmd + f' -- -j{os.cpu_count()}'
    RunShellCmd(build_cmd, LOADER_BUILD_DIR)

#
# Prepare Mock ICD for use with Layer Validation Tests
def BuildMockICD(args):
    VT_DIR = RepoRelative("%s/Vulkan-Tools" % EXTERNAL_DIR_NAME)
    if not os.path.exists(VT_DIR):
        print("Clone Vulkan-Tools Repository")
        clone_tools_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Tools.git'
        RunShellCmd(clone_tools_cmd, EXTERNAL_DIR)

    ICD_BUILD_DIR = RepoRelative("%s/Vulkan-Tools/%s" % (EXTERNAL_DIR_NAME,BUILD_DIR_NAME))

    print("Running update_deps.py for ICD")
    RunShellCmd(f'python3 scripts/update_deps.py --dir {EXTERNAL_DIR_NAME} --config {args.configuration} --arch {args.arch}', VT_DIR)

    print("Run CMake for ICD")
    utils.make_dirs(ICD_BUILD_DIR)
    cmake_cmd = \
        f'cmake -DCMAKE_BUILD_TYPE={args.configuration.capitalize()} -DBUILD_CUBE=NO -DBUILD_VULKANINFO=NO -DINSTALL_ICD=OFF -DVULKAN_HEADERS_INSTALL_DIR={EXTERNAL_DIR}/Vulkan-Headers/{BUILD_DIR_NAME}/install {args.cmake} ..'
    RunShellCmd(cmake_cmd, ICD_BUILD_DIR)

    print("Build Mock ICD")
    build_cmd = f'cmake --build . --config {args.configuration}'
    if not IsWindows(): build_cmd = build_cmd + f' -- -j{os.cpu_count()}'
    RunShellCmd(build_cmd, ICD_BUILD_DIR)

#
# Run the Layer Validation Tests
def RunVVLTests(args):
    print("Run Vulkan-ValidationLayer Tests using Mock ICD")
    lvt_cmd = os.path.join(PROJECT_ROOT, BUILD_DIR_NAME, 'tests')
    if IsWindows(): lvt_cmd = os.path.join(lvt_cmd, args.configuration.capitalize())
    lvt_cmd = os.path.join(lvt_cmd, 'vk_layer_validation_tests')

    lvt_env = dict(os.environ)

    if not IsWindows():
        lvt_env['LD_LIBRARY_PATH'] = os.path.join(EXTERNAL_DIR, 'Vulkan-Loader', BUILD_DIR_NAME, 'loader')
    else:
        loader_dll = os.path.join(EXTERNAL_DIR, 'Vulkan-Loader', BUILD_DIR_NAME, 'loader', args.configuration.capitalize(), 'vulkan-1.dll')
        loader_dll_dst = os.path.join(os.path.dirname(lvt_cmd), 'vulkan-1.dll')
        shutil.copyfile(loader_dll, loader_dll_dst)

    layer_path = os.path.join(PROJECT_ROOT, BUILD_DIR_NAME, 'layers')
    if IsWindows(): layer_path = os.path.join(layer_path, args.configuration.capitalize())
    if not os.path.isdir(layer_path):
        raise Exception(f'VK_LAYER_PATH directory "{layer_path}" does not exist')
    lvt_env['VK_LAYER_PATH'] = layer_path

    icd_filenames = os.path.join(EXTERNAL_DIR, 'Vulkan-Tools', BUILD_DIR_NAME, 'icd')
    if IsWindows(): icd_filenames = os.path.join(icd_filenames, args.configuration.capitalize())
    icd_filenames = os.path.join(icd_filenames, 'VkICD_mock_icd.json')
    if not os.path.isfile(icd_filenames):
        raise Exception(f'VK_ICD_FILENAMES "{icd_filenames}" does not exist')
    lvt_env['VK_ICD_FILENAMES'] = icd_filenames
    # There is a problem in the github CI environment with the latest loader,
    # Probably because the only ICD available is the mock ICD. Disabling
    # physical device sorting avoids this problem.
    lvt_env['VK_LOADER_DISABLE_SELECT'] = '1'
    
    print("Environment variables set by script:")
    print(lvt_env)
    print("Pre-existing environment variables:")
    RunShellCmd("env")
    print("Running tests:")
    RunShellCmd(lvt_cmd, env=lvt_env)

def GetArgParser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-c', '--config', dest='configuration',
        metavar='CONFIG', action='store',
        choices=CONFIGURATIONS, default=DEFAULT_CONFIGURATION,
        help='Build target configuration. Can be one of: {0}'.format(
            ', '.join(CONFIGURATIONS)))
    parser.add_argument(
        '-a', '--arch', dest='arch',
        metavar='ARCH', action='store',
        choices=ARCHS, default=DEFAULT_ARCH,
        help=f'Target architecture. Can be one of: {ARCHS}')
    parser.add_argument(
        '--cmake', dest='cmake',
        metavar='CMAKE', type=str,
        default='', help='Additional args to pass to cmake')
    return parser
