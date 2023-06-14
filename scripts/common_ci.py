#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2017, 2019-2023 The Khronos Group Inc.
# Copyright (c) 2015-2017, 2019-2023 Valve Corporation
# Copyright (c) 2015-2017, 2019-2023 LunarG, Inc.
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

import os
import sys
import subprocess
import platform
import shutil
import argparse

if sys.version_info[0] != 3:
    print("This script requires Python 3. Run script with [-h] option for more details.")
    sys_exit(0)

# Use Ninja for all platforms for performance/simplicity
os.environ['CMAKE_GENERATOR'] = "Ninja"

# Utility for creating a directory if it does not exist. Behaves similarly to 'mkdir -p'
def make_dirs(path, clean=False):
    if clean and os.path.isdir(path):
        shutil.rmtree(path)
    os.makedirs(path, exist_ok=True)

# helper to define paths relative to the repo root
def RepoRelative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))

# TODO: Pass this in as arg, may be useful for running locally
EXTERNAL_DIR_NAME = "external"
BUILD_DIR_NAME = "build"
VVL_BUILD_DIR = RepoRelative(BUILD_DIR_NAME)
TEST_INSTALL_DIR = RepoRelative("build/install")

def externalDir(config): return os.path.join(RepoRelative(EXTERNAL_DIR_NAME), config)

# Returns true if we are running in GitHub actions
# https://docs.github.com/en/actions/learn-github-actions/variables#default-environment-variables
def IsGHA(): 
    if 'GITHUB_ACTION' in os.environ:
        return True
    return False

# Runs a command in a directory and returns its return code.
# Directory is project root by default, or a relative path from project root
def RunShellCmd(command, start_dir = PROJECT_ROOT, env=None, verbose=False):
    if start_dir != PROJECT_ROOT:
        start_dir = RepoRelative(start_dir)
    cmd_list = command.split(" ")

    # Helps a lot when debugging CI issues
    if IsGHA():
        verbose = True

    if verbose:
        print(f'CICMD({cmd_list}, env={env})')
    subprocess.check_call(cmd_list, cwd=start_dir, env=env)

#
# Check if the system is Windows
def IsWindows(): return 'windows' == platform.system().lower()

#
# Set MACOSX_DEPLOYMENT_TARGET
def SetupDarwin(osx):
    if platform.system() != "Darwin":
        return

    # By default it will use the latest MacOS SDK available on the system.
    if osx == 'latest':
        return

    # Currently the Vulkan SDK targets 10.15 as the minimum for MacOS support.
    # If we need to we can raise the minimim like we did for C++17 support.
    os.environ['MACOSX_DEPLOYMENT_TARGET'] = "10.15"
    print(f"Targeting {os.environ['MACOSX_DEPLOYMENT_TARGET']} MacOS Deployment Target", flush=True)

#
# Run VVL scripts
def CheckVVL(config):
    ext_dir = externalDir(config)
    vulkan_registry = ext_dir + "/Vulkan-Headers/registry"
    spirv_unified = ext_dir + "/SPIRV-Headers/include/spirv/unified1/"

    # Verify consistency of generated source code
    print("Check Generated Source Code Consistency")
    gen_check_cmd = f'python scripts/generate_source.py --verify {vulkan_registry} {spirv_unified}'
    RunShellCmd(gen_check_cmd)

    print('Run vk_validation_stats.py')
    valid_usage_json = vulkan_registry + "/validusage.json"
    text_file = RepoRelative(f'{VVL_BUILD_DIR}/layers/vuid_coverage_database.txt')
    gen_check_cmd = f'python scripts/vk_validation_stats.py {valid_usage_json} -text {text_file}'
    RunShellCmd(gen_check_cmd)

#
# Prepare the Validation Layers for testing
def BuildVVL(config, cmake_args, build_tests):
    print("Log CMake version")
    cmake_ver_cmd = 'cmake --version'
    RunShellCmd(cmake_ver_cmd)

    print("Run CMake for Validation Layers")
    cmake_cmd = f'cmake -S . -B {VVL_BUILD_DIR} -DUPDATE_DEPS=ON -DCMAKE_BUILD_TYPE={config}'
    # By default BUILD_WERROR is OFF, CI should always enable it.
    cmake_cmd += ' -DBUILD_WERROR=ON'
    cmake_cmd += f' -DBUILD_TESTS={build_tests}'

    if cmake_args:
         cmake_cmd += f' {cmake_args}'

    RunShellCmd(cmake_cmd)

    print("Build Validation Layers and Tests")
    build_cmd = f'cmake --build {VVL_BUILD_DIR}'
    RunShellCmd(build_cmd)

    print("Install Validation Layers")
    install_cmd = f'cmake --install {VVL_BUILD_DIR} --prefix {TEST_INSTALL_DIR}'
    RunShellCmd(install_cmd)

