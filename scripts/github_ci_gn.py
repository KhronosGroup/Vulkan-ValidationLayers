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

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))

#
# Build VVL repo using gn and Ninja
def BuildGn():
    print("Cloning Ninja depot_tools\n")
    clone_cmd = 'git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git depot_tools'
    ret_code = common_ci.RunShellCmd(clone_cmd, PROJECT_ROOT)
    os.environ['PATH'] = os.environ.get('PATH') + ":" + common_ci.repo_relative("depot_tools")

    if ret_code == 0:
        print("Updating Ninja build dependencies\n")
        update_cmd = './build-gn/update_deps.sh'
        ret_code = common_ci.RunShellCmd(update_cmd, PROJECT_ROOT)

    if ret_code == 0:
        print("Generating Ninja Files\n", )
        # TODO: Enable ccache for Ninja builds : --args="cc_wrapper=\"ccache\""
        #       There are issues passing this on the command line.
        #       Possible solution is to write/append this line to the out/Debug/args.gn file
        gn_gen_cmd = 'gn gen out/Debug'
        ret_code = common_ci.RunShellCmd(gn_gen_cmd, PROJECT_ROOT)

    if ret_code == 0:
        print("Running Ninja Build\n", )
        ninja_build_cmd = 'ninja -C out/Debug'
        ret_code = common_ci.RunShellCmd(ninja_build_cmd, PROJECT_ROOT)

    return ret_code

#
# Module Entrypoint
def main():
    parser = argparse.ArgumentParser(description='''Usage: python3 ./scripts/github_ci_gn.py
    - Reqires python3
    - Run script in repo root
    ''', formatter_class=RawDescriptionHelpFormatter)

    if sys.version_info[0] != 3:
        print("This script requires Python 3. Run script with [-h] option for more details.")
        sys.exit(0)

    ret_code = BuildGn()
    if ret_code:
        sys.exit(ret_code)
    else:
        sys.exit(0)

if __name__ == '__main__':
  main()
