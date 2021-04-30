#!/usr/bin/env python3
# Copyright (c) 2021 Valve Corporation
# Copyright (c) 2021 LunarG, Inc.

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
# Author: Nathaniel Cesario <nathaniel@lunarg.com>

# Test to make sure cmake is compliant with minimum version
# NOTE: this assumes VVL has already been "setup" (update_deps.py has been run, etc.)

import os, platform, subprocess, re
from urllib.parse import urlparse

import utils.utils as utils
import common_ci

def get_cmake_min_version():
    with open(os.path.join(common_ci.PROJECT_ROOT, 'CMakeLists.txt'), 'r') as fd:
        cmakelist = fd.read()
        m = re.match(r'^\s*cmake_minimum_required[(][^)]*VERSION\s+([\d.]+)', cmakelist, re.MULTILINE & re.IGNORECASE)
        if m is not None:
            return m.group(1)
        else:
            return '3.10.2'


curr_platform = platform.system().lower()
cmake_version = get_cmake_min_version()
cmake_urls = {
    'linux': { 'url': f'https://github.com/Kitware/CMake/releases/download/v{cmake_version}/cmake-{cmake_version}-Linux-x86_64.tar.gz', 'dirname': f'cmake-{cmake_version}-Linux-x86_64' },
    'windows': { 'url': f'https://github.com/Kitware/CMake/releases/download/v{cmake_version}/cmake-{cmake_version}-win64-x64.zip', 'dirname': f'cmake-{cmake_version}-win64-x64' }
}

#
# Check if the system is Windows
def get_cmake_url():
    try: return cmake_urls[curr_platform]
    except: raise Exception(f'Unsupported platform: {curr_platform}')

def get_cmake_exe_path(url_info):
    path = os.path.join(os.getcwd(), url_info['dirname'], 'bin', 'cmake')
    if curr_platform == 'windows': path = path + '.exe'
    return path

def get_cmake_args(path):
    args = [path, '-C', '../external/helper.cmake', '..']
    if curr_platform == 'windows': args.extend(['-Ax64'])
    return args

def main():
    url_info = get_cmake_url()
    cmake_exe_path = get_cmake_exe_path(url_info)

    if not os.path.exists(cmake_exe_path):
        url = urlparse(url_info['url'])
        print(f'cmake minimum version does not exist locally; downloading from {url_info["url"]}')
        cmake_archive = os.path.basename(url.path)
        with utils.URLRequest(url) as res:
            with open(cmake_archive, 'wb') as fd: fd.write(res.read())
        utils.expand_archive(cmake_archive)

    cmake_build_dir = 'build-cmake-test'
    utils.make_or_exist_dirs(cmake_build_dir, True)
    currDir = os.getcwd()
    cmake_args = get_cmake_args(cmake_exe_path)
    subprocess.check_call(cmake_args, cwd=cmake_build_dir)

if __name__ == '__main__': main()