#
# Prepare Loader for executing Layer Validation Tests
def BuildLoader():
    LOADER_DIR = RepoRelative(os.path.join("%s/Vulkan-Loader" % EXTERNAL_DIR_NAME))
    # Clone Loader repo
    if not os.path.exists(LOADER_DIR):
        print("Clone Loader Source Code")
        clone_loader_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Loader.git'
        RunShellCmd(clone_loader_cmd, EXTERNAL_DIR_NAME)

    print("Run CMake for Loader")
    LOADER_BUILD_DIR = RepoRelative("%s/Vulkan-Loader/%s" % (EXTERNAL_DIR_NAME, BUILD_DIR_NAME))

    print("Run CMake for Loader")
    cmake_cmd = f'cmake -S {LOADER_DIR} -B {LOADER_BUILD_DIR}'
    cmake_cmd += ' -D UPDATE_DEPS=ON -D BUILD_TESTS=OFF -D CMAKE_BUILD_TYPE=Release'

    # This enables better stack traces from tools like leak sanitizer by using the loader feature which prevents unloading of libraries at shutdown.
    cmake_cmd += ' -D LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING=ON'

    # GitHub actions runs our test as admin on Windows
    if IsGHA() and IsWindows():
        cmake_cmd += ' -D LOADER_USE_UNSAFE_FILE_SEARCH=ON'

    RunShellCmd(cmake_cmd)

    print("Build Loader")
    build_cmd = f'cmake --build {LOADER_BUILD_DIR}'
    RunShellCmd(build_cmd)

    print("Install Loader")
    install_cmd = f'cmake --install {LOADER_BUILD_DIR} --prefix {TEST_INSTALL_DIR}'
    RunShellCmd(install_cmd)

#
# Prepare Mock ICD for use with Layer Validation Tests
def BuildMockICD():
    VT_DIR = RepoRelative("%s/Vulkan-Tools" % EXTERNAL_DIR_NAME)
    if not os.path.exists(VT_DIR):
        print("Clone Vulkan-Tools Repository")
        clone_tools_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Tools.git'
        RunShellCmd(clone_tools_cmd, EXTERNAL_DIR_NAME)

    ICD_BUILD_DIR = RepoRelative("%s/Vulkan-Tools/%s" % (EXTERNAL_DIR_NAME,BUILD_DIR_NAME))

    print("Run CMake for ICD")
    cmake_cmd = f'cmake -S {VT_DIR} -B {ICD_BUILD_DIR} -D CMAKE_BUILD_TYPE=Release '
    cmake_cmd += '-DBUILD_CUBE=NO -DBUILD_VULKANINFO=NO -D INSTALL_ICD=ON -D UPDATE_DEPS=ON'
    RunShellCmd(cmake_cmd)

    print("Build Mock ICD")
    build_cmd = f'cmake --build {ICD_BUILD_DIR}'
    RunShellCmd(build_cmd)

    print("Install Mock ICD")
    install_cmd = f'cmake --install {ICD_BUILD_DIR} --prefix {TEST_INSTALL_DIR}'
    RunShellCmd(install_cmd)

#
# Prepare Profile Layer for use with Layer Validation Tests
def BuildProfileLayer():
    RunShellCmd('pip3 install jsonschema', EXTERNAL_DIR_NAME)

    VP_DIR = RepoRelative("%s/Vulkan-Profiles" % EXTERNAL_DIR_NAME)
    if not os.path.exists(VP_DIR):
        print("Clone Vulkan-Profiles Repository")
        clone_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Profiles.git'
        RunShellCmd(clone_cmd, EXTERNAL_DIR_NAME)

    BUILD_DIR = RepoRelative("%s/Vulkan-Profiles/%s" % (EXTERNAL_DIR_NAME, BUILD_DIR_NAME))

    print("Run CMake for Profile Layer")
    cmake_cmd = f'cmake -S {VP_DIR} -B {BUILD_DIR}'
    cmake_cmd += ' -D CMAKE_BUILD_TYPE=Release'
    cmake_cmd += ' -D UPDATE_DEPS=ON'
    cmake_cmd += ' -D PROFILES_BUILD_TESTS=OFF'
    RunShellCmd(cmake_cmd)

    print("Build Profile Layer")
    build_cmd = f'cmake --build {BUILD_DIR}'
    RunShellCmd(build_cmd)

    print("Install Profile Layer")
    install_cmd = f'cmake --install {BUILD_DIR} --prefix {TEST_INSTALL_DIR}'
    RunShellCmd(install_cmd)

