#!/usr/bin/env python3
#
# Copyright (c) 2023 LunarG, Inc.
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
import argparse
import shutil
import common_ci

def main():
    parser = argparse.ArgumentParser(description='Help build with unreleased extensions')
    parser.add_argument('--spec_repo', action='store', type=str, help='Path to Vulkan-Docs repo')
    parser.add_argument('--build_dir', action='store', type=str, help='Defaults to ./build')
    args = parser.parse_args()

    if not args.spec_repo:
        sys.exit('missing --spec_repo')

    spec_repo_path = os.path.abspath(os.path.dirname(args.spec_repo))
    if (not os.path.isdir(spec_repo_path)):
        sys.exit(f'Cannot find directory {spec_repo_path}')

    vk_xml_path = os.path.join(spec_repo_path, 'xml', 'vk.xml')
    if (not os.path.isfile(vk_xml_path)):
        sys.exit(f'Cannot find vk.xml file at {vk_xml_path}')

    build_dir = args.build_dir if args.build_dir else common_ci.RepoRelative('build')

    # Will grab latest Vulkan-Headers and install properly
    common_ci.RunShellCmd(command=f'cmake -S . -B {build_dir} -D UPDATE_DEPS=ON -D BUILD_WERROR=ON -D BUILD_TESTS=ON -D CMAKE_BUILD_TYPE=Debug', verbose=True)

    external_dir = 'external/Debug/64'
    vulkan_header_path = common_ci.RepoRelative(f'{external_dir}/Vulkan-Headers')
    if (not os.path.isdir(vulkan_header_path)):
        sys.exit(f'Cannot find Vulkan Headers dependency directory at {vulkan_header_path}')

    registry_dir = common_ci.RepoRelative(f'{external_dir}/Vulkan-Headers/registry')
    vul_dir = common_ci.RepoRelative(f'{external_dir}/Vulkan-Utility-Libraries')
    spirv_headers = common_ci.RepoRelative(f'{external_dir}/SPIRV-Headers/include/spirv/unified1')

    # Generate new headers
    common_ci.RunShellCmd(command=f'make -C {os.path.join(spec_repo_path, "xml")}', verbose=True)
    new_headers = os.path.join(spec_repo_path, 'gen', 'include', 'vulkan')

    # Update the XML file
    shutil.copyfile(vk_xml_path, os.path.join(registry_dir, 'vk.xml'))

    # Update header
    for filename in os.listdir(new_headers):
        f = os.path.join(new_headers, filename)
        if os.path.isfile(f):
            shutil.copyfile(f, os.path.join(vulkan_header_path, 'build', 'install', 'include', 'vulkan', filename))

    # Generate entrypoints
    common_ci.RunShellCmd(command=f'python3 {common_ci.RepoRelative("scripts/generate_source.py")} {registry_dir} {spirv_headers}', verbose=True)
    # Update VUL
    common_ci.RunShellCmd(command=f'python3 {os.path.join(vul_dir, "scripts", "generate_source.py")} {registry_dir}', verbose=True)
    common_ci.RunShellCmd(command=f'cmake --build {os.path.join(vul_dir, "build")} --target install', verbose=True)

    # Profit💲
    common_ci.RunShellCmd(command=f'cmake --build {build_dir} --config Debug', verbose=True)

if __name__ == '__main__':
  main()
