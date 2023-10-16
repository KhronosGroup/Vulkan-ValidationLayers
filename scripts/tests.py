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
import os
import common_ci

#
# Module Entrypoint
def Build(args):
    config = args.configuration

    # Since this script uses Ninja to build Windows users need to be in a developer command prompt.
    if common_ci.IsWindows():
        # This environment variable is arbitrary. I just picked one set by the developer command prompt.
        if "VSCMD_ARG_TGT_ARCH" not in os.environ:
            print("This script must be invoked in a developer command prompt!")
            sys.exit(1)

    try:
        common_ci.BuildVVL(config = config, cmake_args = args.cmake, build_tests = "ON", mock_android = args.mockAndroid)
        common_ci.BuildLoader()
        common_ci.BuildProfileLayer(args.mockAndroid)
        common_ci.BuildMockICD(args.mockAndroid)

    except subprocess.CalledProcessError as proc_error:
        print('Command "%s" failed with return code %s' % (' '.join(proc_error.cmd), proc_error.returncode))
        sys.exit(proc_error.returncode)
    except Exception as unknown_error:
        print('An unknown error occured: %s', unknown_error)
        sys.exit(1)

    sys.exit(0)

def Test(args):
    try:
        common_ci.RunVVLTests(args)

    except subprocess.CalledProcessError as proc_error:
        print('Command "%s" failed with return code %s' % (' '.join(proc_error.cmd), proc_error.returncode))
        sys.exit(proc_error.returncode)
    except Exception as unknown_error:
        print('An unknown error occured: %s', unknown_error)
        sys.exit(1)

    sys.exit(0)

if __name__ == '__main__':
    parser = common_ci.GetArgParser()
    parser.add_argument(
        '--mockAndroid', dest='mockAndroid',
        action='store_true', help='Use Mock Android')

    args = parser.parse_args()

    if (args.build):
        Build(args)
    if (args.test):
        Test(args)
