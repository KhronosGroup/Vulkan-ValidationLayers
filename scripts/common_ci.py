#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
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
import argparse

# Use Ninja for all platforms for performance/simplicity
os.environ['CMAKE_GENERATOR'] = "Ninja"

# helper to define paths relative to the repo root
def RepoRelative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

# Points to the directory containing the top level CMakeLists.txt
PROJECT_SRC_DIR = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))
if not os.path.isfile(f'{PROJECT_SRC_DIR}/CMakeLists.txt'):
    print(f'PROJECT_SRC_DIR invalid! {PROJECT_SRC_DIR}')
    sys.exit(1)

# Where all artifacts will ultimately be placed under
CI_BUILD_DIR = RepoRelative('build-ci')
# Where all dependencies will be installed under
CI_EXTERNAL_DIR = f'{CI_BUILD_DIR}/external'
# Where all repos will install to
CI_INSTALL_DIR = f'{CI_BUILD_DIR}/install'

# Returns true if we are running in GitHub actions
# https://docs.github.com/en/actions/learn-github-actions/variables#default-environment-variables
def IsGHA():
    if 'GITHUB_ACTION' in os.environ:
        return True
    return False

# Runs a command in a directory and returns its return code.
# Directory is project root by default, or a relative path from project root
def RunShellCmd(command, start_dir = PROJECT_SRC_DIR, env=None, verbose=False):
    # Flush stdout here. Helps when debugging on CI.
    sys.stdout.flush()

    if start_dir != PROJECT_SRC_DIR:
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
# Prepare the Validation Layers for testing
def BuildVVL(config, cmake_args, build_tests, mock_android):
    print("Log CMake version")
    cmake_ver_cmd = 'cmake --version'
    RunShellCmd(cmake_ver_cmd)

    SRC_DIR = PROJECT_SRC_DIR
    BUILD_DIR = f'{CI_BUILD_DIR}/vvl'

    print("Configure VVL")
    cmake_cmd = f'cmake -S {SRC_DIR} -B {BUILD_DIR}'
    cmake_cmd += f' -D CMAKE_BUILD_TYPE={config}'
    cmake_cmd += f' -D BUILD_TESTS={build_tests}'
    cmake_cmd += f' -D UPDATE_DEPS=ON -D UPDATE_DEPS_DIR={CI_EXTERNAL_DIR}'
    cmake_cmd += ' -D BUILD_WERROR=ON'

    if cmake_args:
         cmake_cmd += f' {cmake_args}'

    if mock_android:
         cmake_cmd += ' -DVVL_MOCK_ANDROID=ON'

    RunShellCmd(cmake_cmd)

    print("Build VVL")
    build_cmd = f'cmake --build {BUILD_DIR}'
    RunShellCmd(build_cmd)

    print("Install VVL")
    install_cmd = f'cmake --install {BUILD_DIR} --prefix {CI_INSTALL_DIR}'
    RunShellCmd(install_cmd)

#
# Prepare Loader for executing Layer Validation Tests
def BuildLoader():
    SRC_DIR = f'{CI_EXTERNAL_DIR}/Vulkan-Loader'
    BUILD_DIR = f'{SRC_DIR}/build'

    if not os.path.exists(SRC_DIR):
        print("Clone Loader Source Code")
        clone_loader_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Loader.git'
        RunShellCmd(clone_loader_cmd, CI_EXTERNAL_DIR)

    print("Run CMake for Loader")
    cmake_cmd = f'cmake -S {SRC_DIR} -B {BUILD_DIR}'
    cmake_cmd += ' -D UPDATE_DEPS=ON -D CMAKE_BUILD_TYPE=Release'

    # GitHub actions runs our test as admin on Windows
    if IsGHA() and IsWindows():
        cmake_cmd += ' -D LOADER_USE_UNSAFE_FILE_SEARCH=ON'

    RunShellCmd(cmake_cmd)

    print("Build Loader")
    build_cmd = f'cmake --build {BUILD_DIR}'
    RunShellCmd(build_cmd)

    print("Install Loader")
    install_cmd = f'cmake --install {BUILD_DIR} --prefix {CI_INSTALL_DIR}'
    RunShellCmd(install_cmd)

#
# Prepare Mock ICD for use with Layer Validation Tests
def BuildMockICD(mockAndroid):
    SRC_DIR = f'{CI_EXTERNAL_DIR}/Vulkan-Tools'
    BUILD_DIR = f'{SRC_DIR}/build'

    if not os.path.exists(SRC_DIR):
        print("Clone Vulkan-Tools Repository")
        clone_tools_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Tools.git'
        RunShellCmd(clone_tools_cmd, CI_EXTERNAL_DIR)

    print("Configure Mock ICD")
    cmake_cmd = f'cmake -S {SRC_DIR} -B {BUILD_DIR} -D CMAKE_BUILD_TYPE=Release '
    cmake_cmd += '-DBUILD_CUBE=NO -DBUILD_VULKANINFO=NO -D INSTALL_ICD=ON -D UPDATE_DEPS=ON'
    if mockAndroid:
        cmake_cmd += ' -DBUILD_MOCK_ANDROID_SUPPORT=ON'
    RunShellCmd(cmake_cmd)

    print("Build Mock ICD")
    build_cmd = f'cmake --build {BUILD_DIR} --target VkICD_mock_icd'
    RunShellCmd(build_cmd)

    print("Install Mock ICD")
    install_cmd = f'cmake --install {BUILD_DIR} --prefix {CI_INSTALL_DIR}'
    RunShellCmd(install_cmd)

