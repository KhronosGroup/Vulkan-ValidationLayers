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

import os
import argparse
import shutil
import subprocess
import sys
import platform

import common_ci

#
# Module Entrypoint
def Build(args):
    try:
        common_ci.BuildVVL(args, True)
        common_ci.BuildLoader(args)
        common_ci.BuildProfileLayer(args)
        common_ci.BuildMockICD(args)
        common_ci.CheckVVLCodegenConsistency(args)

    except subprocess.CalledProcessError as proc_error:
        print('Command "%s" failed with return code %s' % (' '.join(proc_error.cmd), proc_error.returncode))
        sys.exit(proc_error.returncode)
    except Exception as unknown_error:
        print('An unkown error occured: %s', unknown_error)
        sys.exit(1)

    sys.exit(0)

def Test(args):
    try:
        common_ci.RunVVLTests(args)

    except subprocess.CalledProcessError as proc_error:
        print('Command "%s" failed with return code %s' % (' '.join(proc_error.cmd), proc_error.returncode))
        sys.exit(proc_error.returncode)
    except Exception as unknown_error:
        print('An unkown error occured: %s', unknown_error)
        sys.exit(1)

    sys.exit(0)

if __name__ == '__main__':
    parser = common_ci.GetArgParser()
    args = parser.parse_args()

    if (args.build):
        Build(args)
    if (args.test):
        Test(args)
