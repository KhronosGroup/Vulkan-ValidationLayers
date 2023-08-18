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
import subprocess
import sys
import common_ci

def BuildGn():
    if not os.path.exists(common_ci.RepoRelative("depot_tools")):
        print("Cloning Chromium depot_tools\n")
        clone_cmd = 'git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git depot_tools'
        common_ci.RunShellCmd(clone_cmd)

    os.environ['PATH'] = os.environ.get('PATH') + ":" + common_ci.RepoRelative("depot_tools")

    print("Updating Repo Dependencies and GN Toolchain\n")
    update_cmd = './scripts/gn/update_deps.sh'
    common_ci.RunShellCmd(update_cmd)

    print("Checking Header Dependencies\n")
    gn_check_cmd = 'gn gen --check out/Debug'
    common_ci.RunShellCmd(gn_check_cmd)

    print("Generating Ninja Files\n")
    gn_gen_cmd = 'gn gen out/Debug'
    common_ci.RunShellCmd(gn_gen_cmd)

    print("Running Ninja Build\n")
    ninja_build_cmd = 'ninja -C out/Debug'
    common_ci.RunShellCmd(ninja_build_cmd)

#
# Module Entrypoint
def main():
    try:
        BuildGn()

    except subprocess.CalledProcessError as proc_error:
        print('Command "%s" failed with return code %s' % (' '.join(proc_error.cmd), proc_error.returncode))
        sys.exit(proc_error.returncode)
    except Exception as unknown_error:
        print('An unkown error occured: %s', unknown_error)
        sys.exit(1)

    sys.exit(0)

if __name__ == '__main__':
  main()