#
# Run the Layer Validation Tests
def RunVVLTests():
    print("Run VVL Tests using Mock ICD")

    lvt_env = dict(os.environ)

    # Because we installed everything to TEST_INSTALL_DIR all the libraries/json files are in pre-determined locations
    # defined by GNUInstallDirs. This makes setting VK_LAYER_PATH and other environment variables trivial/robust.
    if IsWindows():
        lvt_env['VK_LAYER_PATH'] = os.path.join(TEST_INSTALL_DIR, 'bin')
        lvt_env['VK_DRIVER_FILES'] = os.path.join(TEST_INSTALL_DIR, 'bin\\VkICD_mock_icd.json')
    else:
        lvt_env['LD_LIBRARY_PATH'] = os.path.join(TEST_INSTALL_DIR, 'lib')
        lvt_env['VK_LAYER_PATH'] = os.path.join(TEST_INSTALL_DIR, 'share/vulkan/explicit_layer.d')
        lvt_env['VK_DRIVER_FILES'] = os.path.join(TEST_INSTALL_DIR, 'share/vulkan/icd.d/VkICD_mock_icd.json')

    # Useful for debugging
    # lvt_env['VK_LOADER_DEBUG'] = 'error,warn,info'
    # lvt_env['VK_LAYER_TESTS_PRINT_DRIVER'] = '1'

    lvt_env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_KHRONOS_validation' + os.pathsep + 'VK_LAYER_KHRONOS_profiles'
    lvt_env['VK_KHRONOS_PROFILES_SIMULATE_CAPABILITIES'] = 'SIMULATE_API_VERSION_BIT,SIMULATE_FEATURES_BIT,SIMULATE_PROPERTIES_BIT,SIMULATE_EXTENSIONS_BIT,SIMULATE_FORMATS_BIT,SIMULATE_QUEUE_FAMILY_PROPERTIES_BIT'

    # By default use the max_profile.json
    if "VK_KHRONOS_PROFILES_PROFILE_FILE" not in os.environ:
        lvt_env['VK_KHRONOS_PROFILES_PROFILE_FILE'] = RepoRelative('tests/device_profiles/max_profile.json')

    # By default set portability to false
    if "VK_KHRONOS_PROFILES_EMULATE_PORTABILITY" not in os.environ:
        lvt_env['VK_KHRONOS_PROFILES_EMULATE_PORTABILITY'] = 'false'

    lvt_env['VK_KHRONOS_PROFILES_DEBUG_REPORTS'] = 'DEBUG_REPORT_ERROR_BIT'

    lvt_cmd = os.path.join(TEST_INSTALL_DIR, 'bin', 'vk_layer_validation_tests')

    # The following test fail with thread sanitization enabled.
    failing_tsan_tests = '-VkPositiveLayerTest.QueueThreading'
    failing_tsan_tests += ':NegativeCommand.SecondaryCommandBufferRerecordedExplicitReset'
    failing_tsan_tests += ':NegativeCommand.SecondaryCommandBufferRerecordedNoReset'
    failing_tsan_tests += ':NegativeSyncVal.CopyOptimalImageHazards'
    failing_tsan_tests += ':NegativeViewportInheritance.BasicUsage'
    failing_tsan_tests += ':NegativeViewportInheritance.MultiViewport'
    # NOTE: This test fails sporadically. Make sure to run multiple times.
    failing_tsan_tests += ':PositiveSyncObject.WaitTimelineSemThreadRace'

    RunShellCmd(lvt_cmd + f" --gtest_filter={failing_tsan_tests}", env=lvt_env)

    print("Re-Running multithreaded tests with VK_LAYER_FINE_GRAINED_LOCKING disabled", flush=True)
    lvt_env['VK_LAYER_FINE_GRAINED_LOCKING'] = '0'
    RunShellCmd(lvt_cmd + f' --gtest_filter=*Thread*:{failing_tsan_tests}', env=lvt_env)

def GetArgParser():
    configs = ['release', 'debug']
    default_config = configs[0]

    osx_choices = ['min', 'latest']
    osx_default = osx_choices[1]

    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-c', '--config', dest='configuration',
        metavar='CONFIG', action='store',
        choices=configs, default=default_config,
        help='Build target configuration. Can be one of: {0}'.format(
            ', '.join(configs)))
    parser.add_argument(
        '--cmake', dest='cmake',
        metavar='CMAKE', type=str,
        default='', help='Additional args to pass to cmake')
    parser.add_argument(
        '--build', dest='build',
        action='store_true', help='Build the layers')
    parser.add_argument(
        '--test', dest='test',
        action='store_true', help='Tests the layers')
    parser.add_argument(
        '--osx', dest='osx', action='store',
        choices=osx_choices, default=osx_default,
        help='Sets MACOSX_DEPLOYMENT_TARGET on Apple platforms.')
    return parser
