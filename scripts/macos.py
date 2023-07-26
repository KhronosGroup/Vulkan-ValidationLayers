#!/usr/bin/env python3
# Copyright (c) 2020-2023 Valve Corporation
# Copyright (c) 2020-2023 LunarG, Inc.

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

import subprocess
import sys
import platform
import common_ci
import os

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

def main():
    parser = common_ci.GetArgParser()
    args = parser.parse_args()

    config = args.configuration
    osx = args.osx

    SetupDarwin(osx)

    try:
        common_ci.BuildVVL(config = config, cmake_args = args.cmake, build_tests = "OFF")
    except subprocess.CalledProcessError as proc_error:
        print('Command "%s" failed with return code %s' % (' '.join(proc_error.cmd), proc_error.returncode))
        sys.exit(proc_error.returncode)
    except Exception as unknown_error:
        print('An unkown error occured: %s', unknown_error)
        sys.exit(1)

    sys.exit(0)

if __name__ == '__main__':
  main()

