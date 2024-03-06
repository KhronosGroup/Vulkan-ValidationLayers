#!/usr/bin/env python3
# Copyright 2023-2024 The Khronos Group Inc.
# Copyright 2023-2024 Valve Corporation
# Copyright 2023-2024 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0

import os
import subprocess
import sys

# helper to define paths relative to the repo root
def RepoRelative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '../../', path))

def BuildGn():
    if not os.path.exists(RepoRelative("depot_tools")):
        clone_cmd = 'git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git depot_tools'.split(" ")
        subprocess.check_call(clone_cmd)

    os.environ['PATH'] = os.environ.get('PATH') + ":" + RepoRelative("depot_tools")

    # Updating Repo Dependencies and GN Toolchain
    update_cmd = './scripts/gn/update_deps.sh'
    subprocess.check_call(update_cmd)

    gn_check_cmd = 'gn gen --check out/Debug'.split(" ")
    subprocess.check_call(gn_check_cmd)

    # Generating Ninja Files
    gn_gen_cmd = 'gn gen out/Debug'.split(" ")
    subprocess.check_call(gn_gen_cmd)

    # Running Ninja Build
    ninja_build_cmd = 'ninja -C out/Debug'.split(" ")
    subprocess.check_call(ninja_build_cmd)

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