#
# Prepare Profile Layer for use with Layer Validation Tests
def BuildProfileLayer(mockAndroid):
    RunShellCmd('pip3 install jsonschema')

    SRC_DIR = f'{CI_EXTERNAL_DIR}/Vulkan-Profiles'
    BUILD_DIR = f'{SRC_DIR}/build'

    if not os.path.exists(SRC_DIR):
        print("Clone Vulkan-Profiles Repository")
        clone_cmd = 'git clone https://github.com/KhronosGroup/Vulkan-Profiles.git'
        RunShellCmd(clone_cmd, CI_EXTERNAL_DIR)

    print("Run CMake for Profile Layer")
    cmake_cmd = f'cmake -S {SRC_DIR} -B {BUILD_DIR}'
    cmake_cmd += ' -D CMAKE_BUILD_TYPE=Release'
    cmake_cmd += ' -D UPDATE_DEPS=ON'
    if mockAndroid:
        cmake_cmd += ' -DBUILD_MOCK_ANDROID_SUPPORT=ON'
    RunShellCmd(cmake_cmd)

    print("Build Profile Layer")
    build_cmd = f'cmake --build {BUILD_DIR}'
    RunShellCmd(build_cmd)

    print("Install Profile Layer")
    install_cmd = f'cmake --install {BUILD_DIR} --prefix {CI_INSTALL_DIR}'
    RunShellCmd(install_cmd)

#
# Run the Layer Validation Tests
def RunVVLTests(args):
    print("Run VVL Tests using Mock ICD")

    lvt_env = dict(os.environ)

    # Because we installed everything to CI_INSTALL_DIR all the libraries/json files are in pre-determined locations
    # defined by GNUInstallDirs. This makes setting VK_LAYER_PATH and other environment variables trivial/robust.
    if IsWindows():
        lvt_env['VK_LAYER_PATH'] = os.path.join(CI_INSTALL_DIR, 'bin')
        lvt_env['VK_DRIVER_FILES'] = os.path.join(CI_INSTALL_DIR, 'bin\\VkICD_mock_icd.json')
    else:
        lvt_env['LD_LIBRARY_PATH'] = os.path.join(CI_INSTALL_DIR, 'lib')
        lvt_env['VK_LAYER_PATH'] = os.path.join(CI_INSTALL_DIR, 'share/vulkan/explicit_layer.d')
        lvt_env['VK_DRIVER_FILES'] = os.path.join(CI_INSTALL_DIR, 'share/vulkan/icd.d/VkICD_mock_icd.json')

    # This enables better stack traces from tools like leak sanitizer by using the loader feature which prevents unloading of libraries at shutdown.
    lvt_env['VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING'] = '1'

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

    lvt_cmd = os.path.join(CI_INSTALL_DIR, 'bin', 'vk_layer_validation_tests')

    # The following test(s) fail with thread sanitization enabled.
    failing_tsan_tests = '-VkPositiveLayerTest.QueueThreading'
    failing_tsan_tests += ':NegativeCommand.SecondaryCommandBufferRerecordedExplicitReset'
    failing_tsan_tests += ':NegativeCommand.SecondaryCommandBufferRerecordedNoReset'
    failing_tsan_tests += ':NegativeSyncVal.CopyOptimalImageHazards'
    failing_tsan_tests += ':NegativeViewportInheritance.BasicUsage'
    failing_tsan_tests += ':NegativeViewportInheritance.MultiViewport'
    # NOTE: These test(s) fails sporadically.
    # These need extra care to prevent a regression in the future.
    failing_tsan_tests += ':PositiveSyncObject.WaitTimelineSemThreadRace'
    failing_tsan_tests += ':PositiveQuery.ResetQueryPoolFromDifferentCB'

    if args.mockAndroid:
        # TODO - only reason running this subset, is mockAndoid fails any test that does
        # a manual vkCreateDevice call and need to investigate more why
        RunShellCmd(lvt_cmd + " --gtest_filter=*AndroidHardwareBuffer.*:*AndroidExternalResolve.*", env=lvt_env)
        return

    RunShellCmd(lvt_cmd + f" --gtest_filter={failing_tsan_tests}", env=lvt_env)

    print("Re-Running multithreaded tests with VK_LAYER_FINE_GRAINED_LOCKING disabled")
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
